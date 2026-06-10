#include "test_sensor.h"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

TestSensor::TestSensor() {}
TestSensor::~TestSensor() {}

void TestSensor::_bind_methods() {}

void TestSensor::_ready() {
    if (!rclcpp::ok()) {
        rclcpp::init(0, nullptr);
    }

    ros_node = rclcpp::Node::make_shared("godot_distance_sensor");
    publisher = ros_node->create_publisher<std_msgs::msg::Float32>("sensor_distance", 10);
    
    UtilityFunctions::print("TestSensor bereit! Sende Daten auf /sensor_distance");
}

void TestSensor::_process(double delta) {
    if (rclcpp::ok()) {
        rclcpp::spin_some(ros_node);
    }

    std_msgs::msg::Float32 msg;
    
    if (is_colliding()) {
        Vector3 my_pos = get_global_position();
        Vector3 hit_pos = get_collision_point();
        msg.data = my_pos.distance_to(hit_pos);
    } else {
        msg.data = -1.0; 
    }

    publisher->publish(msg);
}