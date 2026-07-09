#!/usr/bin/env python
import os
import sys

env = Environment()

# Compiler Flags für C++17
env.Append(CXXFLAGS=['-std=c++17', '-fPIC'])

# 1. Godot Pfade
env.Append(CPPPATH=[
    'godot-cpp/gdextension',
    'godot-cpp/include', 
    'godot-cpp/gen/include', 
    'src'
])
env.Append(LIBPATH=['godot-cpp/bin'])

# 2. ROS2 Humble Include-Pfade
# Wir weisen Python an, einfach JEDEN Ordner in /opt/ros/humble/include zu laden.
ros2_include_dir = '/opt/ros/humble/include'
if os.path.exists(ros2_include_dir):
    ros2_paths = [os.path.join(ros2_include_dir, d) for d in os.listdir(ros2_include_dir) if os.path.isdir(os.path.join(ros2_include_dir, d))]
    env.Append(CPPPATH=ros2_paths)


# 3. ROS2 Bibliotheks-Pfade & Linker
env.Append(LIBPATH=['/opt/ros/humble/lib'])
env.Append(LIBS=[
    'godot-cpp.linux.template_debug.x86_64',
    'rclcpp',
    'rcutils',
    'rcl',
    # Standard-Nachrichten
    'std_msgs__rosidl_typesupport_cpp',
    'std_msgs__rosidl_typesupport_c',
    # Sensor-Nachrichten (LaserScan)
    'sensor_msgs__rosidl_typesupport_cpp',
    'sensor_msgs__rosidl_typesupport_c',
    
    # Für den TF-Broadcaster
    'tf2_ros',
    'tf2',
    'geometry_msgs__rosidl_typesupport_cpp',
    'geometry_msgs__rosidl_typesupport_c'
])

if "suffix" not in env:
    env["suffix"] = ".linux.template_debug.x86_64"
    
# Build Targets
source_files = Glob('src/*.cpp') # Variable heißt jetzt source_files
library = env.SharedLibrary("godot_project/addons/ros2_abstandsensor/bin/libtest" + env["suffix"] + ".so", source_files)