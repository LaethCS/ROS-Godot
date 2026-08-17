extends Node3D

func _ready() -> void:
	# Verify if the custom GDExtension class is registered in the engine class database
	if ClassDB.class_exists("RosBridge"):
		print("Success: RosBridge class exists!")
		
		# Verify if the global engine singleton instance is initialized and accessible
		if Engine.has_singleton("RosBridge"):
			print("Success: RosBridge singleton found!")
			var bridge = Engine.get_singleton("RosBridge")
			print("Bridge ready, Linear Velocity: ", bridge.get_linear_velocity())
		else:
			print("Error: RosBridge singleton not found!")
	else:
		print("Error: RosBridge class not registered!")
