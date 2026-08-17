#include "ros2_publisher.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include "ros_bridge.hpp"

using namespace godot;

void ROS2Publisher::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_topic_name", "name"), &ROS2Publisher::set_topic_name);
    ClassDB::bind_method(D_METHOD("get_topic_name"), &ROS2Publisher::get_topic_name);
    ClassDB::bind_method(D_METHOD("publish_message", "message"), &ROS2Publisher::publish_message);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "topic_name"), "set_topic_name", "get_topic_name");
}

ROS2Publisher::ROS2Publisher() {
    topic_name = "/my_generic_topic";
    publisher_ = nullptr;
}

ROS2Publisher::~ROS2Publisher() {}

void ROS2Publisher::_ready() {
    // Retrieve the ROS node from the singleton and establish the publisher instance
    auto bridge = RosBridge::get_singleton();
    if (bridge) {
        auto ros_node = bridge->get_ros_node();
        if (ros_node) {
            publisher_ = ros_node->create_publisher<std_msgs::msg::String>(topic_name.utf8().get_data(), 10);
        }
    }
}

void ROS2Publisher::set_topic_name(const String &name) {
    topic_name = name;
}

String ROS2Publisher::get_topic_name() const {
    return topic_name;
}

// Publishes a string message to the ROS2 topic, applying lazy initialization if the publisher is unallocated
void ROS2Publisher::publish_message(const String &message) {
    if (publisher_ == nullptr) {
        auto bridge = RosBridge::get_singleton();
        if (bridge) {
            auto ros_node = bridge->get_ros_node();
            if (ros_node) {
                publisher_ = ros_node->create_publisher<std_msgs::msg::String>(topic_name.utf8().get_data(), 10);
            }
        }
    }

    if (publisher_ != nullptr) {
        std_msgs::msg::String ros_msg;
        ros_msg.data = message.utf8().get_data();
        publisher_->publish(ros_msg);
    } else {
        UtilityFunctions::printerr("Failed to publish message: Publisher pointer is null.");
    } 
}