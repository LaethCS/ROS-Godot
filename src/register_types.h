#ifndef ROS2_GDEXTENSION_REGISTER_TYPES_H
#define ROS2_GDEXTENSION_REGISTER_TYPES_H

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

// Deklarationen für die Modul-Initialisierung
void initialize_test_module(ModuleInitializationLevel p_level);
void uninitialize_test_module(ModuleInitializationLevel p_level);

#endif