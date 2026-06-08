#include "test_node.h"
#include <godot_cpp/variant/utility_functions.hpp>

// NEU: Die ROS2 Bibliothek einbinden
#include <rclcpp/rclcpp.hpp>

using namespace godot;

TestNode::TestNode() {}
TestNode::~TestNode() {}

void TestNode::_bind_methods() {}

void TestNode::_ready() {
    UtilityFunctions::print("TestNode gestartet. Versuche ROS2 zu initialisieren...");

    // 1. ROS2 initialisieren (0 und nullptr, da wir keine Kommandozeilen-Argumente von Godot übergeben)
    if (!rclcpp::ok()) {
        rclcpp::init(0, nullptr);
        UtilityFunctions::print("ROS2 rclcpp erfolgreich initialisiert!");
    }

    // 2. Einen echten ROS2-Node im Speicher erstellen
    auto ros_node = rclcpp::Node::make_shared("godot_test_node");

    // 3. Einen Text über das ROS2-interne Logging-System ausgeben
    RCLCPP_INFO(ros_node->get_logger(), "Hallo aus der Godot-Engine! Ich bin jetzt im DDS-Netzwerk.");
}