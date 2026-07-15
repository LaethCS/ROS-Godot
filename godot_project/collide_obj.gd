extends Node3D
func _ready():
	# Wir prüfen hier, ob Godot die Klasse überhaupt sieht
	if ClassDB.class_exists("RosBridge"):
		print("Erfolg: Klasse RosBridge existiert!")
		
		# Teste, ob das Singleton (die Instanz) da ist
		if Engine.has_singleton("RosBridge"):
			print("Erfolg: Singleton RosBridge gefunden!")
			var bridge = Engine.get_singleton("RosBridge")
			print("Brücke bereit, Linear Velocity: ", bridge.get_linear_velocity())
		else:
			print("Fehler: Singleton RosBridge nicht gefunden!")
	else:
		print("Fehler: Klasse RosBridge nicht registriert!")
