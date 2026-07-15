#ifndef ROS2_MESSAGE_BUILDER_HPP
#define ROS2_MESSAGE_BUILDER_HPP

// Godot Includes
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/color.hpp>

// ROS2 Core Includes
#include <rclcpp/rclcpp.hpp>

// ROS2 Standard Messages
#include <geometry_msgs/msg/vector3.hpp>
#include <geometry_msgs/msg/quaternion.hpp>
#include <geometry_msgs/msg/transform.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/header.hpp>
#include <std_msgs/msg/color_rgba.hpp>

namespace godot {

class ROS2MessageBuilder {
public:
    // --- Vektoren (Positionen & Geschwindigkeiten) ---
    // Konvertiert Godot Vector3 zu ROS2 geometry_msgs::msg::Vector3
    static geometry_msgs::msg::Vector3 godot_to_ros_vector3(const Vector3& godot_vec);

    // --- Rotationen (Quaternionen) ---
    // Konvertiert Godot Quaternion zu ROS2 geometry_msgs::msg::Quaternion
    static geometry_msgs::msg::Quaternion godot_to_ros_quaternion(const Quaternion& godot_quat);

    // --- Transformationen (Position + Rotation) ---
    // Konvertiert eine Godot Transform3D zu einer ROS2 geometry_msgs::msg::Transform
    static geometry_msgs::msg::Transform godot_to_ros_transform(const Transform3D& godot_transform);

    // Konvertiert eine Godot Transform3D zu einer ROS2 geometry_msgs::msg::Pose
    static geometry_msgs::msg::Pose godot_to_ros_pose(const Transform3D& godot_transform);

    // --- Strings ---
    // Konvertiert Godot String zu ROS2 std_msgs::msg::String
    static std_msgs::msg::String string_to_ros(const String& godot_string);

    // --- Header (Zeitstempel & Frame ID) ---
    // Erstellt einen standardisierten ROS2 Header, der fuer Sensoren wichtig ist
    static std_msgs::msg::Header create_ros_header(const String& frame_id, rclcpp::Node::SharedPtr node);

    // --- Farben ---
    // Konvertiert Godot Color (z.B. fuer Kameras/LEDs) zu ROS2 std_msgs::msg::ColorRGBA
    static std_msgs::msg::ColorRGBA color_to_ros(const Color& godot_color);
};

} // namespace godot

#endif