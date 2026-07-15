#include "ros2_publisher.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

// Include für deine RosBridge, um den ROS-Node zu holen
#include "ros_bridge.hpp" 

using namespace godot;

void ROS2Publisher::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_topic_name", "name"), &ROS2Publisher::set_topic_name);
    ClassDB::bind_method(D_METHOD("get_topic_name"), &ROS2Publisher::get_topic_name);
    ClassDB::bind_method(D_METHOD("publish_message", "message"), &ROS2Publisher::publish_message);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "topic_name"), "set_topic_name", "get_topic_name");
}

ROS2Publisher::ROS2Publisher() {
    topic_name = "/my_generic_topic";
    publisher_ = nullptr; // Sicherstellen, dass er anfangs leer ist
}

ROS2Publisher::~ROS2Publisher() {}

void ROS2Publisher::_ready() {
    // 1. Instanz-Adresse drucken
    UtilityFunctions::print("--- DEBUG: _ready() aufgerufen für Instanz: ", (long)this, " ---");

    auto bridge = RosBridge::get_singleton();
    if (bridge) {
        auto ros_node = bridge->get_ros_node();
        if (ros_node) {
            publisher_ = ros_node->create_publisher<std_msgs::msg::String>(topic_name.utf8().get_data(), 10);
            UtilityFunctions::print("ERFOLG: Publisher erstellt in Instanz: ", (long)this);
        }
    }
}

void ROS2Publisher::set_topic_name(const String &name) {
    topic_name = name;
}

String ROS2Publisher::get_topic_name() const {
    return topic_name;
}

void ROS2Publisher::publish_message(const String &message) {
    // Falls der Publisher aus irgendeinem Grund noch nicht existiert,
    // versuchen wir ihn hier sofort zu erzeugen (Lazy Initialization)
    if (publisher_ == nullptr) {
        UtilityFunctions::print("Publisher war NULL, versuche Initialisierung...");
        auto bridge = RosBridge::get_singleton();
        if (bridge) {
            auto ros_node = bridge->get_ros_node();
            if (ros_node) {
                publisher_ = ros_node->create_publisher<std_msgs::msg::String>(topic_name.utf8().get_data(), 10);
            }
        }
    }

    if (publisher_ != nullptr) {
        std_msgs::msg::String ros_msg;
        ros_msg.data = message.utf8().get_data();
        publisher_->publish(ros_msg);
    } else {
        UtilityFunctions::printerr("Immer noch NULL!");
    }
}

/*void ROS2Publisher::publish_message(const String &message) {
    // 2. Instanz-Adresse drucken
    UtilityFunctions::print("--- DEBUG: publish_message() aufgerufen für Instanz: ", (long)this, " ---");

    if (publisher_ != nullptr) {
        std_msgs::msg::String ros_msg;
        ros_msg.data = message.utf8().get_data();
        publisher_->publish(ros_msg);
        UtilityFunctions::print("Gesendet an ROS2 von Instanz: ", (long)this);
    } else {
        UtilityFunctions::printerr("FEHLER: Publisher ist NULL in Instanz: ", (long)this);
    }
}*/