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

class LidarSensor : public ROS2SensorBase {
    GDCLASS(LidarSensor, ROS2SensorBase);

private:
    double fov_degrees = 270.0;             
    double angle_increment_degrees = 0.25;  

    double angle_min = 0.0;
    double angle_max = 0.0;
    double angle_increment = 0.0;
    
    double range_min = 0.1;
    double range_max = 30.0;
    double noise_stddev = 0.01;

    bool show_debug_laser = false;
    MeshInstance3D *debug_mesh_instance = nullptr;
    Ref<ImmediateMesh> debug_mesh;
    Ref<StandardMaterial3D> debug_material;

    std::default_random_engine random_engine;
    std::normal_distribution<double> noise_distribution;

    std::vector<Vector3> local_ray_directions;
    int ray_count = 0;

    // Spezifischer ROS2 Publisher für diesen Sensor-Knoten
    rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr lidar_publisher;

    void initialize_ray_angles();

protected:
    static void _bind_methods();

public:
    LidarSensor();
    ~LidarSensor();

    void _ready() override;
    
    // Framework-Implementierungen
    void setup_publisher() override;
    void update_data() override;

    void set_fov_degrees(double p_val);
    double get_fov_degrees() const { return fov_degrees; }

    void set_angle_increment_degrees(double p_val);
    double get_angle_increment_degrees() const { return angle_increment_degrees; }

    void set_noise_stddev(double p_val) { noise_stddev = p_val; }
    double get_noise_stddev() const { return noise_stddev; }
    
    void set_range_max(double p_val) { range_max = p_val; }
    double get_range_max() const { return range_max; }

    void set_show_debug_laser(bool p_val);
    bool get_show_debug_laser() const { return show_debug_laser; }
};

}

#endif