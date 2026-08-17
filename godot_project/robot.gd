extends RigidBody3D

# Movement configuration parameters
const SPEED = 5.0
const ROTATION_SPEED = 2.0

# Reference to the global communication bridge singleton
var ros_bridge = null

func _ready() -> void:
	# Retrieve the registered RosBridge singleton at runtime
	if Engine.has_singleton("RosBridge"):
		ros_bridge = Engine.get_singleton("RosBridge")
	else:
		push_error("RosBridge singleton not found! Ensure the GDExtension add-on is enabled.")

# NOTE: For RigidBody3D nodes, use _integrate_forces instead of _physics_process 
# to safely read and modify physics states without conflicts.
func _integrate_forces(state: PhysicsDirectBodyState3D) -> void:
	if not ros_bridge:
		return

	# Fetch velocity commands from the ROS2 communication bridge
	var linear = ros_bridge.get_linear_velocity()
	var angular = ros_bridge.get_angular_velocity()
	
	if linear != 0.0 or angular != 0.0:
		print("Command received - Linear: ", linear, " Angular: ", angular)

	# 1. ROTATION
	# Isolate the current angular velocity vector and update the Y-axis component
	var new_angular = state.angular_velocity
	new_angular.y = angular * ROTATION_SPEED
	state.angular_velocity = new_angular

	# 2. LINEAR MOTION
	# Compute forward movement vector (-Z represents forward in Godot's standard coordinate system)
	var direction = -transform.basis.z
	var target_velocity = direction * linear * SPEED
	
	# Preserve current vertical velocity (incorporating gravity and environmental forces)
	var new_linear = state.linear_velocity
	
	# Update horizontal plane components (X and Z) while keeping Y unaffected by control inputs
	new_linear.x = target_velocity.x
	new_linear.z = target_velocity.z
	
	# Apply the updated velocity vector back to the physics state
	state.linear_velocity = new_linear
