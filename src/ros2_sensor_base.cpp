#include "ros2_sensor_base.hpp"
#include "ros_bridge.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

ROS2SensorBase::ROS2SensorBase() {
    frequency = 25.0; 
    topic_name = "/sensor/data";
    throttle.set_interval(1.0 / frequency);
}

ROS2SensorBase::~ROS2SensorBase() {}

void ROS2SensorBase::_bind_methods() {
    // NOTE: The publishing frequency and topic name are deliberately NOT exposed as
    // inspector properties. Derived sensors hardcode their ROS topic and rate so the
    // simulation reflects a fixed physical device (like an industrial LiDAR), matching
    // the behaviour of the IMU and Odom sensors.
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

    // Trigger an update once per configured interval. The Throttle carries surplus
    // time (delta beyond the interval) into the next step instead of dropping it,
    // so the effective publish rate stays accurate regardless of Godot's framerate.
    if (throttle.update(delta)) {
        update_data();
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
    throttle.set_interval((hz > 0.0) ? (1.0 / hz) : 0.0);
}

double ROS2SensorBase::get_frequency() const {
    return frequency;
}