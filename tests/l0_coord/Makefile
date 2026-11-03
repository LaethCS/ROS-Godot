# L0 coordinate-mapping tests.
# Compiles the REAL src/ros2_message_builder.cpp + test against godot-cpp &
# ROS2. Run INSIDE the ros2_humble_env container AFTER `source /opt/ros/humble/setup.bash`.
#
#   distrobox enter ros2_humble_env
#   source /opt/ros/humble/setup.bash
#   make test
#
# Non-zero exit = test failure.

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -fPIC

# Project root (this Makefile lives at tests/l0_coord/)
ROOT     := ../..
# --- godot-cpp include paths (mirrors SConstruct) ---
GODOT_INC := $(ROOT)/godot-cpp/gdextension $(ROOT)/godot-cpp/include \
             $(ROOT)/godot-cpp/gen/include $(ROOT)/src
# --- ROS include dirs (all subdirs of /opt/ros/humble/include) ---
ROS_INC   := $(wildcard /opt/ros/humble/include/*)

CPPFLAGS  := $(addprefix -I,$(GODOT_INC) $(ROS_INC))
# godot-cpp static lib
LDFLAGS   := -L$(ROOT)/godot-cpp/bin -L/opt/ros/humble/lib -Wl,-rpath,/opt/ros/humble/lib
LIBS      := -lgodot-cpp.linux.template_debug.x86_64 \
             -lrclcpp -lrcutils -lrcl \
             -lstd_msgs__rosidl_typesupport_cpp -lstd_msgs__rosidl_typesupport_c \
             -lsensor_msgs__rosidl_typesupport_cpp -lsensor_msgs__rosidl_typesupport_c \
             -lgeometry_msgs__rosidl_typesupport_cpp -lgeometry_msgs__rosidl_typesupport_c \
             -lnav_msgs__rosidl_typesupport_cpp -lnav_msgs__rosidl_typesupport_c \
             -ltf2_ros -ltf2 \
             -lrosidl_typesupport_cpp -lrosidl_typesupport_c

TEST_BIN  := test_ros2_message_builder

.PHONY: all test clean

all: $(TEST_BIN)

$(TEST_BIN): test_ros2_message_builder.cpp $(ROOT)/src/ros2_message_builder.cpp $(ROOT)/src/ros2_message_builder.hpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) test_ros2_message_builder.cpp $(ROOT)/src/ros2_message_builder.cpp -o $@ $(LDFLAGS) $(LIBS)

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -f $(TEST_BIN)
