#ifndef TEST_SENSOR_H
#define TEST_SENSOR_H

#include <godot_cpp/classes/ray_cast3d.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float32.hpp>

namespace godot {

class TestSensor : public RayCast3D {
    GDCLASS(TestSensor, RayCast3D)

private:
    std::shared_ptr<rclcpp::Node> ros_node;
    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr publisher;
    double time_passed = 0.0; // Unser neuer Timer

protected:
    static void _bind_methods();

public:
    TestSensor();
    ~TestSensor();

    void _ready() override;
    void _physics_process(double delta) override; // Hier gewechselt!
};

}

#endif