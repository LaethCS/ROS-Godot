#ifndef LIDAR_SENSOR_HPP
#define LIDAR_SENSOR_HPP

#include "ros2_sensor_base.hpp"
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <random>
#include <vector>

namespace godot {

// Virtual LiDAR sensor node inheriting from the base framework class,
// performing physics-based raycasts to simulate 2D LaserScan data streams in ROS2.
class LidarSensor : public ROS2SensorBase {
    GDCLASS(LidarSensor, ROS2SensorBase);

private: 
    double fov_degrees = 270.0;             // Total field of view angle in degrees
    double angle_increment_degrees = 0.25;  // Angular resolution between consecutive laser rays

    double angle_min = 0.0;                 // Minimum scan angle in radians
    double angle_max = 0.0;                 // Maximum scan angle in radians
    double angle_increment = 0.0;           // Calculated ray angle increment in radians
    
    double range_min = 0.1;                 // Minimum detectable distance threshold
    double range_max = 30.0;                // Maximum detectable distance range limit
    double noise_stddev = 0.01;             // Standard deviation for simulated Gaussian measurement noise
    bool noise_enabled = true;              // Master toggle: enables/disables simulated measurement noise

    bool show_debug_laser = false;          // Toggle flag to visualize active rays in the 3D viewport
    MeshInstance3D *debug_mesh_instance = nullptr;
    Ref<ImmediateMesh> debug_mesh;
    Ref<StandardMaterial3D> debug_material;

    std::default_random_engine random_engine;
    std::normal_distribution<double> noise_distribution;

    std::vector<Vector3> local_ray_directions; // Precomputed unit vectors for each individual scan ray
    int ray_count = 0;                         // Total number of rays evaluated per scan cycle

    // Dedicated ROS2 publisher for broadcasting LaserScan telemetry data
    rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr lidar_publisher;

    // Recalculates ray angles and direction vectors when scan properties change
    void initialize_ray_angles();

protected:
    // Registers methods and inspectable properties with Godot's runtime type system
    static void _bind_methods();

public:
    LidarSensor();
    ~LidarSensor();

    // Node initialization hook handling debug visualization setup and base configuration
    void _ready() override;
    
    // Framework interface implementations for publisher creation and periodic data updates
    void setup_publisher() override;
    void update_data() override;

    // Getters and setters for configurable sensor properties exposed in the editor inspector
    void set_fov_degrees(double p_val);
    double get_fov_degrees() const { return fov_degrees; }

    void set_angle_increment_degrees(double p_val);
    double get_angle_increment_degrees() const { return angle_increment_degrees; }

    void set_noise_stddev(double p_val) { noise_stddev = p_val; }
    double get_noise_stddev() const { return noise_stddev; }

    void set_noise_enabled(bool p_val) { noise_enabled = p_val; }
    bool get_noise_enabled() const { return noise_enabled; }
    
    void set_range_max(double p_val) { range_max = p_val; }
    double get_range_max() const { return range_max; }

    void set_show_debug_laser(bool p_val);
    bool get_show_debug_laser() const { return show_debug_laser; }
};

}

#endif