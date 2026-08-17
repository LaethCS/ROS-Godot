#ifndef ROS2_MESSAGE_BUILDER_HPP
#define ROS2_MESSAGE_BUILDER_HPP

// Godot Engine Variant Includes
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/color.hpp>

// ROS2 Core Client Library
#include <rclcpp/rclcpp.hpp>

// ROS2 Standard Message Definitions
#include <geometry_msgs/msg/vector3.hpp>
#include <geometry_msgs/msg/quaternion.hpp>
#include <geometry_msgs/msg/transform.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/header.hpp>
#include <std_msgs/msg/color_rgba.hpp>

namespace godot {

/**
 * @brief Utility class responsible for translating Godot engine data types 
 * and structures into equivalent ROS2 standard message formats.
 */
class ROS2MessageBuilder {
public:
    // --- Vectors (Positions & Velocities) ---
    
    /**
     * @brief Converts a Godot Vector3 into a ROS2 geometry_msgs::msg::Vector3.
     * @param godot_vec The source vector in Godot's coordinate system.
     * @return Converted geometry_msgs::msg::Vector3 in ROS coordinate conventions.
     */
    static geometry_msgs::msg::Vector3 godot_to_ros_vector3(const Vector3& godot_vec);

    // --- Rotations (Quaternions) ---
    
    /**
     * @brief Converts a Godot Quaternion into a ROS2 geometry_msgs::msg::Quaternion.
     * @param godot_quat The source rotation quaternion in Godot.
     * @return Converted geometry_msgs::msg::Quaternion mapped to ROS standards.
     */
    static geometry_msgs::msg::Quaternion godot_to_ros_quaternion(const Quaternion& godot_quat);

    // --- Transformations (Position + Rotation) ---
    
    /**
     * @brief Converts a Godot Transform3D into a ROS2 geometry_msgs::msg::Transform.
     * @param godot_transform The spatial transform matrix in Godot.
     * @return Converted geometry_msgs::msg::Transform containing translation and rotation.
     */
    static geometry_msgs::msg::Transform godot_to_ros_transform(const Transform3D& godot_transform);

    /**
     * @brief Converts a Godot Transform3D into a ROS2 geometry_msgs::msg::Pose.
     * @param godot_transform The spatial transform matrix in Godot.
     * @return Converted geometry_msgs::msg::Pose containing position points and orientation.
     */
    static geometry_msgs::msg::Pose godot_to_ros_pose(const Transform3D& godot_transform);

    // --- Strings ---
    
    /**
     * @brief Converts a Godot String into a ROS2 std_msgs::msg::String message.
     * @param godot_string The source string from Godot.
     * @return Converted std_msgs::msg::String containing standard C-string data.
     */
    static std_msgs::msg::String string_to_ros(const String& godot_string);

    // --- Headers (Timestamp & Frame ID) ---
    
    /**
     * @brief Creates a standardized ROS2 message header equipped with node timestamp and frame ID.
     * @param frame_id The target coordinate frame identifier (e.g., "base_link" or "lidar_link").
     * @param node Shared pointer to the active ROS2 node used for obtaining current time stamps.
     * @return Fully populated std_msgs::msg::Header structure.
     */
    static std_msgs::msg::Header create_ros_header(const String& frame_id, rclcpp::Node::SharedPtr node);

    // --- Colors ---
    
    /**
     * @brief Converts a Godot Color object into a ROS2 std_msgs::msg::ColorRGBA message.
     * @param godot_color The source color variant from Godot.
     * @return Converted std_msgs::msg::ColorRGBA with normalized RGBA channels.
     */
    static std_msgs::msg::ColorRGBA color_to_ros(const Color& godot_color);
};

} // namespace godot

#endif // ROS2_MESSAGE_BUILDER_HPP