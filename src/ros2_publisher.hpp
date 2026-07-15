#ifndef ROS2_PUBLISHER_H
#define ROS2_PUBLISHER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>

// WICHTIG: Die ROS2 Includes
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

namespace godot {

class ROS2Publisher : public Node {
    GDCLASS(ROS2Publisher, Node)

private:
    String topic_name;
    
    // HIER IST DER ECHTE PUBLISHER:
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;

protected:
    static void _bind_methods();

public:
    ROS2Publisher();
    ~ROS2Publisher();

    void _ready() override;
    void set_topic_name(const String &name);
    String get_topic_name() const;
    void publish_message(const String &message);
};

} // namespace godot

#endif