#ifndef ROS_BRIDGE_HPP
#define ROS_BRIDGE_HPP

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector3.hpp>     
#include <godot_cpp/variant/quaternion.hpp>   
#include <nav_msgs/msg/odometry.hpp>
#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <geometry_msgs/msg/twist.hpp>
#include <sensor_msgs/msg/imu.hpp>            
#include <mutex>
#include <thread>
#include <string>

namespace godot {
 
// Central middleware bridge managing the ROS2 node lifecycle, asynchronous executor thread,
// velocity command subscriptions, telemetry publishers, and coordinate frame broadcasts.
class RosBridge : public Object {
    GDCLASS(RosBridge, Object)

private:
    static RosBridge *singleton;

    std::shared_ptr<rclcpp::Node> ros_node;
    std::shared_ptr<rclcpp::executors::SingleThreadedExecutor> ros_executor;
    std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_subscriber;
    
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_publisher;
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_publisher;

    std::thread spin_thread;
    bool is_spinning;

    mutable std::mutex velocity_mutex;
    double linear_velocity_x = 0.0;
    double angular_velocity_z = 0.0;

    // Asynchronous callback processing incoming velocity commands from the ROS2 network
    void cmd_vel_callback(geometry_msgs::msg::Twist::ConstSharedPtr msg);

    // Dedicated execution thread running the ROS2 single-threaded executor spin loop
    void spin_ros2();

protected:
    static void _bind_methods();

public:
    RosBridge();
    ~RosBridge();

    static RosBridge *get_singleton();
    std::shared_ptr<rclcpp::Node> get_ros_node() const;

    void _ready();
    void _exit_tree();

    // Thread-safe getters for velocity control commands retrieved from Godot physics loop
    double get_linear_velocity() const;
    double get_angular_velocity() const;

    // Broadcasts dynamic coordinate frame transformations between parent and child links via tf2
    void send_tf_transform(const std::string& parent_frame, const std::string& child_frame, Transform3D global_transform);

    // Publishes IMU sensor telemetry data including linear acceleration, angular velocity, and orientation
    void publish_imu_data(const std::string& frame_id, 
                          const godot::Vector3& linear_acceleration, 
                          const godot::Vector3& angular_velocity, 
                          const godot::Quaternion& orientation);

    // Publishes robot odometry telemetry combining spatial pose coordinates and local twist velocities
    void publish_odometry(const std::string& frame_id, 
                         const std::string& child_frame_id,
                         const godot::Vector3& position, 
                         const godot::Quaternion& orientation,
                         const godot::Vector3& local_linear_velocity,
                         const godot::Vector3& local_angular_velocity);
};

}

#endif