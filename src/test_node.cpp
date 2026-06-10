#include "test_node.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <rclcpp/rclcpp.hpp>

using namespace godot;

TestNode::TestNode() {}
TestNode::~TestNode() {}

void TestNode::_bind_methods() {}

void TestNode::_ready() {
    UtilityFunctions::print("TestNode gestartet.");

    if (!rclcpp::ok()) {
        rclcpp::init(0, nullptr);
    }

    auto ros_node = rclcpp::Node::make_shared("godot_test_node");
    RCLCPP_INFO(ros_node->get_logger(), "Hallo aus der Godot-Engine! Ich bin jetzt im DDS-Netzwerk.");
}