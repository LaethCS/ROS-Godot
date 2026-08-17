#include "lidar_sensor.hpp"
#include "ros_bridge.hpp"
#include <godot_cpp/classes/physics_server3d.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/math.hpp> 
#include <algorithm> 
#include <limits>

using namespace godot;

LidarSensor::LidarSensor() {
    random_engine.seed(std::random_device{}());

    // Configure default simulation framework parameters
    topic_name = "/scan";
    frequency = 20.0; // Target update frequency of 20 Hz

    debug_mesh.instantiate();
    debug_material.instantiate();
    
    // Set unshaded red semi-transparent material properties for visualization lines
    debug_material->set_albedo(Color(1.0, 0.0, 0.0, 0.5)); 
    debug_material->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
    debug_material->set_flag(StandardMaterial3D::FLAG_DISABLE_DEPTH_TEST, false);
}

LidarSensor::~LidarSensor() {}

void LidarSensor::_ready() { 
    if (Engine::get_singleton()->is_editor_hint()) return;

    // 1. Establish core ROS2 framework connection through the base class
    ROS2SensorBase::_ready();

    // 2. Compute ray directions and allocate debug rendering instances
    initialize_ray_angles();

    debug_mesh_instance = memnew(MeshInstance3D);
    debug_mesh_instance->set_mesh(debug_mesh);
    debug_mesh_instance->set_material_override(debug_material);
    debug_mesh_instance->set_as_top_level(true); 
    add_child(debug_mesh_instance);
}

// Dynamically creates a ROS2 publisher for LaserScan messages using inspector-defined parameters
void LidarSensor::setup_publisher() {
    if (ros_node) {
        lidar_publisher = ros_node->create_publisher<sensor_msgs::msg::LaserScan>(topic_name.utf8().get_data(), 10);
    }
}

// Executes raycasting queries across all scan angles, computes distances with noise, and publishes LaserScan telemetry
void LidarSensor::update_data() {
    PhysicsDirectSpaceState3D *space_state = get_world_3d()->get_direct_space_state();
    if (!space_state || !lidar_publisher) return;

    Transform3D global_transform = get_global_transform();
    Vector3 origin = global_transform.origin;

    noise_distribution = std::normal_distribution<double>(0.0, noise_stddev);
    std::vector<float> scan_ranges(ray_count, 0.0f);

    if (show_debug_laser && debug_mesh.is_valid()) {
        debug_mesh->clear_surfaces();
        debug_mesh->surface_begin(Mesh::PRIMITIVE_LINES);
    }

    // Iterate through all individual laser rays to evaluate physics collisions
    for (int i = 0; i < ray_count; ++i) {
        Vector3 global_dir = global_transform.basis.xform(local_ray_directions[i]).normalized();
        Vector3 target_pos = origin + (global_dir * range_max);

        Ref<PhysicsRayQueryParameters3D> query = PhysicsRayQueryParameters3D::create(origin, target_pos);
        query->set_collision_mask(1); 

        Dictionary result = space_state->intersect_ray(query);
        Vector3 draw_target = target_pos; 

        if (result.is_empty()) {
            scan_ranges[i] = std::numeric_limits<float>::infinity();
        } else {
            Vector3 hit_point = result["position"];
            draw_target = hit_point; 
            double true_distance = origin.distance_to(hit_point);

            if (true_distance < range_min) {
                scan_ranges[i] = std::numeric_limits<float>::infinity();
            } else {
                double noise = noise_distribution(random_engine);
                double noisy_distance = true_distance + noise;
                noisy_distance = std::clamp(noisy_distance, range_min, range_max);
                scan_ranges[i] = static_cast<float>(noisy_distance);
            }
        }

        if (show_debug_laser && debug_mesh.is_valid()) {
            debug_mesh->surface_add_vertex(origin);      
            debug_mesh->surface_add_vertex(draw_target); 
        }
    }

    if (show_debug_laser && debug_mesh.is_valid()) {
        debug_mesh->surface_end();
    }

    // Populate standard ROS2 LaserScan message structure and broadcast over DDS network
    auto msg = sensor_msgs::msg::LaserScan();
    msg.header.stamp = ros_node->now();
    msg.header.frame_id = "lidar_link";
    msg.angle_min = angle_min;
    msg.angle_max = angle_max;
    msg.angle_increment = angle_increment;
    msg.time_increment = 0.0;
    msg.scan_time = (1.0 / frequency);
    msg.range_min = range_min;
    msg.range_max = range_max;
    msg.ranges = scan_ranges;

    lidar_publisher->publish(msg);

    // Broadcast coordinate frame transformation (TF) linking base structure to the lidar sensor node
    if (RosBridge::get_singleton()) {
        RosBridge::get_singleton()->send_tf_transform("base_link", "lidar_link", global_transform);
    }
}

// Calculates spatial ray vectors and total ray counts based on configured field-of-view and angular resolution
void LidarSensor::initialize_ray_angles() {
    local_ray_directions.clear();
    
    if (angle_increment_degrees <= 0.001) {
        angle_increment_degrees = 0.001;
    }
    if (fov_degrees <= 1.0) {
        fov_degrees = 1.0;
    }

    double fov_rad = Math::deg_to_rad(fov_degrees);
    angle_increment = Math::deg_to_rad(angle_increment_degrees);
    
    angle_min = -fov_rad / 2.0;
    angle_max = fov_rad / 2.0;

    ray_count = static_cast<int>((angle_max - angle_min) / angle_increment) + 1;
    local_ray_directions.reserve(ray_count);

    for (int i = 0; i < ray_count; ++i) {
        double current_angle = angle_min + (i * angle_increment);
        Vector3 dir = Vector3(-std::sin(current_angle), 0.0, -std::cos(current_angle));
        local_ray_directions.push_back(dir);
    }
}

void LidarSensor::set_fov_degrees(double p_val) {
    fov_degrees = p_val;
    initialize_ray_angles(); 
}

void LidarSensor::set_angle_increment_degrees(double p_val) {
    angle_increment_degrees = p_val;
    initialize_ray_angles(); 
}

void LidarSensor::set_show_debug_laser(bool p_val) {
    show_debug_laser = p_val;
    if (!show_debug_laser && debug_mesh.is_valid()) {
        debug_mesh->clear_surfaces(); 
    }
}

void LidarSensor::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_fov_degrees", "p_val"), &LidarSensor::set_fov_degrees);
    ClassDB::bind_method(D_METHOD("get_fov_degrees"), &LidarSensor::get_fov_degrees);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "fov_degrees", PROPERTY_HINT_RANGE, "10.0,360.0,0.5"), "set_fov_degrees", "get_fov_degrees");

    ClassDB::bind_method(D_METHOD("set_angle_increment_degrees", "p_val"), &LidarSensor::set_angle_increment_degrees);
    ClassDB::bind_method(D_METHOD("get_angle_increment_degrees"), &LidarSensor::get_angle_increment_degrees);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angle_increment_degrees", PROPERTY_HINT_RANGE, "0.05,5.0,0.01"), "set_angle_increment_degrees", "get_angle_increment_degrees");

    ClassDB::bind_method(D_METHOD("set_noise_stddev", "p_val"), &LidarSensor::set_noise_stddev);
    ClassDB::bind_method(D_METHOD("get_noise_stddev"), &LidarSensor::get_noise_stddev);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "noise_stddev"), "set_noise_stddev", "get_noise_stddev");

    ClassDB::bind_method(D_METHOD("set_range_max", "p_val"), &LidarSensor::set_range_max);
    ClassDB::bind_method(D_METHOD("get_range_max"), &LidarSensor::get_range_max);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "range_max"), "set_range_max", "get_range_max");

    ClassDB::bind_method(D_METHOD("set_show_debug_laser", "p_val"), &LidarSensor::set_show_debug_laser);
    ClassDB::bind_method(D_METHOD("get_show_debug_laser"), &LidarSensor::get_show_debug_laser);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_debug_laser"), "set_show_debug_laser", "get_show_debug_laser");
}