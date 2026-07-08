#include "ros_bridge.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <geometry_msgs/msg/transform_stamped.hpp>

using namespace godot;

RosBridge *RosBridge::singleton = nullptr;

RosBridge::RosBridge() {
    if (singleton == nullptr) {
        singleton = this;
    }
}

RosBridge::~RosBridge() {
    if (singleton == this) {
        singleton = nullptr;
    }
}

RosBridge *RosBridge::get_singleton() {
    return singleton;
}

void RosBridge::_ready() {

    UtilityFunctions::print("RosBridge::_ready() wurde aufgerufen");
    if (singleton != this) {
        UtilityFunctions::printerr("WARNUNG: RosBridge existiert bereits. Diese Instanz wird gelöscht.");
        queue_free();
        return;
    }

    if (!rclcpp::ok()) {
        rclcpp::init(0, nullptr);
    }

    ros_node = rclcpp::Node::make_shared("godot_ros_bridge");
    
    // Wir nutzen hier eine QoS, die "transient_local" ist, damit RViz 
    // die Daten sofort sieht, sobald es sich verbindet (auch wenn es später startet)
    auto qos = rclcpp::QoS(rclcpp::KeepLast(10)).transient_local();
    lidar_publisher = ros_node->create_publisher<sensor_msgs::msg::LaserScan>("/scan_laeth_bachelor", qos);    
    
    ros_executor = std::make_shared<rclcpp::executors::SingleThreadedExecutor>();
    ros_executor->add_node(ros_node);
    
    is_spinning = true;
    spin_thread = std::thread(&RosBridge::spin_ros2, this);

    tf_broadcaster = std::make_shared<tf2_ros::TransformBroadcaster>(ros_node);
    
    UtilityFunctions::print("ROS2 Bridge Node erfolgreich gestartet und Publisher auf /scan bereit!");
}

void RosBridge::spin_ros2() {
    while (rclcpp::ok() && is_spinning) {
        ros_executor->spin_some(std::chrono::milliseconds(10));
    }
}

void RosBridge::_exit_tree() {
    is_spinning = false;
    if (spin_thread.joinable()) {
        spin_thread.join();
    }
    if (rclcpp::ok()) {
        rclcpp::shutdown();
    }
    UtilityFunctions::print("ROS2 Bridge sauber beendet.");
}

void RosBridge::publish_lidar_data(const std::string& frame_id, const std::vector<float>& ranges,
                                   double angle_min, double angle_max, double angle_increment,
                                   double range_min, double range_max, double update_rate) {
    if (!ros_node || !lidar_publisher) return;

    auto msg = sensor_msgs::msg::LaserScan();
    msg.header.stamp = ros_node->now();
    msg.header.frame_id = frame_id;
    msg.angle_min = angle_min;
    msg.angle_max = angle_max;
    msg.angle_increment = angle_increment;
    msg.time_increment = 0.0;
    msg.scan_time = update_rate;
    msg.range_min = range_min;
    msg.range_max = range_max;
    msg.ranges = ranges;

    lidar_publisher->publish(msg);
}

void RosBridge::_bind_methods() {}