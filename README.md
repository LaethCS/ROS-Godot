## Über das Projekt

Dieses Plugin ist Teil der Bachelorarbeit mit dem Titel "Konzeption und Entwicklung eines ROS2-fähigen Add-ons für die Godot-Engine zur Integration vereinfachter Robotersimulationen". Es ermöglicht eine latenzarme Echtzeit-Kommunikation zwischen der Godot-Spielengine und dem Industrie-Standard-Robotik-Framework ROS2.

Anstatt auf langsame Netzwerk-Schnittstellen wie WebSockets zu setzen, wurde in C++ ein nativer Echtzeit-Dolmetscher (eine GDExtension) programmiert. Das Plugin enthält als ersten Machbarkeitsnachweis (Proof of Concept) einen virtuellen Abstandssensor (`ros2_abstandsensor`). In der 3D-Simulation schießt dieser Sensor einen unsichtbaren Laserstrahl (RayCast3D) ab, berechnet physikalische Kollisionen und funkt die exakte Entfernung als `std_msgs/Float32` direkt in das ROS2-Netzwerk. Durch eine Drosselung auf die Physik-Engine liefert der Sensor stabile Werte mit exakt 25 Hz.

---

## Systemvoraussetzungen

Die Verteilung dieses GDExtension-Add-ons ist extrem einfach, da lediglich die kompilierte `.so` Bibliothek weitergegeben wird. Für die Nutzung müssen folgende Basis-Systeme installiert sein:

| Komponente | Version / Spezifikation |
| --- | --- |
| **Betriebssystem** | Ubuntu 22.04 LTS (Jammy Jellyfish) 

 |
| **ROS2** | Humble Hawksbill 

 |
| **Game Engine** | Godot 4.x (z.B. Godot 4.5 für Linux) 

 |
| **Shell** | Bash 

 |

---

## ⚠️ Wichtiger Start-Hinweis (Kritisch)

Godot darf niemals einfach per Doppelklick über ein Desktop-Icon gestartet werden, da sonst die ROS2-Umgebungsvariablen fehlen und die Engine lautlos im Hintergrund abstürzt. Die Engine muss zwingend aus einem vorbereiteten Terminal gestartet werden.

1. Öffne ein neues Ubuntu-Terminal.
2. Lade die ROS2-Umgebung für Bash mit `source /opt/ros/humble/setup.bash`.


3. Alternativ, falls Zsh genutzt wird, lade die Umgebung mit `source /opt/ros/humble/setup.zsh`.


4. Starte die ausführbare Godot-Datei (z.B. `./Godot_v4.5-stable_linux.x86_64`) exakt aus diesem Terminal heraus.



---

## Installation (Plug-and-Play)

Das Plugin ist so konzipiert, dass die gesamte Komplexität der ROS2-Integration für den Nutzer unsichtbar gekapselt ist.

1. Entpacke die heruntergeladene ZIP-Datei auf dem Rechner.


2. Öffne ein leeres oder bestehendes Godot-Projekt über das vorbereitete ROS2-Terminal.
3. Ziehe den entpackten Ordner `addons` (welcher den Unterordner `ros2_abstandsensor` enthält) per Drag-and-Drop direkt in das Dateisystem-Fenster (FileSystem Dock) des Godot-Editors.


4. Klicke oben in der Godot-Menüleiste auf **Projekt** und wähle **Projekteinstellungen** aus.


5. Wechsle im sich öffnenden Fenster auf den Reiter **Plugins**.


6. Setze ganz rechts den Haken bei **Aktivieren** (Enable), um das Plugin offiziell anzumelden.



---

## Verwendung & Testen

Sobald das Plugin aktiviert ist, kann der Sensor völlig ohne C++ Kenntnisse in der Engine genutzt werden.

1. Öffne eine beliebige 3D-Szene im Godot-Editor.


2. Drücke **Strg + A** (oder klicke auf das Plus-Symbol "Knoten hinzufügen") und tippe `TestSensor` in die Suchleiste ein.


3. Platziere den C++ Knoten in der Welt und platziere ein Objekt (z.B. einen Würfel mit `StaticBody3D` und `CollisionShape3D`) in seinem Strahl.


4. Drücke in Godot auf den **Play**-Button (F5), um die Simulation zu starten.


5. Öffne ein zweites, komplett neues Terminal und source die ROS2-Umgebung erneut.


6. Tippe den Befehl `ros2 topic echo /sensor_distance` ein, um dich live auf den Kanal des Godot-Sensors aufzuschalten.


7. Beobachte das Terminal: Sobald das Hindernis in Godot bewegt wird, kommen die Distanz-Daten in Echtzeit im Ubuntu-Terminal an.
