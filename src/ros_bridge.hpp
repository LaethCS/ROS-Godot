#ifndef ROS_BRIDGE_HPP
#define ROS_BRIDGE_HPP

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <geometry_msgs/msg/twist.hpp>
#include <mutex>
#include <thread>
#include <string>

namespace godot {

class RosBridge : public Object {
    GDCLASS(RosBridge, Object)

private:
    static RosBridge *singleton;

    std::shared_ptr<rclcpp::Node> ros_node;
    std::shared_ptr<rclcpp::executors::SingleThreadedExecutor> ros_executor;
    std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_subscriber;

    std::thread spin_thread;
    bool is_spinning;

    mutable std::mutex velocity_mutex;
    double linear_velocity_x = 0.0;
    double angular_velocity_z = 0.0;

    void cmd_vel_callback(geometry_msgs::msg::Twist::ConstSharedPtr msg);
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

    double get_linear_velocity() const;
    double get_angular_velocity() const;

    // Generischer TF-Broadcaster für alle Sensoren im Framework
    void send_tf_transform(const std::string& frame_id, Transform3D global_transform);
};

}

#endif