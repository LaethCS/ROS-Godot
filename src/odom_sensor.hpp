#ifndef ODOM_SENSOR_HPP
#define ODOM_SENSOR_HPP

#include <godot_cpp/classes/node3d.hpp>

namespace godot {

// Virtual odometry sensor node that tracks spatial position and velocities
// of a physical parent body, broadcasting standard ROS2 odometry messages and TF transforms.
class OdomSensor : public Node3D {
    GDCLASS(OdomSensor, Node3D);

private:
    double update_rate = 0.02; // Update interval in seconds (targeting 50 Hz)
    double update_timer = 0.0; // Time accumulator used for frequency throttling

protected:
    // Registers runtime methods with Godot's class database
    static void _bind_methods();
 
public:
    OdomSensor();
    ~OdomSensor();

    // Physics callback executing odometry state extraction and transmission at each fixed time step
    void _physics_process(double delta) override;
};

}

#endif