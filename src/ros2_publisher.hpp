#ifndef ROS2_PUBLISHER_HPP
#define ROS2_PUBLISHER_HPP

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>

// ROS2 core client library and standard string message definitions
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

namespace godot {

/**
 * @brief A generic ROS2 publisher node exposing properties to the Godot Inspector
 * to broadcast string messages across the DDS network without requiring C++ code.
 */
class ROS2Publisher : public Node {
    GDCLASS(ROS2Publisher, Node)

private:
    String topic_name;
    
    // Underlying ROS2 publisher instance for string messages
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;

protected:
    // Registers class methods and inspector properties with Godot's runtime database
    static void _bind_methods();

public:
    ROS2Publisher();
    ~ROS2Publisher(); 

    // Node lifecycle override initializing the ROS2 publisher upon tree entry
    void _ready() override;

    // Sets the target ROS2 topic name via inspector or script
    void set_topic_name(const String &name);

    // Retrieves the currently configured ROS2 topic name
    String get_topic_name() const;

    // Publishes a string message to the configured ROS2 topic, utilizing lazy initialization if needed
    void publish_message(const String &message);
};

} // namespace godot

#endif // ROS2_PUBLISHER_HPP