#include "ros2_sensor_base.hpp"
#include "ros_bridge.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

ROS2SensorBase::ROS2SensorBase() {
    time_accumulator = 0.0;
    frequency = 25.0; 
    topic_name = "/sensor/data";
}

ROS2SensorBase::~ROS2SensorBase() {}

void ROS2SensorBase::_bind_methods() {
    // Bind topic name property to the editor inspector
    ClassDB::bind_method(D_METHOD("set_topic_name", "name"), &ROS2SensorBase::set_topic_name);
    ClassDB::bind_method(D_METHOD("get_topic_name"), &ROS2SensorBase::get_topic_name);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "topic_name"), "set_topic_name", "get_topic_name");

    // Bind publishing frequency property to the editor inspector
    ClassDB::bind_method(D_METHOD("set_frequency", "hz"), &ROS2SensorBase::set_frequency);
    ClassDB::bind_method(D_METHOD("get_frequency"), &ROS2SensorBase::get_frequency);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "frequency"), "set_frequency", "get_frequency");
}

void ROS2SensorBase::_ready() {
    // Skip execution while running inside the editor preview
    if (Engine::get_singleton()->is_editor_hint()) {
        return; 
    }

    RosBridge* bridge = RosBridge::get_singleton();
    
    if (bridge != nullptr) {
        ros_node = bridge->get_ros_node();
        
        if (ros_node) {
            setup_publisher(); // Invokes the derived class publisher setup implementation
        }
    } else {
        UtilityFunctions::printerr("ROS2SensorBase Error: RosBridge singleton not found!");
    }
}

// Throttles sensor update calls based on the configured publishing frequency
void ROS2SensorBase::_process(double delta) {
    if (Engine::get_singleton()->is_editor_hint()) return;

    time_accumulator += delta;
    
    // Accumulate time steps to trigger updates precisely at the targeted frequency rate
    if (frequency > 0.0 && time_accumulator >= (1.0 / frequency)) {
        update_data(); // Invokes the derived class data update implementation
        time_accumulator = 0.0;
    } 
}

void ROS2SensorBase::set_topic_name(const String &name) {
    topic_name = name;
}

String ROS2SensorBase::get_topic_name() const {
    return topic_name;
}

void ROS2SensorBase::set_frequency(double hz) {
    frequency = hz;
}

double ROS2SensorBase::get_frequency() const {
    return frequency;
}