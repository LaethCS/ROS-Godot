#include "register_types.h"
#include "ros_bridge.hpp"
#include "ros2_sensor_base.hpp"
#include "lidar_sensor.hpp"
#include "ros2_publisher.hpp"
#include "imu_sensor.hpp"
#include "odom_sensor.hpp"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

// Initializes custom extension classes and registers the central communication bridge as a global engine singleton
void initialize_test_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    
    // Register concrete and abstract node classes with Godot's runtime type database
    ClassDB::register_class<RosBridge>();
    ClassDB::register_abstract_class<ROS2SensorBase>();
    ClassDB::register_class<LidarSensor>(); 
    ClassDB::register_class<ROS2Publisher>();
    ClassDB::register_class<ImuSensor>();
    ClassDB::register_class<OdomSensor>(); 

    // Register RosBridge as a global engine singleton to enable cross-script access via GDScript
    Engine::get_singleton()->register_singleton("RosBridge", memnew(RosBridge));
}

// Cleans up module resources and unregisters engine singletons upon scene shutdown
void uninitialize_test_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    
    // Unregister and free the global singleton instance
    Engine::get_singleton()->unregister_singleton("RosBridge");
}

extern "C" {
// Main entry point for GDExtension library loading and initialization handshake with the Godot engine
GDExtensionBool GDE_EXPORT test_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_test_module);
    init_obj.register_terminator(uninitialize_test_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}