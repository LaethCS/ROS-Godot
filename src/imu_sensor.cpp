#include "imu_sensor.hpp"
#include "ros_bridge.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/rigid_body3d.hpp>
#include <random>

using namespace godot;

ImuSensor::ImuSensor() {
    update_rate = 0.02; // 50 Hz update rate
    update_timer = 0.0;
    noise_stddev = 0.05;
    last_linear_velocity = Vector3(0, 0, 0);
}

ImuSensor::~ImuSensor() {}

void ImuSensor::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_update_rate", "rate"), &ImuSensor::set_update_rate);
    ClassDB::bind_method(D_METHOD("get_update_rate"), &ImuSensor::get_update_rate);
    ClassDB::add_property("ImuSensor", PropertyInfo(Variant::FLOAT, "update_rate"), "set_update_rate", "get_update_rate");

    ClassDB::bind_method(D_METHOD("set_noise_stddev", "noise"), &ImuSensor::set_noise_stddev);
    ClassDB::bind_method(D_METHOD("get_noise_stddev"), &ImuSensor::get_noise_stddev);
    ClassDB::add_property("ImuSensor", PropertyInfo(Variant::FLOAT, "noise_stddev"), "set_noise_stddev", "get_noise_stddev");
}

void ImuSensor::_physics_process(double delta) {
    update_timer += delta;
    if (update_timer < update_rate) {
        return;
    }
    update_timer = 0.0;

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

    // Retrieve angular velocity and global spatial orientation quaternion
    Vector3 angular_vel = parent->get_angular_velocity();
    Quaternion orientation = get_global_transform().basis.get_rotation_quaternion();

    // Introduce Gaussian noise to emulate realistic sensor characteristics
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> noise(0.0, noise_stddev);

    linear_accel.x += noise(gen);
    linear_accel.y += noise(gen);
    linear_accel.z += noise(gen);
    angular_vel.x += noise(gen);
    angular_vel.y += noise(gen);
    angular_vel.z += noise(gen);

    // Dispatch IMU telemetry package to the centralized ROS2 communication bridge
    if (RosBridge::get_singleton()) {
        RosBridge::get_singleton()->publish_imu_data("imu_link", linear_accel, angular_vel, orientation);
    }
}

void ImuSensor::set_update_rate(double p_rate) { update_rate = p_rate; }
double ImuSensor::get_update_rate() const { return update_rate; }
void ImuSensor::set_noise_stddev(double p_noise) { noise_stddev = p_noise; }
double ImuSensor::get_noise_stddev() const { return noise_stddev; }  