#include "ros_bridge.hpp"
#include <godot_cpp/variant/utility_functions.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include "ros2_message_builder.hpp"

using namespace godot;

RosBridge *RosBridge::singleton = nullptr;

RosBridge::RosBridge() {
    if (singleton == nullptr) {
        singleton = this;
    }

    UtilityFunctions::print("RosBridge: Initialisiere zentralen ROS2-Knoten...");

    if (!rclcpp::ok()) {
        rclcpp::init(0, nullptr);
    }

    ros_node = rclcpp::Node::make_shared("godot_ros_bridge");
    
    auto qos = rclcpp::QoS(rclcpp::KeepLast(10)).reliable();
    
    // Transform Broadcaster Setup
    tf_broadcaster = std::make_shared<tf2_ros::TransformBroadcaster>(ros_node);

    // Subscriber Setup für /cmd_vel
    cmd_vel_subscriber = ros_node->create_subscription<geometry_msgs::msg::Twist>(
        "/cmd_vel", 
        qos, 
        std::bind(&RosBridge::cmd_vel_callback, this, std::placeholders::_1)
    );

    ros_executor = std::make_shared<rclcpp::executors::SingleThreadedExecutor>();
    ros_executor->add_node(ros_node);
    
    is_spinning = true;
    spin_thread = std::thread(&RosBridge::spin_ros2, this);
    
    UtilityFunctions::print("ROS2 Bruecke erfolgreich hochgefahren!");
}

RosBridge::~RosBridge() {
    is_spinning = false;
    
    if (ros_executor) {
        ros_executor->cancel();
    }
    
    if (spin_thread.joinable()) {
        spin_thread.join();
    }
    if (rclcpp::ok()) {
        rclcpp::shutdown();
    }
    if (singleton == this) {
        singleton = nullptr;
    }
    UtilityFunctions::print("ROS2 Bridge sauber beendet.");
}

RosBridge *RosBridge::get_singleton() {
    return singleton;
}

std::shared_ptr<rclcpp::Node> RosBridge::get_ros_node() const {
    return ros_node;
}

void RosBridge::_ready() {}
void RosBridge::_exit_tree() {}

void RosBridge::cmd_vel_callback(geometry_msgs::msg::Twist::ConstSharedPtr msg) {
    std::lock_guard<std::mutex> lock(velocity_mutex);
    linear_velocity_x = msg->linear.x;
    angular_velocity_z = msg->angular.z;
}

double RosBridge::get_linear_velocity() const {
    std::lock_guard<std::mutex> lock(velocity_mutex);
    return linear_velocity_x;
}

double RosBridge::get_angular_velocity() const {
    std::lock_guard<std::mutex> lock(velocity_mutex);
    return angular_velocity_z;
}

void RosBridge::spin_ros2() {
    ros_executor->spin(); 
}

void RosBridge::send_tf_transform(const std::string& frame_id, Transform3D global_transform) {
    if (!ros_node || !tf_broadcaster) return;

    rclcpp::Time current_time = ros_node->now();
    geometry_msgs::msg::TransformStamped t;
    
    t.header.stamp = current_time;
    t.header.frame_id = "map"; 
    t.child_frame_id = frame_id; 

    t.transform = ROS2MessageBuilder::godot_to_ros_transform(global_transform);

    tf_broadcaster->sendTransform(t);
}

void RosBridge::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_linear_velocity"), &RosBridge::get_linear_velocity);
    ClassDB::bind_method(D_METHOD("get_angular_velocity"), &RosBridge::get_angular_velocity);
}