#ifndef IMU_SENSOR_HPP
#define IMU_SENSOR_HPP

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {

// Virtual Inertial Measurement Unit (IMU) sensor node that interfaces with physical bodies
// to compute linear acceleration, angular velocity, and orientation for ROS2 telemetry integration.
class ImuSensor : public Node3D {
    GDCLASS(ImuSensor, Node3D);

private:
    double update_rate;          // Time interval (in seconds) between telemetry updates
    double update_timer;         // Accumulator tracking elapsed time for frequency throttling
    Vector3 last_linear_velocity;// Velocity record of the previous frame used for acceleration calculation
    double noise_stddev;         // Standard deviation of Gaussian noise applied to sensor readouts

protected:
    // Registers class methods and properties with Godot's runtime database for editor inspection
    static void _bind_methods();

public:
    ImuSensor();
    ~ImuSensor();

    // Physics callback executing calculation steps and publishing telemetry data at the set rate
    void _physics_process(double delta) override;

    // Getters and setters exposing configuration parameters to the Godot Inspector interface
    void set_update_rate(double p_rate);
    double get_update_rate() const;
    void set_noise_stddev(double p_noise);
    double get_noise_stddev() const;
};

}

#endif 