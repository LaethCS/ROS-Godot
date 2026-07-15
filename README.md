# ROS2-Godot Integration Framework

## Über das Projekt

Dieses Plugin ist Teil der Bachelorarbeit mit dem Titel "Konzeption und Entwicklung eines ROS2-fähigen Add-ons für die Godot-Engine zur Integration vereinfachter Robotersimulationen". Es ermöglicht eine latenzarme Echtzeit-Kommunikation zwischen der Godot-Spielengine und dem Industrie-Standard-Robotik-Framework ROS2.

Anstatt auf langsame Netzwerk-Schnittstellen wie WebSockets zu setzen, wurde in C++ ein nativer Echtzeit-Dolmetscher (eine GDExtension) programmiert. 

Das Add-on bringt direkt **zwei einsatzbereite Proof-of-Concept-Sensoren (Plug-and-Play)** mit, die ohne C++-Kenntnisse sofort in Godot genutzt werden können:

### 1. Virtueller 2D-LiDAR-Sensor (`LidarSensor`) - *Komplexer PoC*
Dieser Sensor simuliert einen echten rotierenden Laserscanner für Navigations- und SLAM-Aufgaben.
* **Funktionsweise:** Tastet die Umgebung über rotierende bzw. gefächerte physikalische Strahlen ab und berechnet in Echtzeit die exakten Kollisionspunkte zu Hindernissen.
* **Datenübertragung:** Konvertiert die gemessenen Abstände direkt in das standardisierte, industrielle Datenformat **`sensor_msgs/msg/LaserScan`**.
* **Frequenz & Netz:** Die Daten werden threadsicher direkt in das ROS2-DDS-Netzwerk auf das Topic **/scan** gefunkt. Damit ist dieser Sensor sofort kompatibel mit SLAM-Algorithmen und kann direkt in **RViz** visualisiert werden.

### 2. Virtueller Abstandssensor (`TestSensor`) - *Einfacher PoC*
* **Funktionsweise:** Dieser C++ Knoten baut auf einem untergeordneten Godot **`RayCast3D`**-Kindknoten auf. Er schießt einen unsichtbaren Laserstrahl nach vorne ab, nutzt die physikalische Kollisionsabfrage von Godot und berechnet die exakte Distanz zu einem physischen Hindernis.
* **Datenübertragung:** Sendet die gemessene Entfernung als einfache Fließkommazahl im Format **`std_msgs/msg/Float32`** direkt in das ROS2-Netzwerk auf das Topic **/sensor_distance**.
* **Synchronisation:** Durch eine feste Drosselung auf die Physik-Engine (Akkumulator-Logik) liefert der Sensor stabile Werte mit exakt 25 Hz, um ein Flackern oder Desynchronisationen zwischen Render-Frames und Physik-Schritten zu verhindern.

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

Godot darf **niemals** einfach per Doppelklick über ein Desktop-Icon gestartet werden, da sonst die ROS2-Umgebungsvariablen fehlen und die Engine lautlos im Hintergrund abstürzt oder das GDExtension-Plugin nicht geladen werden kann. Die Engine muss zwingend aus einem vorbereiteten Terminal gestartet werden:

1. Öffne ein neues Terminal.
2. Lade die ROS2-Umgebung: 
   * Für Bash: `source /opt/ros/humble/setup.bash`
   * Für Zsh: `source /opt/ros/humble/setup.zsh`
3. Starte die ausführbare Godot-Datei (z.B. `./Godot_v4.5-stable_linux.x86_64`) exakt aus diesem Terminal heraus.

---

## Installation (Plug-and-Play)

Das Plugin ist so konzipiert, dass die gesamte Komplexität der ROS2-Integration für den Nutzer unsichtbar gekapselt ist.

1. Entpacke die ZIP-Datei des Plugins.
2. Öffne ein leeres oder bestehendes Godot-Projekt über das vorbereitete ROS2-Terminal.
3. Ziehe den entpackten Ordner `addons` per Drag-and-Drop direkt in das **Dateisystem-Fenster** (FileSystem Dock) des Godot-Editors.
4. Klicke oben in der Godot-Menüleiste auf **Projekt** -> **Projekteinstellungen** -> Reiter **Plugins**.
5. Setze ganz rechts den Haken bei **Aktivieren** (Enable), um das Plugin in der Engine zu registrieren.

---

## Verwendung & Testen (Beide Sensoren)

Sobald das Plugin aktiviert ist, kannst du die Sensoren ohne Programmierung sofort in deiner 3D-Simulation testen:

1. Öffne eine beliebige 3D-Szene im Godot-Editor.
2. Drücke **Strg + A** (Knoten hinzufügen), suche nach dem gewünschten Sensor-Knoten:
   * Füge `LidarSensor` für den LiDAR-PoC hinzu.
   * Füge `TestSensor` für den einfachen Abstandssensor hinzu.
3. Platziere Objekte mit Kollisionen (z. B. Wände/Würfel mit `StaticBody3D` und `CollisionShape3D`) im Scanbereich deines Sensors.
4. Drücke **F5** (Simulation starten).
5. Öffne ein zweites, neues Terminal und lade die ROS2-Umgebung (`source /opt/ros/humble/setup.bash` bzw. `.zsh`).

### Daten live abfangen:
* **Für den LiDAR-Sensor:** Tippe folgenden Befehl ein:
  ```bash
  ros2 topic echo /scan