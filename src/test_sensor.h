#ifndef TEST_SENSOR_H
#define TEST_SENSOR_H

#include <godot_cpp/classes/ray_cast3d.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float32.hpp> 

namespace godot {

class TestSensor : public RayCast3D {
    GDCLASS(TestSensor, RayCast3D);

private:
    rclcpp::Node::SharedPtr ros_node;
    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr publisher;

protected:
    static void _bind_methods();

public:
    TestSensor();
    ~TestSensor();

    void _ready() override;
    void _process(double delta) override; 
};

}

#endif