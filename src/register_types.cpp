#include "register_types.h"
#include "ros_bridge.hpp"
#include "ros2_sensor_base.hpp"
#include "test_sensor.h"
#include "lidar_sensor.hpp"
#include "ros2_publisher.hpp"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/engine.hpp> // WICHTIG für Singleton

using namespace godot;

void initialize_test_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    
    // 1. Klassen registrieren
    ClassDB::register_class<RosBridge>();
    ClassDB::register_abstract_class<ROS2SensorBase>();
    ClassDB::register_class<TestSensor>(); 
    ClassDB::register_class<LidarSensor>(); 
    ClassDB::register_class<ROS2Publisher>();

    // 2. RosBridge als Singleton registrieren, damit GDScript darauf zugreifen kann!
    Engine::get_singleton()->register_singleton("RosBridge", memnew(RosBridge));
}

void uninitialize_test_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    
    // Singleton wieder freigeben
    Engine::get_singleton()->unregister_singleton("RosBridge");
}

extern "C" {
// Initialisierungs-Einstiegspunkt für GDExtension
GDExtensionBool GDE_EXPORT test_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_test_module);
    init_obj.register_terminator(uninitialize_test_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}