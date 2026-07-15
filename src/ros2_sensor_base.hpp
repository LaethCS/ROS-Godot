#ifndef ROS2_SENSOR_BASE_HPP
#define ROS2_SENSOR_BASE_HPP

#include <godot_cpp/classes/node3d.hpp>
#include <rclcpp/rclcpp.hpp>

namespace godot {

class ROS2SensorBase : public Node3D {
    GDCLASS(ROS2SensorBase, Node3D)

protected:
    static void _bind_methods();

public:
    ROS2SensorBase();
    
    // WICHTIG: Das 'virtual' löst den Linker-Fehler!
    virtual ~ROS2SensorBase();

    void _ready() override;
    void _process(double delta) override;
    
    // Zwingt die Kind-Klasse, die Logik für das Daten-Sammeln zu definieren
    virtual void update_data() = 0;

    void set_topic_name(const String &name);
    String get_topic_name() const;
    void set_frequency(double hz);
    double get_frequency() const;

protected:
    String topic_name = "/sensor/data";
    double frequency = 25.0; 
    double time_accumulator = 0.0;
    
    // Der geteilte ROS2-Knoten aus der RosBridge
    std::shared_ptr<rclcpp::Node> ros_node;
    
    // Zwingt die Kind-Klasse, ihren spezifischen Publisher zu definieren
    virtual void setup_publisher() = 0; 
};

}

#endif