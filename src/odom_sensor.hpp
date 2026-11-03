#ifndef ODOM_SENSOR_HPP
#define ODOM_SENSOR_HPP

#include <godot_cpp/classes/node3d.hpp>
#include "throttle.hpp"

namespace godot {

// Virtual odometry sensor node that tracks spatial position and velocities
// of a physical parent body, broadcasting standard ROS2 odometry messages and TF transforms.
class OdomSensor : public Node3D {
    GDCLASS(OdomSensor, Node3D);

private:
    // Shared throttling helper. Interval defaults to ~30 Hz (0.033333 s,
    // the Nav2/Gazebo odometry reference rate).
    Throttle throttle{0.033333};

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