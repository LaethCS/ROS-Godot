#include "odom_sensor.hpp"
#include "ros_bridge.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/rigid_body3d.hpp>

using namespace godot;

OdomSensor::OdomSensor() {}
OdomSensor::~OdomSensor() {}

void OdomSensor::_bind_methods() {}

// Physics process loop throttling odometry updates to the configured frequency,
// extracting parent rigid body state, converting velocities to local frame, and publishing via ROS bridge.
void OdomSensor::_physics_process(double delta) {
    if (!throttle.update(delta)) return;

    // Odometry requires a physical parent body to extract motion tracking data
    RigidBody3D* parent = Object::cast_to<RigidBody3D>(get_parent());
    if (!parent) return;

    // 1. Retrieve global position and orientation from Godot physics space
    Transform3D global_transform = parent->get_global_transform();
    Vector3 global_pos = global_transform.origin;
    Quaternion global_rot = global_transform.basis.get_rotation_quaternion();

    // 2. Transform global linear and angular velocities into the local robot reference frame
    Vector3 global_lin_vel = parent->get_linear_velocity();
    Vector3 global_ang_vel = parent->get_angular_velocity();

    // Use the inverse orientation quaternion to rotate global velocity vectors into local coordinates
    Vector3 local_lin_vel = global_transform.basis.get_rotation_quaternion().inverse().xform(global_lin_vel);
    Vector3 local_ang_vel = global_transform.basis.get_rotation_quaternion().inverse().xform(global_ang_vel);

    RosBridge* bridge = RosBridge::get_singleton();
    if (bridge) {
        // A) Publish standard ROS2 odometry telemetry message on the /odom topic
        bridge->publish_odometry("odom", "base_link", global_pos, global_rot, local_lin_vel, local_ang_vel);

        // B) Broadcast dynamic tf2 transform linking the odom frame to the base_link frame
        bridge->send_tf_transform("odom", "base_link", global_transform);
    }
}