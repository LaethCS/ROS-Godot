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
    publisher = ros_node->create_publisher<std_msgs::msg::Float32>("/sensor_distance", 10);
}

void TestSensor::_physics_process(double delta) {
    // ROS2 muss weiterhin atmen können
    if (rclcpp::ok()) {
        rclcpp::spin_some(ros_node);
    }

    // Timer akkumulieren
    time_passed += delta;

    // 0.04 Sekunden = exakt 25 Hz
    if (time_passed >= 0.04) {
        time_passed = 0.0; // Timer zurücksetzen

        std_msgs::msg::Float32 msg;

        // Zwingt Godot, den Laser EXAKT jetzt physikalisch neu zu berechnen
        force_raycast_update(); 

        if (is_colliding()) {
            Vector3 my_pos = get_global_position();
            Vector3 hit_pos = get_collision_point();
            msg.data = my_pos.distance_to(hit_pos);
        } else {
            msg.data = -1.0;
        }

        publisher->publish(msg);
    }
}
