# Test Guide (ROS-Godot)

Layered test strategy. See `TESTS.md` for the full plan, thesis-claim mapping,
and the mutation/sabotage correctness checklist.

| Layer | Location | Run | Requires |
|---|---|---|---|
| **L0a** throttle logic | `tests/l0/` | `make -C tests/l0 test` | g++ only (no ROS/Godot) |
| **L0b** coordinate math | `tests/l0_coord/` | inside container: `make test` | godot-cpp + ROS2 |
| **L1** scene/defaults | `godot_project/tests/` via GUT | `Godot --headless -s addons/gut/gut_cmdln.gd` | GUT addon in `godot_project/addons/gut/` |
| **L2** ROS live | `tests/l2/` (not yet scaffolded) | `ros2 launch` in `ros2_humble_env` | ROS2 container |
| **L3** golden scenario | `tests/l3/` (not yet scaffolded) | same as L2 | ROS2 container + fixed world |
| **L4** control latency | `tests/l4/` (not yet scaffolded) | same | ROS2 container |

---

## L0 — Pure-logic tests (fastest; run on bare host)

Currently covers the **time-throttle helper** — the single source of truth that
lists under `tests/l0/include/throttle.hpp`. This is the one place all three
sensors (LiDAR via `ROS2SensorBase`, IMU, Odom) should converge.

```bash
make -C tests/l0 test
```

A non-zero exit means a test failed; the printed message names the failing
assert. A passing run prints `ALL L0 TESTS PASSED`.

### Wiring `throttle.hpp` into the sensors (next step; not required to build tests)

Replace the duplicated timing fields:
- `ROS2SensorBase`: replace `time_accumulator` arithmetic in `_process` with a
  `Throttle` member.
- `ImuSensor::_physics_process` / `OdomSensor::_physics_process`: replace the
  `update_timer += delta; if (update_timer < update_rate) return; `
  `update_timer = 0.0;` block with `Throttle::update(delta)`.

After wiring, re-run `make -C tests/l0 test` and the sabotage checklist
(`TESTS.md §4.1`) before/after to confirm the new wiring is covered.

---

## L0b — Coordinate-mapping tests (compiles the real production code)

Located in `tests/l0_coord/`. Unlike L0a, these compile the **actual**
`src/ros2_message_builder.cpp` against the real godot-cpp + ROS2 headers/libs,
so they exercise the exact functions the sensors call (`godot_to_ros_vector3`,
`godot_to_ros_quaternion`, `godot_to_ros_pose`, `godot_to_ros_transform`) — not
a re-implementation. This is the #1 place "wrong data, no crash" coordinate bugs
live.

Because it needs godot-cpp and ROS message types, it runs **inside the
container**:

```bash
distrobox enter ros2_humble_env
source /opt/ros/humble/setup.bash
make -C /home/scar/Godot/ROS-Godot/tests/l0_coord test
```

A non-zero exit means a test failed; a passing run prints
`ALL L0_COORD TESTS PASSED`.

> **Already validated:** a planted sign-inversion in `godot_to_ros_vector3` made
> the `godot +X -> ros y=-1` and `pose pos.y` assertions turn RED — proving this
> suite catches coordinate-remap errors. Run this check again after any edit to
> `ros2_message_builder.cpp`.

---

## L1 — GUT (GDScript) scene/default tests

Scaffold directory `godot_project/tests/` holds GUT suites. Add the GUT add-on
under `godot_project/addons/gut/` (clone from https://github.com/bitwes/Gut),
then enable it in `godot_project/project.godot` under `[editor_plugins]`:

```ini
enabled=PackedStringArray("res://addons/gut/plugin.cfg")
```

Example suites to write: assert fixed topics (`/lidar_sensor`, `/imu/data`,
`/odom`) and fixed frequencies (10/60/30 Hz) from `get_topic_name()`,
`get_frequency()`, inspector defaults; assert `noise_enabled=false` yields
exact geometry; assert a sensor node without a ROS bridge in scope does not
crash `_ready`.

---

## L2–L4 — ROS live integration (inside `ros2_humble_env`)

Not scaffolded yet (per the agreed "plan + scaffold only" scope). See
`TESTS.md §1 (L2/L3/L4)` and §5 step 4–6 for exactly what each asserts
(subtopic+frame_id, on-wire frequency, monotonic stamps, TF chain, golden
scenario, `/cmd_vel` latency under 50 ms per H1).

---

## Correctness of the tests themselves

A test that survives a **planted bug** is worthless. `TESTS.md §4.1` lists the
sabotage table (invert coord sign → coord test turns RED; change a frequency
constant → cadence/default test turns RED; drop `+9.81` → IMU golden turns RED;
etc.). Run these against every new test before trusting it.
