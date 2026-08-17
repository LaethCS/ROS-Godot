#!/usr/bin/env python
import os
import sys

env = Environment()

# Embed RPATH to automatically locate ROS2 Humble shared libraries at runtime
env.Append(LINKFLAGS=['-Wl,-rpath,/opt/ros/humble/lib'])

# Set compiler flags for C++17 standard and position-independent code
env.Append(CXXFLAGS=['-std=c++17', '-fPIC'])

# 1. Godot include paths and library directory configuration
env.Append(CPPPATH=[
    'godot-cpp/gdextension',
    'godot-cpp/include', 
    'godot-cpp/gen/include', 
    'src'
])
env.Append(LIBPATH=['godot-cpp/bin'])

# 2. Dynamically include all subdirectories within the ROS2 Humble include path
ros2_include_dir = '/opt/ros/humble/include'
if os.path.exists(ros2_include_dir):
    ros2_paths = [os.path.join(ros2_include_dir, d) for d in os.listdir(ros2_include_dir) if os.path.isdir(os.path.join(ros2_include_dir, d))]
    env.Append(CPPPATH=ros2_paths)

# 3. ROS2 library search paths and link dependencies
env.Append(LIBPATH=['/opt/ros/humble/lib'])
env.Append(LIBS=[
    'godot-cpp.linux.template_debug.x86_64',
    'rclcpp',
    'rcutils',
    'rcl',
    # Type support libraries for standard ROS2 message interfaces
    'std_msgs__rosidl_typesupport_cpp',
    'std_msgs__rosidl_typesupport_c',
    'sensor_msgs__rosidl_typesupport_cpp',
    'sensor_msgs__rosidl_typesupport_c',
    'geometry_msgs__rosidl_typesupport_cpp',
    'geometry_msgs__rosidl_typesupport_c',
    'nav_msgs__rosidl_typesupport_cpp',     # Odometry message support
    'nav_msgs__rosidl_typesupport_c',
    # TF2 libraries for coordinate frame broadcasting and transformations
    'tf2_ros',
    'tf2',
    # Base ROSIDL interface support libraries
    'rosidl_typesupport_cpp',
    'rosidl_typesupport_c'
])

if "suffix" not in env:
    env["suffix"] = ".linux.template_debug.x86_64"
    
# Compile all source files into the shared library target for the GDExtension plugin
source_files = Glob('src/*.cpp')
library = env.SharedLibrary("godot_project/addons/ros2_godot_bridge/bin/libtest" + env["suffix"] + ".so", source_files)