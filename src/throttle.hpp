#ifndef ROS_GODOT_THROTTLE_HPP
#define ROS_GODOT_THROTTLE_HPP

// == Pure, dependency-free time-throttle helper =============================
//
// Single source of truth for the "publish at most once per interval" logic
// that today exists in THREE copied forms:
//   - ROS2SensorBase::_process  (time_accumulator, zeroed on publish)
//   - ImuSensor::_physics_process (update_timer)
//   - OdomSensor::_physics_process (update_timer)
//
// This class is header-only, uses only <cstdarg>-free plain double arithmetic,
// so tests under tests/l0/ compile and run WITHOUT ROS or Godot.
//
// FIX: the original code did `time_accumulator = 0.0` on publish. The correct
// behaviour is `acc -= target_interval` so that surplus time beyond the interval
// is carried into the next step; otherwise the effective rate drifts below
// nominal as the frame rate drops (violating the ±10% H2 claim). Tests enforce
// this.

class Throttle {
public:
    // interval: seconds between ticks (e.g. 0.1 for 10 Hz). <=0 disables ticking.
    explicit Throttle(double interval = 0.0) : interval_(interval) {}

    void set_interval(double interval) {
        interval_ = interval;
        // Reset so a changed rate does not immediately fire on stale surplus.
        acc_ = 0.0;
    }
    double interval() const { return interval_; }

    // Feed an elapsed-time step (Godot delta / physics delta). Returns true
    // exactly when a tick (publish) is due; surplus time is carried forward.
    bool update(double delta) {
        if (interval_ <= 0.0) return false;
        acc_ += delta;
        if (acc_ < interval_) return false;
        acc_ -= interval_;       // carry surplus (NOT `= 0.0`)
        return true;
    }

    // Test hook: current carried-in surplus.
    double accumulated() const { return acc_; }

private:
    double interval_;
    double acc_ = 0.0;
};

#endif // ROS_GODOT_THROTTLE_HPP
