extends ROS2Publisher 

func _ready():
	# Wir sind jetzt Teil des ROS2Publisher-Objekts.
	# Da wir selbst ein Publisher sind, brauchen wir kein '$' oder externe Referenz.
	print("ROS2Publisher Skript bereit. Instanz-ID: ", self.get_instance_id())

func _input(event): 
	if event.is_action_pressed("ui_accept"): 
		print("Sende Nachricht...")
		# Ruf die Funktion direkt auf 'self' auf
		self.publish_message("Hallo ROS2, hier spricht die C++ Instanz!")
