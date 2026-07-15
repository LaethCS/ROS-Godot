#include "test_sensor.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

TestSensor::TestSensor() {
    raycast = nullptr;
}

TestSensor::~TestSensor() {}

void TestSensor::_bind_methods() {
    // Da wir von ROS2SensorBase erben, haben wir topic_name und frequency 
    // bereits als Property im Inspector! Wir müssen hier nichts mehr binden.
}

void TestSensor::_ready() {
    // 1. Zuerst die Basis-Klasse initialisieren (sucht RosBridge, etc.)
    ROS2SensorBase::_ready();

    // 2. Den RayCast finden, der als Kind in der Szene liegen sollte
    raycast = Object::cast_to<RayCast3D>(get_node_or_null("RayCast3D"));
    if (!raycast) {
        UtilityFunctions::printerr("TestSensor Fehler: Bitte füge einen RayCast3D Knoten als Kind hinzu!");
    }
}

void TestSensor::setup_publisher() {
    // Diese Methode wird von der Basis-Klasse automatisch gerufen, 
    // sobald die RosBridge bereit ist.
    if (ros_node) {
        float_publisher = ros_node->create_publisher<std_msgs::msg::Float32>(topic_name.utf8().get_data(), 10);
    }
}

void TestSensor::update_data() {
    // Diese Methode wird von der Basis-Klasse exakt im Takt (25Hz) aufgerufen.
    if (!raycast || !float_publisher) return;

    auto msg = std_msgs::msg::Float32();

    // Physik-Update forcieren
    raycast->force_raycast_update();

    if (raycast->is_colliding()) {
        Vector3 my_pos = get_global_position();
        Vector3 hit_pos = raycast->get_collision_point();
        msg.data = my_pos.distance_to(hit_pos);
    } else {
        msg.data = -1.0;
    }

    float_publisher->publish(msg);
}