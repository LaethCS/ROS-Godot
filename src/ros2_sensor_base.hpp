#ifndef ROS2_SENSOR_BASE_HPP
#define ROS2_SENSOR_BASE_HPP

#include <godot_cpp/classes/node3d.hpp>
#include <rclcpp/rclcpp.hpp>

namespace godot {

/**
 * @brief Abstract base class providing common infrastructure for all ROS2 sensors 
 * within the Godot simulation environment, handling lifecycle, timing, and node sharing.
 */
class ROS2SensorBase : public Node3D {
    GDCLASS(ROS2SensorBase, Node3D)

protected:
    // Registers class properties and methods with Godot's runtime type database
    static void _bind_methods();

public:
    ROS2SensorBase();
    virtual ~ROS2SensorBase();

    // Node initialization lifecycle hook resolving the ROS2 node from the singleton bridge
    void _ready() override;

    // Per-frame processing loop accumulating time steps to throttle sensor updates precisely
    void _process(double delta) override;
    
    /**
     * @brief Pure virtual method enforcing derived sensor implementations 
     * to define specific data collection and publishing logic.
     */ 
    virtual void update_data() = 0;

    // Getters and setters for configurable sensor topic names and publishing frequencies
    void set_topic_name(const String &name);
    String get_topic_name() const;
    void set_frequency(double hz);
    double get_frequency() const;

protected:
    String topic_name = "/sensor/data";
    double frequency = 25.0; 
    double time_accumulator = 0.0;
    
    // Shared ROS2 node reference obtained from the singleton bridge
    std::shared_ptr<rclcpp::Node> ros_node;
    
    /**
     * @brief Pure virtual method enforcing derived sensor classes 
     * to initialize their specific ROS2 publisher instances.
     */
    virtual void setup_publisher() = 0; 
};

} // namespace godot

#endif // ROS2_SENSOR_BASE_HPP