#ifndef IMU_SENSOR_HPP
#define IMU_SENSOR_HPP

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include "throttle.hpp"

namespace godot {

// Virtual Inertial Measurement Unit (IMU) sensor node that interfaces with physical bodies
// to compute linear acceleration, angular velocity, and orientation for ROS2 telemetry integration.
class ImuSensor : public Node3D {
    GDCLASS(ImuSensor, Node3D);

private:
    double update_rate;          // Time interval (in seconds) between telemetry updates
    // Shared throttling helper; interval = update_rate. Surplus time carries forward
    // so the publish cadence stays accurate at variable physics tick rates.
    Throttle throttle;
    Vector3 last_linear_velocity;// Velocity record of the previous frame used for acceleration calculation
    Quaternion last_orientation; // Orientation record of the previous frame used to derive angular velocity
    bool has_last_orientation = false; // Whether a previous orientation sample has already been recorded
    double noise_stddev;         // Standard deviation of Gaussian noise applied to sensor readouts
    bool noise_enabled = true;   // Master toggle: enables/disables simulated IMU measurement noise

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
    void set_noise_enabled(bool p_val);
    bool get_noise_enabled() const;
};

}

#endif 