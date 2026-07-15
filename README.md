# ROS2-Godot Integration Framework

## Über das Projekt

Dieses Plugin ist Teil der Bachelorarbeit mit dem Titel "Konzeption und Entwicklung eines ROS2-fähigen Add-ons für die Godot-Engine zur Integration vereinfachter Robotersimulationen". Es ermöglicht eine latenzarme Echtzeit-Kommunikation zwischen der Godot-Spielengine und dem Industrie-Standard-Robotik-Framework ROS2.

Anstatt auf langsame Netzwerk-Schnittstellen wie WebSockets zu setzen, wurde in C++ ein nativer Echtzeit-Dolmetscher (eine GDExtension) programmiert. 

**Kern-Features & Architektur:**
* **Sensor-Framework:** Begonnen als Machbarkeitsnachweis mit einem simplen Abstandssensor (`ros2_abstandsensor` bei 25 Hz), beinhaltet das Plugin nun komplexe Sensoren wie **2D-LiDAR**, welche industriestandardisierte `sensor_msgs/LaserScan` publizieren.
* **Dynamisches TF-Broadcasting:** Das Plugin übermittelt automatisch den Transformationsbaum (TF) zwischen der Welt und dem Roboter, was die direkte Nutzung im ROS2-Navigations-Stack (Nav2) und in RViz ermöglicht.
* **Bidirektionale Steuerung:** Über implementierte Subscriber lauscht das System auf Topics wie `/cmd_vel` (`geometry_msgs/msg/Twist`) und übersetzt diese threadsicher in physikalische Bewegungen innerhalb der Godot-Engine.
* **Abstraktionsschicht (Developer Experience):** C++-Komplexität ist vollständig gekapselt. Über eine generische Basis-Klasse (`ROS2SensorBase`) und Godot-Signale können Endnutzer ROS2-Knoten ohne C++-Kenntnisse direkt im Godot-Editor via Drag-and-Drop (Plug-and-Play) konfigurieren und über GDScript ansteuern.

---

## Systemvoraussetzungen

Die Verteilung dieses GDExtension-Add-ons ist extrem einfach, da lediglich die kompilierte `.so` Bibliothek weitergegeben wird. Für die Nutzung müssen folgende Basis-Systeme installiert sein:

| Komponente | Version / Spezifikation |
| --- | --- |
| **Betriebssystem** | Ubuntu 22.04 LTS (Jammy Jellyfish) |
| **ROS2** | Humble Hawksbill |
| **Game Engine** | Godot 4.x (getestet mit Godot 4.5 für Linux) |
| **Shell** | Bash oder Zsh |

---

## ⚠️ Wichtiger Start-Hinweis (Kritisch)

Godot darf **niemals** einfach per Doppelklick über ein Desktop-Icon gestartet werden, da sonst die ROS2-Umgebungsvariablen fehlen und die Engine lautlos im Hintergrund abstürzt oder das Plugin nicht geladen werden kann. Die Engine muss zwingend aus einem vorbereiteten Terminal gestartet werden:

1. Öffne ein neues Terminal.
2. Lade die ROS2-Umgebung: 
   * Für Bash: `source /opt/ros/humble/setup.bash`
   * Für Zsh: `source /opt/ros/humble/setup.zsh`
3. Starte die ausführbare Godot-Datei (z.B. `./Godot_v4.5-stable_linux.x86_64`) exakt aus diesem Terminal heraus.

---

## Installation (Plug-and-Play)

Das Plugin ist so konzipiert, dass die gesamte Komplexität der ROS2-Integration für den Nutzer unsichtbar gekapselt ist.

1. Entpacke die heruntergeladene ZIP-Datei deines Plugins.
2. Öffne ein leeres oder bestehendes Godot-Projekt über das vorbereitete ROS2-Terminal.
3. Ziehe den entpackten Ordner `addons` (welcher den Unterordner mit der Plugin-Struktur enthält) per Drag-and-Drop direkt in das **Dateisystem-Fenster** (FileSystem Dock) des Godot-Editors.
4. Klicke oben in der Godot-Menüleiste auf **Projekt** und wähle **Projekteinstellungen** aus.
5. Wechsle im sich öffnenden Fenster auf den Reiter **Plugins**.
6. Setze ganz rechts den Haken bei **Aktivieren** (Enable), um das Plugin in der Engine zu registrieren.

---

## Verwendung & Testen

Sobald das Plugin aktiviert ist, können die ROS2-Knoten völlig ohne C++-Kenntnisse in der Engine genutzt werden.

1. Öffne eine beliebige 3D-Szene im Godot-Editor.
2. Drücke **Strg + A** (Knoten hinzufügen) und tippe z.B. `TestSensor` oder deine neuen abstrakten Publisher-Knoten in die Suchleiste ein.
3. Platziere den C++-Knoten in der Welt und ordne ihm in Godot ein physikalisches Objekt (z.B. `StaticBody3D` mit `CollisionShape3D`) zu.
4. Drücke den **Play**-Button (F5), um die Simulation zu starten.
5. Öffne ein zweites Terminal und source die ROS2-Umgebung erneut.
6. **Daten empfangen:** Tippe `ros2 topic echo /sensor_distance` (oder das entsprechende Topic), um die Sensordaten (z.B. Lidar-Scans oder Distanzen) in Echtzeit im Terminal zu sehen.
7. **Roboter steuern:** Sende eine Twist-Nachricht über `/cmd_vel` im Terminal, um zu beobachten, wie sich das Objekt in Godot physikalisch bewegt.