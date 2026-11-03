#include "ros_bridge.hpp"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include "ros2_message_builder.hpp"

using namespace godot;

RosBridge *RosBridge::singleton = nullptr;

// Initializes the ROS2 node, sets up Quality of Service profiles, configures subscribers,
// publishers, and spawns a background thread for asynchronous ROS2 executor execution.
RosBridge::RosBridge() {
    if (singleton == nullptr) {
        singleton = this;
    }

    // When the plugin is loaded inside the Godot editor, do NOT create the ROS2
    // node. Otherwise the editor process would join the ROS2 graph as an extra
    // "godot_ros_bridge" node (double publishers/subscriptions on /cmd_vel,
    // /imu/data, /odom, /tf). The bridge is only initialized at runtime.
    if (Engine::get_singleton()->is_editor_hint()) {
        is_spinning = false;
        return;
    }

    if (!rclcpp::ok()) {
        rclcpp::init(0, nullptr);
    }  

    ros_node = rclcpp::Node::make_shared("godot_ros_bridge");

    // RELIABLE QoS is used across the bridge. This matches the default reliability
    // that rviz2 (Humble) uses for Odometry / Imu displays, so pre-usable topics
    // (/cmd_vel, /imu/data, /odom) stay compatible with a large set of tools.
    // (REP-2003 suggests best_effort for some sensor streams, but rviz2 Humble
    //  subscribes those displays with RELIABLE, so we follow that to stay visible.)
    auto qos = rclcpp::QoS(rclcpp::KeepLast(10)).reliable();

    // Setup transform broadcaster for dynamic coordinate frames (TF)
    tf_broadcaster = std::make_shared<tf2_ros::TransformBroadcaster>(ros_node);

    // Setup subscriber for velocity commands (/cmd_vel) controlling the robot
    cmd_vel_subscriber = ros_node->create_subscription<geometry_msgs::msg::Twist>(
        "/cmd_vel", 
        qos, 
        std::bind(&RosBridge::cmd_vel_callback, this, std::placeholders::_1)
    );

    // Initialize telemetry publishers for IMU and odometry data streams (RELIABLE).
    imu_publisher = ros_node->create_publisher<sensor_msgs::msg::Imu>("/imu/data", qos);
    odom_publisher = ros_node->create_publisher<nav_msgs::msg::Odometry>("/odom", qos);

    ros_executor = std::make_shared<rclcpp::executors::SingleThreadedExecutor>();
    ros_executor->add_node(ros_node);
    
    is_spinning = true;
    spin_thread = std::thread(&RosBridge::spin_ros2, this);
}

// Cleans up the background execution thread, cancels active executors, and shuts down ROS2 context safely.
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
}

RosBridge *RosBridge::get_singleton() {
    return singleton;
}

std::shared_ptr<rclcpp::Node> RosBridge::get_ros_node() const {
    return ros_node;
}

void RosBridge::_ready() {}
void RosBridge::_exit_tree() {}

// Thread-safe callback executed asynchronously when a new Twist velocity message arrives from the network.
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

// Background loop executing the single-threaded ROS2 executor spin process.
void RosBridge::spin_ros2() {
    ros_executor->spin(); 
}

// Broadcasts real-time coordinate frame transformations between parent and child links using tf2.
void RosBridge::send_tf_transform(const std::string& parent_frame, const std::string& child_frame, Transform3D global_transform) {
    if (!ros_node || !tf_broadcaster) return;

    rclcpp::Time current_time = ros_node->now();
    geometry_msgs::msg::TransformStamped t;
    
    t.header.stamp = current_time;
    t.header.frame_id = parent_frame; 
    t.child_frame_id = child_frame; 

    t.transform = ROS2MessageBuilder::godot_to_ros_transform(global_transform);

    tf_broadcaster->sendTransform(t);
}

// Publishes IMU telemetry data handling coordinate system mapping between Godot and ROS standards.
void RosBridge::publish_imu_data(const std::string& frame_id, 
                                 const godot::Vector3& linear_acceleration, 
                                 const godot::Vector3& angular_velocity, 
                                 const godot::Quaternion& orientation) {
    if (!ros_node || !imu_publisher) return;

    sensor_msgs::msg::Imu msg;
    msg.header.stamp = ros_node->now();
    msg.header.frame_id = frame_id;

    // Mathematical conversion from Godot engine axes to ROS standard frame conventions
    msg.linear_acceleration.x = -linear_acceleration.z;
    msg.linear_acceleration.y = -linear_acceleration.x;
    msg.linear_acceleration.z = linear_acceleration.y;

    msg.angular_velocity.x = -angular_velocity.z;
    msg.angular_velocity.y = -angular_velocity.x;
    msg.angular_velocity.z = angular_velocity.y;

    msg.orientation.x = -orientation.z;
    msg.orientation.y = -orientation.x;
    msg.orientation.z = orientation.y;
    msg.orientation.w = orientation.w;

    msg.orientation_covariance[0] = 0.01;
    msg.orientation_covariance[4] = 0.01;
    msg.orientation_covariance[8] = 0.01;

    msg.angular_velocity_covariance[0] = 0.01;
    msg.angular_velocity_covariance[4] = 0.01;
    msg.angular_velocity_covariance[8] = 0.01;

    msg.linear_acceleration_covariance[0] = 0.01;
    msg.linear_acceleration_covariance[4] = 0.01;
    msg.linear_acceleration_covariance[8] = 0.01;

    imu_publisher->publish(msg);
}

// Publishes robot odometry telemetry combining spatial positioning and twist velocity vectors.
void RosBridge::publish_odometry(const std::string& frame_id, 
                                const std::string& child_frame_id,
                                const godot::Vector3& position, 
                                const godot::Quaternion& orientation,
                                const godot::Vector3& local_linear_velocity,
                                const godot::Vector3& local_angular_velocity) {
    if (!ros_node || !odom_publisher) return;

    nav_msgs::msg::Odometry msg;
    msg.header.stamp = ros_node->now();
    msg.header.frame_id = frame_id;         
    msg.child_frame_id = child_frame_id;   

    // Map spatial coordinates and local velocities to ROS standard conventions
    msg.pose.pose.position.x = -position.z;
    msg.pose.pose.position.y = -position.x;
    msg.pose.pose.position.z = position.y;

    msg.pose.pose.orientation.x = -orientation.z;
    msg.pose.pose.orientation.y = -orientation.x;
    msg.pose.pose.orientation.z = orientation.y;
    msg.pose.pose.orientation.w = orientation.w;

    msg.twist.twist.linear.x = -local_linear_velocity.z;
    msg.twist.twist.linear.y = -local_linear_velocity.x;
    msg.twist.twist.linear.z = local_linear_velocity.y;

    msg.twist.twist.angular.x = -local_angular_velocity.z;
    msg.twist.twist.angular.y = -local_angular_velocity.x;
    msg.twist.twist.angular.z = local_angular_velocity.y;

    for (int i = 0; i < 36; i += 7) {
        msg.pose.covariance[i] = 0.01;
        msg.twist.covariance[i] = 0.01;
    }

    odom_publisher->publish(msg);
}

void RosBridge::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_linear_velocity"), &RosBridge::get_linear_velocity);
    ClassDB::bind_method(D_METHOD("get_angular_velocity"), &RosBridge::get_angular_velocity);
}