extends ROS2Publisher

func _ready() -> void:
	# As a subclass extending ROS2Publisher, this script directly inherits 
	# publishing capabilities, eliminating the need for external node references.
	print("ROS2Publisher script ready. Instance ID: ", self.get_instance_id())

func _input(event: InputEvent) -> void:
	# Trigger message broadcast on UI accept action (e.g., Enter or Spacebar key press)
	if event.is_action_pressed("ui_accept"):
		print("Broadcasting message...")
		# Invoke the native C++ method directly on the instance
		self.publish_message("Hello ROS2, greetings from the C++ instance!")
