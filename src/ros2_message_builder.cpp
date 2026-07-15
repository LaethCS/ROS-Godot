#include "ros2_message_builder.hpp"

namespace godot {

geometry_msgs::msg::Vector3 ROS2MessageBuilder::godot_to_ros_vector3(const Vector3& godot_vec) {
    geometry_msgs::msg::Vector3 ros_vec;
    // Koordinaten-Transformation: 
    // Godot X -> ROS X (Vorwaerts)
    // Godot Y -> ROS Z (Oben)
    // Godot Z -> ROS Y (Links, daher negiert)
    ros_vec.x = -godot_vec.z;
    ros_vec.y = -godot_vec.x;
    ros_vec.z = godot_vec.y;
    return ros_vec;
}

geometry_msgs::msg::Quaternion ROS2MessageBuilder::godot_to_ros_quaternion(const Quaternion& godot_quat) {
    geometry_msgs::msg::Quaternion ros_quat;
    // Quaternionen muessen entsprechend der Achsen-Transformation angepasst werden
    ros_quat.x = -godot_quat.z;
    ros_quat.y = -godot_quat.x;
    ros_quat.z = godot_quat.y;
    ros_quat.w = godot_quat.w;
    return ros_quat;
}

geometry_msgs::msg::Transform ROS2MessageBuilder::godot_to_ros_transform(const Transform3D& godot_transform) {
    geometry_msgs::msg::Transform ros_tf;
    
    // Translation (Position)
    ros_tf.translation = godot_to_ros_vector3(godot_transform.origin);
    
    // Rotation (Basis)
    Quaternion q = godot_transform.basis.get_rotation_quaternion();
    ros_tf.rotation = godot_to_ros_quaternion(q);
    
    return ros_tf;
}

geometry_msgs::msg::Pose ROS2MessageBuilder::godot_to_ros_pose(const Transform3D& godot_transform) {
    geometry_msgs::msg::Pose ros_pose;
    
    // Position (Achtung: Pose nutzt Point, nicht Vector3)
    geometry_msgs::msg::Vector3 vec = godot_to_ros_vector3(godot_transform.origin);
    ros_pose.position.x = vec.x;
    ros_pose.position.y = vec.y;
    ros_pose.position.z = vec.z;
    
    // Orientation
    Quaternion q = godot_transform.basis.get_rotation_quaternion();
    ros_pose.orientation = godot_to_ros_quaternion(q);
    
    return ros_pose;
}

std_msgs::msg::String ROS2MessageBuilder::string_to_ros(const String& godot_string) {
    std_msgs::msg::String ros_msg;
    // Wandelt den komplexen Godot-String sauber in einen Standard C++ String um
    ros_msg.data = godot_string.utf8().get_data();
    return ros_msg;
}

std_msgs::msg::Header ROS2MessageBuilder::create_ros_header(const String& frame_id, rclcpp::Node::SharedPtr node) {
    std_msgs::msg::Header header;
    
    // Setze den ROS2 Zeitstempel, falls der Node existiert
    if (node) {
        header.stamp = node->now();
    }
    
    // Setze die Frame ID (z.B. "lidar_link" oder "base_link")
    header.frame_id = frame_id.utf8().get_data();
    
    return header;
}

std_msgs::msg::ColorRGBA ROS2MessageBuilder::color_to_ros(const Color& godot_color) {
    std_msgs::msg::ColorRGBA ros_color;
    ros_color.r = godot_color.r;
    ros_color.g = godot_color.g;
    ros_color.b = godot_color.b;
    ros_color.a = godot_color.a;
    return ros_color;
}

} 