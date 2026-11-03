# ROS2-Godot Integration Framework

  

## About the Project

> **Quick orientation for this repository.** Everything ROS2-related here runs inside a `distrobox` container named `ros2_humble_env`. Do **all** project changes (build, run, edit, `ros2 ...` tools) from inside that container, and **always source the ROS2 environment first**. Details in [Development Environment](#---development-environment---distrobox-container) below.

This plugin enables low-latency, real-time communication between the Godot game engine and the industry-standard robotics framework, ROS2.

Instead of relying on slow network detours like WebSockets, this project uses a native C++ real-time interpreter (**GDExtension**). It provides a high-performance middleware bridge that abstracts complex ROS2 structures and makes them accessible for robotics simulation in Godot via plug-and-play.

---
## Core Architecture

The framework strictly separates low-level communication from the game engine's high-level logic. It is built on three pillars:

1. **`RosBridge` (Central Communication Singleton):** The backbone of the plugin. It runs the ROS2 execution context in a dedicated background thread, ensuring Godot's rendering performance remains completely unaffected. The `RosBridge` secures bidirectional data exchange via mutexes (e.g., for `/cmd_vel` control commands) and handles dynamic TF broadcasting for the transformation tree.

2. **`ROS2SensorBase` (C++ Sensor Base Class):** A blueprint for all physical sensors in the simulation. It automates lifecycle management and implements accumulator-based time throttling. As a result, all derived sensors transmit at precisely defined, stable frequencies (LiDAR 10 Hz, IMU 60 Hz, Odometry ~30 Hz), independent of Godot's framerate.

3. **`ROS2Publisher` (Message Builder & Abstraction Layer):** A universal interface for Godot developers. Topic names and QoS profiles can be configured directly via the graphical Godot Inspector. The node dynamically converts GDScript data into the target ROS2 format and sends it to the DDS network, requiring zero C++ knowledge.
---
## Integrated Robotics Sensors 

The add-on includes ready-to-use sensor nodes that replicate the most important industrial standards for mobile navigation (e.g., Nav2 / SLAM):

* **`LidarSensor`:** Simulates a rotating laser scanner using `RayCast3D` rays. Calculates distances and publishes them as a **`sensor_msgs/msg/LaserScan`** message on the `/lidar_sensor` topic.
* **`ImuSensor`:** An Inertial Measurement Unit (IMU). Measures the robot's physical acceleration and rotation rates (gyroscope) in the 3D world and publishes them as **`sensor_msgs/msg/Imu`**. Critical for odometry fusion and drift compensation.
* **`OdomSensor`:** The odometry node reads the robot's kinematic position, rotation, and velocity, sending them as **`nav_msgs/msg/Odometry`**. Essential for localization.

### Sensor configuration (fixed, like a physical device)

To model concrete industrial hardware, each sensor's **ROS topic and publishing frequency are hardcoded** (not changeable in the Inspector — matching the behaviour of real devices where these are fixed by the hardware). The values follow established robotics-simulation practice (Gazebo / Nav2 references):

| Sensor | ROS Topic | Message | Frequency | Frame |
| --- | --- | --- | --- | --- |
| **LidarSensor** | `/lidar_sensor` | `sensor_msgs/msg/LaserScan` | **10 Hz** | `base_link` → `lidar_link` |
| **ImuSensor** | `/imu/data` | `sensor_msgs/msg/Imu` | **60 Hz** | `base_link` → `imu_link` |
| **OdomSensor** | `/odom` | `nav_msgs/msg/Odometry` | **~30 Hz** | `odom` → `base_link` |

Only the **measurement noise** stays adjustable via the Inspector. Each sensor exposes a `noise_enabled` (bool) switch — enable it for realistic, industrial-grade readings, or turn it off to get clean "ground-truth" data for debugging. The noise magnitude itself is tuned with `noise_stddev`.

Setting the frequencies:
- **LiDAR 10 Hz** — de-facto Nav2/Gazebo standard for 2D laser scanners (real units: SICK / RPLIDAR ≈ 5–15 Hz).
- **IMU 60 Hz** — matches Godot's default physics tick rate so IMU data is synced with the simulated physics step.
- **Odometry ~30 Hz** — Gazebo/Nav2 default, typically tied to the physics update rate.
---
## ⚙️ Development Environment (Distrobox Container)

This project's ROS2 side is not expected to run on the bare host. Everything hardcodes `/opt/ros/humble`, so the ROS2 stack is installed inside a dedicated container.

| Component | Version / Specification |
| --- | --- |
| **Host OS** | Ubuntu 22.04 LTS (Jammy Jellyfish) — base image of the container |
| **Container** | `ros2_humble_env` (created with [distrobox](https://github.com/89luca89/distrobox)) |
| **ROS2 (in container)** | Humble Hawksbill — installed from the official `deb` repo. Sources at `/opt/ros/humble` |
| **Game Engine** | Godot 4.x (tested with **Godot 4.5** for Linux). Binary lives on the host at `/home/scar/Godot/Godot_v4.5-stable_linux.x86_64` |
| **Shell** | Bash or Zsh |

### Container creation (once)

The container was provisioned with roughly:

```bash
distrobox create --name ros2_humble_env --image docker.io/library/ubuntu:22.04
distrobox enter ros2_humble_env      # enter to run the commands below
```

Inside the container:

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y software-properties-common
sudo add-apt-repository universe
sudo apt update && sudo apt install -y locales
sudo locale-gen en_US en_US.UTF-8
sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
export LANG=en_US.UTF-8

sudo apt install -y curl gnupg lsb-release
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
  -o /usr/share/keyrings/ros-archive-keyring.gpg
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" \
  | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null

sudo apt update
sudo apt install -y ros-humble-desktop
```

### Using the container (every time)

1. Enter the container from the host:
   ```bash
   distrobox enter ros2_humble_env
   ```
2. **Always source** the ROS2 environment before using anything ROS2-related:
   * Bash: `source /opt/ros/humble/setup.bash`
   * Zsh: `source /opt/ros/humble/setup.zsh`
3. Now run the project commands (build, run Godot, `ros2 topic echo ...`) in this exact shell.

<details>
<summary><b>Build once source (recommended alias)</b></summary>

To avoid having to `source` by hand every time, add to `~/.bashrc` / `~/.zshrc` **inside the container**:

```bash
source /opt/ros/humble/setup.bash
```

…or via distrobox so it applies on every `distrobox enter`:

```bash
echo 'source /opt/ros/humble/setup.bash' >> ~/.bashrc   # if not already present
```

</details>

---
## ⚠️ Important Startup Note (Critical)

Godot must **never be started by double-clicking** a desktop icon, otherwise the ROS2 environment variables will be missing and the engine will crash. It also must run from a shell where the container's ROS2 env is available.
1. Open a terminal and enter the container: `distrobox enter ros2_humble_env`
2. Source the ROS2 environment (unless already baked into `~/.bashrc`):
* For Bash: `source /opt/ros/humble/setup.bash`
* For Zsh: `source /opt/ros/humble/setup.zsh`
3. **Mandatory:** Start the Godot engine executable directly from this exact terminal, e.g.:
   ```bash
   /home/scar/Godot/Godot_v4.5-stable_linux.x86_64 --path /home/scar/Godot/ROS-Godot/godot_project
   ```
   (`distrobox` shares the host filesystem, so the path is the same inside and outside the container.)
---
## Installation & Usage (Plug-and-Play)

1. Extract the plugin's ZIP file.
2. Drag and drop the `addons` folder directly into the **FileSystem Dock** of your Godot editor.
3. In Godot, go to **Project** -> **Project Settings** -> **Plugins** tab. Check the **Enable** box.
4. Press **Ctrl + A** in the editor and add one of the ready-made sensors (e.g., `LidarSensor` or `ImuSensor`) to your robot in the 3D scene.
5. Start the Godot simulation with **F5**.
6. Open a new terminal, source the ROS2 environment, and verify the live data, e.g., using: `ros2 topic echo /lidar_sensor` or `ros2 topic echo /odom`.

---
## For Developers: Extending with Custom Sensors

Thanks to the `ROS2SensorBase` abstraction layer, adding new sensors is extremely straightforward:

1. **Inherit:** Create a new C++ class that inherits from `ROS2SensorBase` and override the `_physics_process(double delta)` loop for data acquisition.
2. **Bind:** Register your variables for the Godot GUI in the `_bind_methods()` method.
3. **Register:** Add `ClassDB::register_class<YourClass>();` into the initialization file (`register_types.cpp`).
4. **Compile:** Run the build command from **inside the container** (after sourcing), e.g.:
   ```bash
   distrobox enter ros2_humble_env
   source /opt/ros/humble/setup.bash
   scons platform=linux target=template_debug
   ```
   The node will be immediately available in the editor's search menu after the next Godot restart.

---

## Testing

> **Why tests are required here.** This project's thesis claims measurable
> behavior — sensor publishing frequencies (LiDAR 10 Hz, IMU 60 Hz, odom ~30 Hz)
> within ±10 %, low jitter, correct frame/TF conventions, and correct data for
> Nav2. Editing a sensor or adding a feature can silently regress these without
> any crash. The layered suite below exists to catch exactly that. Before
> trusting any test, do the mutation check (see `tests/TESTS.md §4`): plant a
> bug and confirm the test turns red.

The test suite is layered so the fast, deterministic checks run on the bare
host without ROS or Godot, while the ROS-live checks run inside the container.

| Layer | Scope | How to run | Requires |
|---|---|---|---|
| **L0a** | Pure timing logic | `make -C tests/l0 test` | `g++` only, no ROS/Godot |
| **L0b** | Coordinate math (real `ros2_message_builder`) | inside container: `make -C tests/l0_coord test` | godot-cpp + ROS2 |
| **L1** | Scene/node defaults via GUT | `Godot --headless -s addons/gut/gut_cmdln.gd` | GUT addon in `godot_project/addons/gut/` |
| **L2–L4** | ROS-live integration (topic, frequency on the wire, TF chain, golden scenario, `/cmd_vel` latency) | via `ros2` inside container | `ros2_humble_env` |

### L0 — Logic tests

Two suites, both deterministic:

```bash
# L0a — Throttle timing (bare host, no ROS/Godot)
make -C tests/l0 test

# L0b — Coordinate math, compiling the real ros2_message_builder (in container)
distrobox enter ros2_humble_env
source /opt/ros/humble/setup.bash
make -C tests/l0_coord test
```

**L0a** tests the **`Throttle`** helper in `src/throttle.hpp` — the single
time-throttle source of truth that all sensors now share (LiDAR via
`ROS2SensorBase`, plus `ImuSensor` and `OdomSensor`). **L0b** compiles and tests
the real `src/ros2_message_builder.cpp` coordinate mappings against godot-cpp +
ROS. A non-zero exit means a test failed; a passing run prints the matching
`ALL L0... TESTS PASSED`.

### L1 — Scene/default tests (GUT)

Scaffold lives in `godot_project/tests/`, driven by the GUT for Godot add-on
(https://github.com/bitwes/Gut). Add GUT under `godot_project/addons/gut/` and
enable it in `godot_project/project.godot`:

```ini
[editor_plugins]
enabled=PackedStringArray("res://addons/gut/plugin.cfg")
```

### L2–L4 — ROS-live integration (inside the container)

Run these after sourcing ROS2. They assert the on-the-wire contract: correct
topic + `frame_id`, `~frequency` with limited jitter, monotonic timestamps,
the `odom → base_link → lidar_link/imu_link` TF chain, and — in the golden
scenario — that a robot at a known pose reports correct LiDAR ranges, IMU
acceleration, and odometry. When adding a **new feature**, add a matching
test here so the feature is proven end-to-end, not just that it "builds".

### Reference

Full strategy, thesis-claim mapping, tooling setup, and the mutation/sabotage
correctness checklist are in `tests/TESTS.md`.