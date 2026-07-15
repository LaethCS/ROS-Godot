#ifndef TEST_SENSOR_HPP
#define TEST_SENSOR_HPP

#include "ros2_sensor_base.hpp" // Unsere Basis-Klasse
#include <godot_cpp/classes/ray_cast3d.hpp>
#include <std_msgs/msg/float32.hpp>

namespace godot {

class TestSensor : public ROS2SensorBase {
    GDCLASS(TestSensor, ROS2SensorBase) // Erbt von ROS2SensorBase

protected:
    static void _bind_methods();

public:
    TestSensor();
    ~TestSensor();

    void _ready() override;
    
    // Die Pflicht-Methoden der Basis-Klasse
    void setup_publisher() override;
    void update_data() override;

private:
    RayCast3D* raycast;
    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr float_publisher;
};

}

#endif