# ROS2-Godot Integration Framework

## Über das Projekt

Dieses Plugin ist Teil der Bachelorarbeit mit dem Titel "Konzeption und Entwicklung eines ROS2-fähigen Add-ons für die Godot-Engine zur Integration vereinfachter Robotersimulationen". Es ermöglicht eine latenzarme Echtzeit-Kommunikation zwischen der Godot-Spielengine und dem Industrie-Standard-Robotik-Framework ROS2.

Anstatt auf langsame, unzuverlässige Netzwerk-Umwege wie WebSockets zu setzen, wurde in C++ ein nativer Echtzeit-Dolmetscher als **GDExtension** programmiert. Das Projekt bietet eine hochperformante Middleware-Brücke, die komplexe ROS2-Strukturen abstrahiert und für Simulationen innerhalb von Godot per Plug-and-Play zugänglich macht.

---

## Kern-Architektur & Komponenten

Das Framework ist modular aufgebaut und trennt die Low-Level-Kommunikation strikt von der High-Level-Logik der Game Engine. Es basiert auf drei tragenden Säulen:

### 1. Das Kommunikations-Zentrum (`RosBridge`)
Die Klasse `RosBridge` bildet das datentechnische Rückgrat des gesamten Plugins.
* **Multithreading:** Sie kapselt den ROS2-Ausführungskontext und führt den Executor-Loop (`spin_ros2`) in einem dedizierten Hintergrund-Thread aus. Dadurch bleibt die Render-Performance von Godot völlig unbeeinflusst von der Netzwerklast.
* **Bidirektionalität & Thread-Sicherheit:** Über `RosBridge` laufen sowohl Publisher als auch Subscriber zusammen. Sie lauscht beispielsweise auf Steuersignale wie `/cmd_vel` (`geometry_msgs/msg/Twist`) und puffert eintreffende Daten asynchron ab. Der Datenzugriff zwischen dem ROS2-Thread und dem synchronen Physik-Thread von Godot wird über `std::mutex` strikt abgesichert, um Race Conditions zu verhindern.
* **TF-Broadcasting:** `RosBridge` übernimmt das dynamische Broadcasting des Transformationsbaums (TF2) zwischen Simulations-Koordinatensystemen (`base_link`, `lidar_link`), wodurch die Weltdaten direkt mit Tools wie RViz oder Nav2 kompatibel sind.

### 2. Die C++ Sensor-Basisklasse (`ROS2SensorBase`)
Um die Implementierung neuer Sensortypen zu vereinheitlichen, erben alle simulierten Sensoren von der abstrakten Basisklasse `ROS2SensorBase`.
* **Lifecycle-Management:** Sie automatisiert die Node-Registrierung an der zentralen `RosBridge` und hält den Boilerplate-Code für Entwickler minimal.
* **Frequenz-Synchronisation:** Da Godots Render-Schleife (`_process`) variabel ist, physikalische Sensoren aber feste Abtastraten erfordern, implementiert `ROS2SensorBase` eine akkumulatorbasierte Zeit-Drosselung. Dadurch arbeiten alle abgeleiteten Sensoren mit exakt definierten, stabilen Frequenzen (z. B. 25 Hz), unabhängig von den Frames per Second (FPS) des Grafikrechners.

### 3. Der ROS2 Message Builder (`ROS2Publisher`)
Der `ROS2Publisher` dient als universelle Schnittstelle zur Datenübertragung und fungiert als flexibler Nachrichtengenerator.
* **Editor-Introspektion (DX):** Über Godots Reflexionsmechanismus (`_bind_methods` und `ADD_PROPERTY`) macht der Knoten Parameter wie Topic-Namen, Frame-IDs und QoS-Profile direkt im grafischen Godot-Inspector (GUI) konfigurierbar.
* **GDScript-Anbindung:** Er konvertiert eintreffende Datenströme dynamisch in das Ziel-ROS2-Nachrichtenformat. Entwickler können somit Sensordaten oder benutzerdefinierte Zustände via **GDScript** über Standard-Godot-Signale (z. B. `emit_signal("on_data_ready")`) direkt ins DDS-Netzwerk publizieren, ohne C++ anfassen zu müssen.

---

## Enthaltene Proof-of-Concept-Sensoren

Das Add-on liefert zwei einsatzbereite Sensorknoten mit, die die Mächtigkeit der Architektur demonstrieren:

* **`LidarSensor` (Komplexer PoC):** Erbt von `ROS2SensorBase` und simuliert einen rotierenden Laserscanner. Er tastet die Umgebung über ein Array aus physikalischen `RayCast3D`-Strahlen ab, berechnet Distanzen und transformiert diese in eine standardisierte **`sensor_msgs/msg/LaserScan`**-Nachricht, die auf dem Topic `/scan` veröffentlicht wird.
* **`TestSensor` (Einfacher PoC):** Ein Single-Point-Distanzmesser, der auf einem zugewiesenen `RayCast3D`-Kindknoten aufbaut. Er ermittelt die lineare Distanz zu einem Objekt und publiziert den Wert als **`std_msgs/msg/Float32`** auf dem Topic `/sensor_distance` mit stabilen 25 Hz.

---

## Systemvoraussetzungen

Die Verteilung dieses GDExtension-Add-ons ist extrem effizient, da lediglich die kompilierte `.so` Bibliothek weitergegeben wird. 

| Komponente | Version / Spezifikation |
| --- | --- |
| **Betriebssystem** | Ubuntu 22.04 LTS (Jammy Jellyfish) |
| **ROS2** | Humble Hawksbill |
| **Game Engine** | Godot 4.x (getestet mit Godot 4.5 für Linux) |
| **Shell** | Bash oder Zsh |

---

## ⚠️ Wichtiger Start-Hinweis (Kritisch)


1. Öffne ein neues Terminal.
2. Lade die ROS2-Umgebung: 
   * Für Bash: `source /opt/ros/humble/setup.bash`
   * Für Zsh: `source /opt/ros/humble/setup.zsh`
3. Navigiere zum Godot-Verzeichnis und starte die ausführbare Datei (z.B. `./Godot_v4.5-stable_linux.x86_64`) exakt aus diesem Terminal heraus.

---

## Installation & Verwendung (Plug-and-Play)

1. Entpacke die ZIP-Datei des Plugins.
2. Kopiere oder ziehe den Ordner `addons` per Drag-and-Drop direkt in das **Dateisystem-Fenster** (FileSystem Dock) deines Godot-Editors.
3. Klicke in der Menüleiste auf **Projekt** -> **Projekteinstellungen** -> Reiter **Plugins**.
4. Setze ganz rechts den Haken bei **Aktivieren** (Enable), um die GDExtension-Bibliothek in der Engine zu registrieren.
5. Drücke im Editor **Strg + A**, füge entweder einen `LidarSensor` oder einen `TestSensor` zu deiner 3D-Szene hinzu und platziere kollisionsfähige Objekte (`StaticBody3D`) in deren Erfassungsbereich.
6. Starte die Simulation mit **F5**.
7. Öffne ein zweites Terminal, source die ROS2-Umgebung und fange die Daten live ab:
   * LiDAR: `ros2 topic echo /scan`
   * Abstand: `ros2 topic echo /sensor_distance`

---

## Für Entwickler: Eigene Sensoren erweitern

Dank der Abstraktionsschicht ist das Hinzufügen neuer Funktionalitäten extrem geradlinig:

1. **C++ Klasse von `ROS2SensorBase` ableiten:** Deklariere die Klasse im Header und überschreibe die `_physics_process(double delta)` Schleife für die Sensor-Datenerfassung.
2. **Methoden binden:** Registriere Variablen und Funktionen in der Methode `_bind_methods()`, um sie für Godots GUI sichtbar zu machen.
3. **In `register_types.cpp` anmelden:** Füge den Befehl `ClassDB::register_class<DeineNeueKlasse>();` in das Initialisierungsmodul ein.
4. **Kompilieren (Build-Prozess):** Führe in deinem Terminal den SCons-Build-Befehl aus (z.B. `scons platform=linux target=template_debug`), um deine Änderungen in die `.so`-Bibliothek zu kompilieren. Nach einem Neustart von Godot erscheint der neue Knoten sofort im Editor-Suchmenü.