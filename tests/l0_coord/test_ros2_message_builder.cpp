// == Tests for ROS2MessageBuilder coordinate mappings (tests/l0_coord ========
// Compiles against the REAL production code (src/ros2_message_builder.cpp) and
// the REAL godot-cpp + ROS2 libraries, so the assertions exercise the exact
// functions the sensors call — NOT a re-implementation.
//
// Build/run inside the ros2_humble_env container:
//   source /opt/ros/humble/setup.bash
//   make test
// Exit code != 0 means a test failed.

#include "ros2_message_builder.hpp"

#include <geometry_msgs/msg/vector3.hpp>
#include <geometry_msgs/msg/quaternion.hpp>
#include <geometry_msgs/msg/transform.hpp>
#include <geometry_msgs/msg/pose.hpp>

#include <cstdio>
#include <cmath>

using namespace godot;

static int g_failures = 0;

#define CHECK_MSG(cond, msg)                                                   \
    do {                                                                       \
        if (!(cond)) {                                                         \
            std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, msg);          \
            ++g_failures;                                                      \
        }                                                                      \
    } while (0)

#define CHECK_NEAR(a, b, eps, msg)                                             \
    CHECK_MSG(std::fabs((a) - (b)) <= (eps), (msg))

// ---------------------------------------------------------------------------
// 1. godot_to_ros_vector3: known axis remap.
//    Godot X -> -ROS Z ; Godot Y -> +ROS Z?? No: y=+y. Recheck against impl:
//      ros.x = -godot.z ; ros.y = -godot.x ; ros.z = godot.y
//    So with godot (x,y,z) = (a,b,c):
//      ros = (-c, -a, b)
// ---------------------------------------------------------------------------
static void test_vector3() {
    // Identity-ish case: godot(0,0,0) -> ros(0,0,0)
    auto v = ROS2MessageBuilder::godot_to_ros_vector3(Vector3(0, 0, 0));
    CHECK_NEAR(v.x, 0.0, 1e-9, "origin maps to origin");
    CHECK_NEAR(v.y, 0.0, 1e-9, "origin maps to origin");
    CHECK_NEAR(v.z, 0.0, 1e-9, "origin maps to origin");

    // godot(1,0,0) -> ros.x=-0=-0, ros.y=-1=-1? Wait recompute:
    //   a=1,b=0,c=0 -> ros.x=-0=0, ros.y=-1=-1, ros.z=0. So (1,0,0)->(0,-1,0).
    {
        auto r = ROS2MessageBuilder::godot_to_ros_vector3(Vector3(1, 0, 0));
        CHECK_NEAR(r.x, 0.0,  1e-9, "godot +X -> ros x=0");
        CHECK_NEAR(r.y, -1.0, 1e-9, "godot +X -> ros y=-1");
        CHECK_NEAR(r.z, 0.0,  1e-9, "godot +X -> ros z=0");
    }
    // godot(0,1,0) -> ros.x=0, ros.y=0, ros.z=1  -> (0,0,1)
    {
        auto r = ROS2MessageBuilder::godot_to_ros_vector3(Vector3(0, 1, 0));
        CHECK_NEAR(r.x, 0.0, 1e-9, "godot +Y -> ros x=0");
        CHECK_NEAR(r.y, 0.0, 1e-9, "godot +Y -> ros y=0");
        CHECK_NEAR(r.z, 1.0, 1e-9, "godot +Y -> ros z=+1");
    }
    // godot(0,0,1) -> ros.x=-1, ros.y=0, ros.z=0 -> (-1,0,0)
    {
        auto r = ROS2MessageBuilder::godot_to_ros_vector3(Vector3(0, 0, 1));
        CHECK_NEAR(r.x, -1.0, 1e-9, "godot +Z -> ros x=-1");
        CHECK_NEAR(r.y, 0.0,  1e-9, "godot +Z -> ros y=0");
        CHECK_NEAR(r.z, 0.0,  1e-9, "godot +Z -> ros z=0");
    }
}

// ---------------------------------------------------------------------------
// 2. godot_to_ros_quaternion: same axis remap on (x,y,z), keep w.
//      ros = (-q.z, -q.x, q.y, q.w)
// ---------------------------------------------------------------------------
static void test_quaternion() {
    // Identity quat (0,0,0,1) stays identity.
    auto q = ROS2MessageBuilder::godot_to_ros_quaternion(Quaternion(0, 0, 0, 1));
    CHECK_NEAR(q.x, 0.0, 1e-9, "identity quat keeps x=0");
    CHECK_NEAR(q.y, 0.0, 1e-9, "identity quat keeps y=0");
    CHECK_NEAR(q.z, 0.0, 1e-9, "identity quat keeps z=0");
    CHECK_NEAR(q.w, 1.0, 1e-9, "identity quat keeps w=1");

    // Known remap: godot quat (x=1,y=0,z=0,w=0) -> ros = (-0, -1, 0, 0) = (0,-1,0,0)
    auto q2 = ROS2MessageBuilder::godot_to_ros_quaternion(Quaternion(1, 0, 0, 0));
    CHECK_NEAR(q2.x, 0.0,  1e-9, "q x=1 -> ros x=0");
    CHECK_NEAR(q2.y, -1.0, 1e-9, "q x=1 -> ros y=-1");
    CHECK_NEAR(q2.z, 0.0,  1e-9, "q x=1 -> ros z=0");
    CHECK_NEAR(q2.w, 0.0,  1e-9, "q x=1 -> ros w=0");
}

// ---------------------------------------------------------------------------
// 3. godot_to_ros_pose: position + orientation follow the same mappings.
// ---------------------------------------------------------------------------
static void test_pose() {
    Transform3D t;  // default == identity
    t.origin = Vector3(1, 2, 3);

    auto p = ROS2MessageBuilder::godot_to_ros_pose(t);
    // origin (1,2,3) -> ros.x=-3, ros.y=-1, ros.z=2
    CHECK_NEAR(p.position.x, -3.0, 1e-9, "pose pos.x");
    CHECK_NEAR(p.position.y, -1.0, 1e-9, "pose pos.y");
    CHECK_NEAR(p.position.z,  2.0, 1e-9, "pose pos.z");
    // Identity rotation -> ROS identity.
    CHECK_NEAR(p.orientation.x, 0.0, 1e-9, "pose orient x");
    CHECK_NEAR(p.orientation.y, 0.0, 1e-9, "pose orient y");
    CHECK_NEAR(p.orientation.z, 0.0, 1e-9, "pose orient z");
    CHECK_NEAR(p.orientation.w, 1.0, 1e-9, "pose orient w");
}

// ---------------------------------------------------------------------------
// 4. godot_to_ros_transform: translation + rotation combined.
// ---------------------------------------------------------------------------
static void test_transform() {
    Transform3D t;  // default == identity
    t.origin = Vector3(0, 0, 5);

    auto xf = ROS2MessageBuilder::godot_to_ros_transform(t);
    CHECK_NEAR(xf.translation.x, -5.0, 1e-9, "transform tx.x (godot +Z becomes -x)");
    CHECK_NEAR(xf.translation.y, 0.0,  1e-9, "transform tx.y");
    CHECK_NEAR(xf.translation.z, 0.0,  1e-9, "transform tx.z");
    CHECK_NEAR(xf.rotation.w, 1.0, 1e-9, "transform rot w identity");
}

// ---------------------------------------------------------------------------
// 5. Round-trip sanity: forward-then-normalize keeps unit quaternion magnitude.
// ---------------------------------------------------------------------------
static void test_quat_unit_magnitude() {
    Vector3 axis(0.3, 1.0, 0.2);
    axis.normalize();
    Quaternion g(axis, 1.234);
    auto ros = ROS2MessageBuilder::godot_to_ros_quaternion(g);
    double mag = std::sqrt(ros.x*ros.x + ros.y*ros.y + ros.z*ros.z + ros.w*ros.w);
    CHECK_NEAR(mag, 1.0, 1e-6, "axis remap preserves unit quaternion (axis remap is an orthonormal swap)");
}

int main() {
    test_vector3();
    test_quaternion();
    test_pose();
    test_transform();
    test_quat_unit_magnitude();

    if (g_failures == 0) {
        std::printf("\nALL L0_COORD TESTS PASSED\n");
        return 0;
    }
    std::printf("\n%d L0_COORD TEST(S) FAILED\n", g_failures);
    return 1;
}
