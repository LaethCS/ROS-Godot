#ifndef ROS_BRIDGE_HPP
#define ROS_BRIDGE_HPP

#include <godot_cpp/classes/node.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <tf2_ros/transform_broadcaster.h> 
#include <geometry_msgs/msg/transform_stamped.hpp> 
#include <thread>
#include <memory>

namespace godot {

class RosBridge : public Node {
    GDCLASS(RosBridge, Node)

private:
    static RosBridge *singleton;

    std::shared_ptr<rclcpp::Node> ros_node;
    std::shared_ptr<rclcpp::executors::SingleThreadedExecutor> ros_executor;
    std::thread spin_thread;
    bool is_spinning = false;

    rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr lidar_publisher;
    
    // Der Transform Broadcaster
    std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster;

    void spin_ros2();

protected:
    static void _bind_methods();

public:
    RosBridge();
    ~RosBridge();

    static RosBridge *get_singleton();

    void _ready() override;
    void _exit_tree() override;

    void publish_lidar_data(const std::string& frame_id, const std::vector<float>& ranges,
                            double angle_min, double angle_max, double angle_increment,
                            double range_min, double range_max, double update_rate,
                            Transform3D global_transform); // WICHTIG: Transform3D hinzugefügt
                            
};

}

#endif