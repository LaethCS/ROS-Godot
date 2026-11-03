// == Tests for Throttle (tests/l0) =========================================
// Plain C++ assertion harness: no ROS, no Godot, no engine boot.
// Run with:  make test   (see tests/l0/Makefile)
// Exit code != 0 means a test failed.

#include "../../src/throttle.hpp"

#include <cmath>
#include <cstdio>
#include <vector>

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
// 1. Exactly at interval -> ticks once.
// ---------------------------------------------------------------------------
static void test_exact_interval() {
    Throttle t(0.1);          // 10 Hz
    int ticks = 0;
    for (int i = 0; i < 10; ++i)
        if (t.update(0.1)) ++ticks;
    CHECK_MSG(ticks == 10, "exactly-1/freq steps should tick every time");
    CHECK_MSG(t.accumulated() == 0.0, "no surplus after exact steps");
}

// ---------------------------------------------------------------------------
// 2. Below interval -> must NOT tick yet.
// ---------------------------------------------------------------------------
static void test_below_interval_no_tick() {
    Throttle t(0.1);
    // Single small steps must NOT tick on their own.
    CHECK_MSG(!t.update(0.049),  "delta<interval must not tick");
    CHECK_MSG(!t.update(0.049),  "delta<interval must not tick (acc=0.098 < 0.1)");
    // acc = 0.098; a 0.05 step reaches 0.148 >= 0.1 -> ticks, surplus = 0.148-0.1 = 0.048.
    CHECK_MSG(t.update(0.05),   "accumulated across steps must cross threshold");
    CHECK_NEAR(t.accumulated(), 0.048, 1e-9, "surplus 0.148-0.1=0.048 carried");
    // Now acc=0.048; a 0.02 step reaches 0.068 < 0.1 -> must NOT tick yet.
    CHECK_MSG(!t.update(0.02),   "accumulated below threshold must not tick");
    CHECK_NEAR(t.accumulated(), 0.068, 1e-9, "accumulator now 0.068");
}

// ---------------------------------------------------------------------------
// 3. Golden deviation (H2 claim): ~60Hz frame deltas into a 10Hz sensor and a
//    30Hz sensor must stay within ±10% of nominal over a long window.
//    This is the behavioural contract the whole suite guards.
// ---------------------------------------------------------------------------
static double wallclock_ticks_60fps(double interval, int seconds) {
    Throttle t(interval);
    const double dt = 1.0 / 60.0;
    long ticks = 0;
    for (int s = 0; s < seconds; ++s)
        for (int f = 0; f < 60; ++f)
            if (t.update(dt)) ++ticks;
    return static_cast<double>(ticks) / static_cast<double>(seconds); // Hz
}

static void test_frequency_within_ten_percent() {
    // 10 Hz LiDAR: expect 9.0..11.0 ticks/sec.
    double lidar = wallclock_ticks_60fps(0.1, 30);
    CHECK_MSG(lidar >= 9.0 && lidar <= 11.0, "10Hz sensor within ±10% at 60fps");
    std::printf("info: measured LiDAR rate = %.3f Hz\n", lidar);
    // ~30 Hz odom: expect 27.0..33.0.
    double odom = wallclock_ticks_60fps(0.033333, 30);
    CHECK_MSG(odom >= 27.0 && odom <= 33.0, "30Hz sensor within ±10% at 60fps");
    std::printf("info: measured Odom rate  = %.3f Hz\n", odom);
    // 60 Hz IMU at 60fps: expect 54.0..66.0.
    double imu = wallclock_ticks_60fps(1.0 / 60.0, 30);
    CHECK_MSG(imu >= 54.0 && imu <= 66.0, "60Hz sensor within ±10% at 60fps");
    std::printf("info: measured IMU rate   = %.3f Hz\n", imu);
}

// ---------------------------------------------------------------------------
// 4. Low-frame-rate drift guard (the old `= 0.0` bug).
//    At 20fps (dt=0.05) a 10Hz sensor MUST still tick ≈10Hz, NOT only 6.67Hz
//    (which is what `acc = 0.0` would produce: it would floor to every other
//    frame). Enforce surplus-carrying.
// ---------------------------------------------------------------------------
static void test_low_framerate_no_drift() {
    Throttle t(0.1);
    long ticks = 0;
    const double dt = 0.05; // 20 fps
    for (int s = 0; s < 30; ++s)
        for (int f = 0; f < 20; ++f)
            if (t.update(dt)) ++ticks;
    double rate = static_cast<double>(ticks) / 30.0;
    std::printf("info: 20fps 10Hz measured rate = %.3f Hz\n", rate);
    CHECK_MSG(rate >= 9.0 && rate <= 11.0,
              "10Hz sensor still ~10Hz at 20fps (surplus carry vs zeroing)");
}

// ---------------------------------------------------------------------------
// 5. Interval <= 0 -> disabled, never ticks, no div-by-zero.
// ---------------------------------------------------------------------------
static void test_disabled() {
    Throttle t(0.0);
    CHECK_MSG(!t.update(1.0), "interval<=0 must never tick");
    Throttle t2(-1.0);
    CHECK_MSG(!t2.update(1.0), "negative interval must never tick");
    // set_interval to a positive value re-enables.
    t2.set_interval(0.2);
    CHECK_MSG(t2.update(0.2), "set_interval must re-enable ticking");
}

// ---------------------------------------------------------------------------
// 6. set_interval reset clears stale surplus (no immediate misfire).
// ---------------------------------------------------------------------------
static void test_set_interval_resets() {
    Throttle t(1.0);
    t.update(0.9);            // surplus pending 0.9
    t.set_interval(0.1);
    CHECK_NEAR(t.accumulated(), 0.0, 1e-9, "set_interval must reset accumulator");
}

int main() {
    test_exact_interval();
    test_below_interval_no_tick();
    test_frequency_within_ten_percent();
    test_low_framerate_no_drift();
    test_disabled();
    test_set_interval_resets();

    if (g_failures == 0) {
        std::printf("\nALL L0 TESTS PASSED\n");
        return 0;
    }
    std::printf("\n%d L0 TEST(S) FAILED\n", g_failures);
    return 1;
}
