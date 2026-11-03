# Test Plan & Strategy (ROS-Godot)

This project is a **bachelor thesis** that makes *measurable* claims about the
simulation (latency, real-time factor, sensor frequency stability, data
consistency for Nav2). Every one of those claims is a **testable assertion**.
This document defines *what* to test, *how*, and — critically — **how to prove
the tests are actually testing something**.

> **Why tests are mandatory here, not optional:**
> You will edit sensors / add features. The code today already contains two
> **divergent timing implementations** doing the "same" job:
>   - `ROS2SensorBase::_process` accumulates `time_accumulator` and **zeroes
>     it** on publish (`= 0.0`, not `-= interval`).
>   - `ImuSensor` / `OdomSensor` each roll their own `update_timer < update_rate`
>     inside `_physics_process`.
> A test suite tied to your **H2 (±10% frequency) / TF3 (jitter)** claims is what
> catches this divergence the moment an edit regresses it.

---

## 1. Test surface (map to thesis claims)

| Layer | What to test | Fails_guards_claim |
|---|---|---|
| **L0 – Pure logic** (no ROS/Godot) | time-throttle cadence, coordinate remap math, clamp/bounds, LiDAR `ray_count`/angle math, IMU `a=dv/dt`+gravity, odom local-frame rotation | H2, TF3, H1, TF4 "data-correctness" |
| **L1 – Scene/node behavior** (GUT, headless Godot) | Inspector defaults, fixed topics/frequencies, noise on/off effect, node without RosBridge doesn't crash | H2 defaults, robustness of edits |
| **L2 – System/ROS live** (rclcpp test node in `ros2_humble_env`) | subtopic+frame_id received, ~frequency on the wire, `stamp` monotonic, TF chain `odom→base_link→lidar_link/imu_link` present | TF3, TF4, TF2 |
| **L3 – Golden scenario** (L2 + fixed world) | robot at known pose facing known wall → LiDAR range, IMU accel, odom pose match truth within tolerance | TF5, "we get the data we want" |
| **L4 – E2E control latency** | `/cmd_vel` → robot moves, latency < 50 ms (H1) | H1, TF1 |

### Concrete bug-classes the tests must own
1. **Timing divergence** — LiDAR vs IMU vs Odom publish cadence disagree after a
   frequency edit. (L0 keeps the single helper honest.)
2. **Coordinate remap sign errors** — `godot_to_ros_vector3` axis reorder is the
   #1 way "we get wrong data" while nothing throws. (L0.)
3. **Accumulator zero-vs-subtract aliasing** — low framerate drifts the effective
   rate. (L0 boundary tests at `delta ≈ 1/freq` and below.)
4. **Clamp / no-hit** — LiDAR `range_min`/`range_max`/`inf` behavior. (L0/L1.)
5. **frame_id / QoS / TF conventions** drift after edits. (L2/L3.)

---

## 2. Tooling

- **L0:** Plain C++ assertion harness (headers + one `.cpp`). No ROS, no Godot,
  no engine boot → fastest feedback. Located under `tests/l0/`. The throttle
  logic is pure `double`s and complies standalone.
- **L1:** **GUT** (Godot Unit Test) add-on under
  `godot_project/addons/gut/` + tests under `godot_project/tests/`. Run headless:
  `Godot --headless -s addons/gut/gut_cmdln.gd`.
- **L2–L4:** `rclcpp` subscriber test node + `ros2 launch` / `launch_testing`,
  run inside the `ros2_humble_env` distrobox (matches documented dev env).

---

## 3. Scaffold provided in this commit

```
tests/
  TESTS.md              <- this file
  README.md             <- how to build + run each layer
  l0/
    include/throttle.hpp      <- extracted pure time-throttle helper
    include/throttle.hpp.in   <- (future) generated/param-ized
    src/throttle.cpp          <- (if logic put in .cpp; header-only here)
    test_throttle.cpp         <- L0 test binary (runs standalone)
    Makefile                  <- `make test` builds+runs, exit!=0 on failure
```

`tests/l0/include/throttle.hpp` is the **single source of truth** the sensors
should later use, so both the LiDAR `time_accumulator` path and the IMU/Odom
`update_timer` path converge on one implementation (fixing the divergence).

---

## 4. Correctness method — how to be sure the tests are right

The question "how do we know the tests work?" has a concrete answer:
a test is only meaningful if it **fails when the code is wrong**.

### 4.1 Mutation / sabotage checklist (do this on every L0 test)
For each test, temporarily introduce a real defect, confirm the test turns
**RED**, then revert and confirm **GREEN**. A test that survives a planted bug
is worthless — delete or fix it.

| Planted defect | Test that must turn RED |
|---|---|
| Invert a sign in `godot_to_ros_vector3` | L0 coord-remap golden asserts |
| Change `frequency` constant (e.g. 10 → 12 Hz) | L0 cadence / L1 default-frequency assert |
| `time_accumulator = 0.0` instead of `-= interval` | L0 low-framerate drift test |
| Remove `+9.81` gravity in IMU | L0 IMU accel golden assert |
| Swap `lidar_link` ↔ `base_link` | L2 frame_id assert |
| Drop the `range_min` clamp | L0 boundary test |
| Publish unconditionally in `_process` | L0 cadence "no publish below 1/freq" test |

### 4.2 Known-answer / golden tests
Hardcode independently-computed expected values (trigonometry, Nav2
conventions), *not* re-runs of the production formulas. Passing a golden test is
meaningful; passing a "code≈code" test is not.

### 4.3 Boundary cases
- `delta` exactly `1/freq` → publishes.
- `delta` slightly below `1/freq` → does not.
- `frequency <= 0` → guard; no division-by-zero / no publish.
- no ray hit → `inf`; hits below `range_min` → `inf`; resultant clamped to
  `[range_min, range_max]`.

### 4.4 Golden scenario (the "data we want" proof — L3)
Robot + known wall at 2.00 m straight ahead → assert `ranges[center] ≈ 2.00
± tol`. Robot stationary → IMU `a_y ≈ 9.81` and `angular_vel ≈ 0` (noise off).
Odometry pose == Godot transform within tol. This maps directly to TF5 and is
your end-to-end correctness proof for the write-up.

### 4.5 CI / repeatability
- L0 tests: deterministic, no RNG, no network → reproducible on any machine.
- For L1/L2 noise tests, seed the RNG deterministically or assert statistical
  bounds that hold for a fixed seed.

---

## 5. Order of work

1. **L0a throttle** (done) — unified LiDAR/IMU/Odom onto `src/throttle.hpp`; test in `tests/l0/`.
2. **L0b coord-remap** (done) — compiles the real `src/ros2_message_builder.cpp`
   against godot-cpp + ROS in the container; test in `tests/l0_coord/`. Already
   sabotage-checked (planting a sign inversion turned the assertions RED).
3. **L1 GUT** defaults (fixed topics/freqs, noise toggles).
4. **L2 rclcpp** frequency + frame_id + stamp on the wire.
5. **L3 golden scenario.**
6. **L4 `/cmd_vel` latency.**
