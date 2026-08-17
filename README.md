# ROS2-Godot Integration Framework

  

## About the Project

This plugin enables low-latency, real-time communication between the Godot game engine and the industry-standard robotics framework, ROS2.

Instead of relying on slow network detours like WebSockets, this project uses a native C++ real-time interpreter (**GDExtension**). It provides a high-performance middleware bridge that abstracts complex ROS2 structures and makes them accessible for robotics simulation in Godot via plug-and-play.

---
## Core Architecture

The framework strictly separates low-level communication from the game engine's high-level logic. It is built on three pillars:

1. **`RosBridge` (Central Communication Singleton):** The backbone of the plugin. It runs the ROS2 execution context in a dedicated background thread, ensuring Godot's rendering performance remains completely unaffected. The `RosBridge` secures bidirectional data exchange via mutexes (e.g., for `/cmd_vel` control commands) and handles dynamic TF broadcasting for the transformation tree.

2. **`ROS2SensorBase` (C++ Sensor Base Class):** A blueprint for all physical sensors in the simulation. It automates lifecycle management and implements accumulator-based time throttling. As a result, all derived sensors transmit at precisely defined, stable frequencies (e.g., 25 Hz), independent of Godot's framerate.

3. **`ROS2Publisher` (Message Builder & Abstraction Layer):** A universal interface for Godot developers. Topic names and QoS profiles can be configured directly via the graphical Godot Inspector. The node dynamically converts GDScript data into the target ROS2 format and sends it to the DDS network, requiring zero C++ knowledge.
---
## Integrated Robotics Sensors 

The add-on includes ready-to-use sensor nodes that replicate the most important industrial standards for mobile navigation (e.g., Nav2 / SLAM):

* **`LidarSensor`:** Simulates a rotating laser scanner using `RayCast3D` rays. Calculates distances and publishes them as a **`sensor_msgs/msg/LaserScan`** message on the `/scan` topic.
* **`ImuSensor`:** An Inertial Measurement Unit (IMU). Measures the robot's physical acceleration and rotation rates (gyroscope) in the 3D world and publishes them as **`sensor_msgs/msg/Imu`**. Critical for odometry fusion and drift compensation.
* **`OdomSensor`:** The odometry node reads the robot's kinematic position, rotation, and velocity, sending them as **`nav_msgs/msg/Odometry`**. Essential for localization.
---
## System Requirements
The distribution of this add-on is extremely straightforward, utilizing the compiled `.so` library.

| Component | Version / Specification |
| --- | --- |
| **Operating System** | Ubuntu 22.04 LTS (Jammy Jellyfish) |
| **ROS2** | Humble Hawksbill |
| **Game Engine** | Godot 4.x (tested with Godot 4.5 for Linux) |
| **Shell** | Bash or Zsh |

---
## ⚠️ Important Startup Note (Critical)

Godot must **never be started by double-clicking** a desktop icon, otherwise the ROS2 environment variables will be missing and the engine will crash.
1. Open a new terminal.
2. Source the ROS2 environment:
* For Bash: `source /opt/ros/humble/setup.bash`
* For Zsh: `source /opt/ros/humble/setup.zsh`
1. **Mandatory:** Start the Godot engine executable (e.g., `./Godot_v4.5-stable_linux.x86_64`) directly from this exact terminal.
---
## Installation & Usage (Plug-and-Play)

1. Extract the plugin's ZIP file.
2. Drag and drop the `addons` folder directly into the **FileSystem Dock** of your Godot editor.
3. In Godot, go to **Project** -> **Project Settings** -> **Plugins** tab. Check the **Enable** box.
4. Press **Ctrl + A** in the editor and add one of the ready-made sensors (e.g., `LidarSensor` or `ImuSensor`) to your robot in the 3D scene.
5. Start the Godot simulation with **F5**.
6. Open a new terminal, source the ROS2 environment, and verify the live data, e.g., using: `ros2 topic echo /scan` or `ros2 topic echo /odom`.

---
## For Developers: Extending with Custom Sensors

Thanks to the `ROS2SensorBase` abstraction layer, adding new sensors is extremely straightforward:

1. **Inherit:** Create a new C++ class that inherits from `ROS2SensorBase` and override the `_physics_process(double delta)` loop for data acquisition.
2. **Bind:** Register your variables for the Godot GUI in the `_bind_methods()` method.
3. **Register:** Add `ClassDB::register_class<YourClass>();` into the initialization file (`register_types.cpp`).
4. **Compile:** Run the build command (e.g., `scons platform=linux target=template_debug`). The node will be immediately available in the editor's search menu after the next Godot restart.