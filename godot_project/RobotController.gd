extends CharacterBody3D

# Geschwindigkeitsparameter
const SPEED = 5.0
const ROTATION_SPEED = 2.0

# Referenz zur globalen Brücke
var ros_bridge = null

func _ready():
	
	
	# Wir holen uns das registrierte Singleton zur Laufzeit
	if Engine.has_singleton("RosBridge"):
		ros_bridge = Engine.get_singleton("RosBridge")
	else:
		push_error("ROSBridge Singleton wurde nicht gefunden! Ist das Addon aktiviert?")

func _physics_process(delta):
	if not ros_bridge:
		print("hello from not if not ros_bridge")
		return

	var linear = ros_bridge.get_linear_velocity()
	var angular = ros_bridge.get_angular_velocity()
	if linear != 0.0:
		print("Linear Velocity empfangen: ", linear)
	# Debug: Schau im Godot Output Fenster, ob hier Zahlen > 0 stehen!
	# Wenn hier nur 0.0 steht, kommt das Signal nicht in Godot an.
	print("Linear: ", linear, " Angular: ", angular) 

	rotate_y(-angular * ROTATION_SPEED * delta)
	var direction = -transform.basis.z 
	velocity = direction * linear * SPEED
	
	move_and_slide()
