#include "imu_sensor.hpp"
#include "ros_bridge.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/rigid_body3d.hpp>
#include <random>

using namespace godot;

ImuSensor::ImuSensor() {
    update_rate = 1.0 / 60.0; // 60 Hz, matching Godot's default physics tick rate
    throttle.set_interval(update_rate);
    noise_stddev = 0.05;
    last_linear_velocity = Vector3(0, 0, 0);
}

ImuSensor::~ImuSensor() {}

void ImuSensor::_bind_methods() {
    // NOTE: update_rate is intentionally NOT exposed as an inspector property.
    // The rate is fixed (60 Hz, matching Godot's physics tick) to model a concrete
    // IMU device. Noise can be tuned either by adjusting its magnitude
    // (noise_stddev) or by switching the measurement noise fully on/off
    // (noise_enabled).
    ClassDB::bind_method(D_METHOD("set_noise_stddev", "noise"), &ImuSensor::set_noise_stddev);
    ClassDB::bind_method(D_METHOD("get_noise_stddev"), &ImuSensor::get_noise_stddev);
    ClassDB::add_property("ImuSensor", PropertyInfo(Variant::FLOAT, "noise_stddev"), "set_noise_stddev", "get_noise_stddev");

    ClassDB::bind_method(D_METHOD("set_noise_enabled", "p_val"), &ImuSensor::set_noise_enabled);
    ClassDB::bind_method(D_METHOD("get_noise_enabled"), &ImuSensor::get_noise_enabled);
    ClassDB::add_property("ImuSensor", PropertyInfo(Variant::BOOL, "noise_enabled"), "set_noise_enabled", "get_noise_enabled");
}

void ImuSensor::_physics_process(double delta) {
    if (!throttle.update(delta)) return;

    // IMUs require a physical parent body to measure dynamics
    RigidBody3D* parent = Object::cast_to<RigidBody3D>(get_parent());
    if (!parent) {
        return;
    }

    // Compute linear acceleration numerically via derivative of velocity (a = dv/dt)
    Vector3 current_vel = parent->get_linear_velocity();
    Vector3 linear_accel = (current_vel - last_linear_velocity) / delta;
    last_linear_velocity = current_vel;

    // Account for gravitational force which is inherently measured by real-world inertial sensors
    linear_accel.y += 9.81;

    // Capture the global pose of the parent RigidBody3D so rotation follows the
    // robot's physics motion (the body's own physics-driven pose), not the possibly
    // stale Node3D hierarchy transform at this read point. Broadcasts imu_link later.
    Transform3D global_pose = parent->get_global_transform();
    Quaternion orientation = global_pose.basis.get_rotation_quaternion().normalized();

    // Compute angular velocity by differentiating orientation between consecutive
    // samples (like a real IMU gyro). This is robust even when the RigidBody's
    // get_angular_velocity() reports 0 at the sample instant.
    Vector3 angular_vel(0.0, 0.0, 0.0);
    if (has_last_orientation) {
        Quaternion delta_q = orientation * last_orientation.inverse();
        // delta_q rotation angle is the integrated angular motion over this step.
        double angle = delta_q.get_angle();
        Vector3 axis = delta_q.get_axis();
        if (!axis.is_zero_approx() && angle > 0.0) {
            angular_vel = axis * (angle / delta);
        }
        // Handle 180° flips where get_axis() may be ambiguous but non-zero:
        if (angle <= 0.0) {
            // No measurable rotation this step -> keep zero.
            angular_vel = Vector3(0.0, 0.0, 0.0);
        }
    }
    last_orientation = orientation;
    has_last_orientation = true;

    // Introduce Gaussian noise to emulate realistic sensor characteristics (optional)
    if (noise_enabled) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<double> noise(0.0, noise_stddev);

        linear_accel.x += noise(gen);
        linear_accel.y += noise(gen);
        linear_accel.z += noise(gen);
        angular_vel.x += noise(gen);
        angular_vel.y += noise(gen);
        angular_vel.z += noise(gen);
    }

    // Dispatch IMU telemetry package to the centralized ROS2 communication bridge
    if (RosBridge::get_singleton()) {
        RosBridge* bridge = RosBridge::get_singleton();
        bridge->publish_imu_data("imu_link", linear_accel, angular_vel, orientation);

        // Broadcast the imu_link frame so downstream nodes can transform IMU data
        // into base_link / odom (required by SLAM, robot_localization, ...).
        bridge->send_tf_transform("base_link", "imu_link", global_pose);
    }
}

void ImuSensor::set_update_rate(double p_rate) {
    update_rate = p_rate;
    throttle.set_interval((p_rate > 0.0) ? p_rate : 0.0);
}
double ImuSensor::get_update_rate() const { return update_rate; }
void ImuSensor::set_noise_stddev(double p_noise) { noise_stddev = p_noise; }
double ImuSensor::get_noise_stddev() const { return noise_stddev; }
void ImuSensor::set_noise_enabled(bool p_val) { noise_enabled = p_val; }
bool ImuSensor::get_noise_enabled() const { return noise_enabled; }
