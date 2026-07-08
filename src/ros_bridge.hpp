#ifndef ROS_BRIDGE_HPP
#define ROS_BRIDGE_HPP

#include <godot_cpp/classes/node.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <thread>
#include <memory>
#include <string>

namespace tf2_ros {
    class TransformBroadcaster;
}

namespace godot {

class RosBridge : public Node {
    GDCLASS(RosBridge, Node);

private:
    // Der einzige Zeiger auf unsere Singleton-Instanz
    static RosBridge *singleton;

    // ROS2 Kernkomponenten
    std::shared_ptr<rclcpp::Node> ros_node;
    rclcpp::executors::SingleThreadedExecutor::SharedPtr ros_executor;
    std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster;
    
    // Der Hintergrund-Thread für ROS2 (damit Godot nicht blockiert wird)
    std::thread spin_thread;
    bool is_spinning = false;

    // --- PUBLISHER ---
    // Ein Publisher speziell für Lidar-Daten
    rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr lidar_publisher;

    void spin_ros2();

protected:
    static void _bind_methods();

public:
    RosBridge();
    ~RosBridge();

    void _ready() override;
    void _exit_tree() override; // Wird aufgerufen, wenn das Spiel beendet wird

    // Die globale Zugriffsfunktion (Singleton)
    static RosBridge *get_singleton();

    // Die Schnittstelle für deine Lidar-Sensoren
    void publish_lidar_data(
        const std::string& frame_id, 
        const std::vector<float>& ranges,
        double angle_min, double angle_max, double angle_increment,
        double range_min, double range_max, double update_rate
    );
};

}

#endif