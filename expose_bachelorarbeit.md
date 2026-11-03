# Exposé – Bachelorarbeit

**Arbeitstitel (Arbeitsversion):**
> Entwicklung und Evaluation einer Godot-basierten Simulationsumgebung für mobile ROS2-Roboter hinsichtlich Echtzeit-Performance und Eignung für autonome Navigation (Nav2)

**Studiengang / Hochschule:** (eintragen)
**Bearbeiter/in:** (Name & Matrikelnummer)
**Erstgutachter/in / Betreuer/in:** (Name)
**Abgabetermin (geplant):** (Datum)

---

## 1. Ausgangslage und Motivation

Im Rahmen der Praxisphase wurde am Unternehmen eine Robotersimulation für mobile, ROS2-basierte Roboter auf Basis der **Godot Engine** entwickelt. Die Grundlage ist eine native C++-Schnittstelle (GDExtension, `RosBridge`), die einen Echtzeit-Kommunikationskanal zwischen Godot und einem ROS2-Netzwerk bereitstellt. Bereits umgesetzt und stabil lauffähig sind:

- **LiDAR** – `sensor_msgs/msg/LaserScan` auf `/lidar_sensor` (10 Hz)
- **IMU** – `sensor_msgs/msg/Imu` auf `/imu/data` (60 Hz)
- **Odometry** – `nav_msgs/msg/Odometry` auf `/odom` (~30 Hz)

sowie die Ansteuerung des simulierten Roboters über `/cmd_vel` aus ROS2 heraus.

Diese Grundlage bildet die technische Basis der vorliegenden Bachelorarbeit. Während die Funktionsfähigkeit der Umgebung bereits gegeben ist, fehlt bisher ein **systematischer Nachweis ihrer Eignung** als verlässliche Simulationsplattform für ROS2-Robotik – insbesondere für den anspruchsvollen Anwendungsfall der **autonomen Navigation (Nav2-Stack)**. Genau diese Lücke adressiert die Arbeit.

---

## 2. Zielsetzung und Forschungsfrage

Ziel der Arbeit ist es, die Godot-basierte Simulation nicht nur weiterzuentwickeln und zu strukturieren, sondern **ihre Eignung als Echtzeit-Simulationsumgebung für mobile ROS2-Roboter messbar zu belegen** und ihre **Nutzbarkeit für den Nav2-Stack als Fallbeispiel** zu demonstrieren.

**Übergeordnete Forschungsfrage:**

> **„Inwieweit eignet sich eine Godot-basierte Simulationsumgebung als Echtzeit-Plattform für mobile ROS2-Roboter zur autonomen Navigation?“**

### Beantwortbare Teilfragen (messbar)

**A) Performance / Echtzeitverhalten**
- **TF1 (Steuerlatenz):** Wie hoch ist die Ende-zu-Ende-Latenz zwischen dem Senden eines `/cmd_vel`-Steuerbefehls und der sichtbaren Roboterbewegung in der Godot-Simulation, und bleibt sie über die Laufzeit stabil?
- **TF2 (Echtzeitfaktor):** Erreicht die Simulation bei gleichzeitig aktiver Sensorik (LiDAR 10 Hz, IMU 60 Hz, Odometry ~30 Hz) einen Echtzeitfaktor ≥ 1,0, und wie skaliert dieser mit steigender Szenenkomplexität (leere Welt vs. Warehouse)?
- **TF3 (Sensor-Timing):** Werden die Soll-Frequenzen der Sensoren über die Laufzeit stabil erreicht, d.h. wie stark streut (jittert) das tatsächliche Veröffentlichungsintervall?

**C) Eignung für autonome Navigation (Nav2 – Demonstrator)**
- **TF4 (Datenqualität/Voraussetzung):** Erfüllen die Topics die von Nav2 geforderten Frame- und QoS-Konventionen (TF-Kette `odom → base_link → lidar_link/imu_link`), sodass eine Nav2-basierte Lokalisierung (AMCL) und Pfadplanung darauf stabil arbeiten können?
- **TF5 (funktionaler Nachweis):** Kann der Nav2-Stack an die Godot-Simulation angeschlossen werden und eine definierte Zielpose autonom erreichen (lokalisieren, planen, regeln)?

### Arbeitshypothesen
Zur Prüfbarkeit werden die folgenden (widerlegbaren) Hypothesen formuliert:
- **H1:** Die Steuerlatenz (`/cmd_vel` → Bewegung) liegt unter 50 ms und driftet über 30 Minuten Laufzeit nicht signifikant.
- **H2:** Die Sensor-Veröffentlichungsfrequenzen weichen bei aktivem Messrauschen um weniger als ±10 % vom Sollwert ab (geringer Jitter).
- **H3:** Die Simulation erreicht bei realistischer Sensorlast einen Echtzeitfaktor ≥ 1,0 und kann einen Nav2-Stack zur autonomen Zielanfahrt stabil versorgen.

---

## 3. Vorgehensweise und Methodik

1. **Weiterentwicklung & Strukturierung:** Modularisierung der bestehenden GDExtension-Implementierung, Dokumentation der drei Säulen (`RosBridge`, `ROS2SensorBase`, `ROS2Publisher`).
2. **Sauberes Fundament:** Verifikation von Bewegungsmodell (Differenzialantrieb) und TF-Kette `odom → base_link → lidar_link/imu_link` als Voraussetzung für alle Messungen.
3. **Versuchsaufbau:** Feste Testumgebung auf Basis der existierenden Demo-/Warehouse-Szene mit definierten Roboterpfaden und identischem Startzustand für reproduzierbare Läufe.
4. **Messdurchführung:** Systematische Messungen über mehrere Wiederholungen je Szenario (Latenz, Echtzeitfaktor, Frequenzstabilität, Ressourcen CPU/RAM/GPU) mit dokumentierter Messmethodik, Auswertung nach Mittelwert und Streuung.
5. **Nav2-Demonstrator:** Anbindung des Nav2-Stacks als Fallbeispiel; Nachweis einer autonomen Anfahrt einer Zielpose.
6. **Diskussion & Ausblick:** Bewertung der Ergebnisse, Grenzen der Umgebung, Empfehlungen.

---

## 4. Abgrenzung (Scoping)

Um den Umfang einer Bachelorarbeit einhaltbar zu gestalten, werden folgende Punkte **nicht** bzw. nur am Rande behandelt:
- **Keine vollständige Optimierung des Nav2-Stacks:** Nav2 dient als *Fallbeispiel/Demonstrator*, nicht als Gegenstand einer tiefgehenden Tuning-Studie.
- **Kein Vergleich mit kommerziellen Simulatoren** (z.B. Gazebo, Webots) über kurze qualitative Einordnung in den Stand der Technik hinaus.
- **Keine Implementierung neuer Sensortypen** über die vorhandene Trias (LiDAR, IMU, Odometry) hinaus.
- **Keine Unterstützung für Multi-Roboter-Simulationen**; Fokus liegt auf einem Einzelroboter.

---

## 5. Geplante Gliederung

1. Einleitung (Motivation, Zielsetzung, Aufbau der Arbeit)
2. Grundlagen (ROS2, mobile Robotik & autonome Navigation, Simulationsumgebungen: Godot vs. Gazebo/Webots)
3. Stand der Technik & Abgrenzung zum Eigenentwurf
4. Architektur & Weiterentwicklung der Godot-ROS2-Simulation
5. Versuchsaufbau & Messmethodik (Evaluationskriterien)
6. Ergebnisse (Performance, Echtzeitverhalten, Sensordatenkonsistenz)
7. Nav2-Demonstrator: autonome Navigation in der Godot-Simulation
8. Diskussion, Grenzen & Ausblick
9. Zusammenfassung

---

## 6. Geplanter Zeitplan (Indikativ)

| Phase | Inhalt | Dauer |
|---|---|---|
| 1 | Strukturierung/Modularisierung, TF & Bewegungsmodell verifizieren | 1–2 Wochen |
| 2 | Versuchsaufbau & Messmethodik festlegen | 1–2 Wochen |
| 3 | Messungen durchführen & auswerten | 2–3 Wochen |
| 4 | Nav2-Demonstrator | 2–4 Wochen (skalierbar) |
| 5 | Ausarbeitung & Korrektur | Rest |

> Puffer für die Nav2-Anbindung ist bewusst eingeplant (größtes unvorhersehbares Einrichtungspotenzial).

---

## 7. Erwarteter Beitrag / Ergebnis

- Ein funktionsfähiges, modular aufgebautes und dokumentiertes Godot-/ROS2-Simulationsframework.
- Reproduzierbare Evaluationsergebnisse zu Latenz, Echtzeitfaktor und Sensordatenkonsistenz.
- Ein Nachweis (Fallbeispiel), dass die Umgebung eine autonome Navigation nach Nav2 unterstützt.
- Eine klare wissenschaftliche Einordnung der Eignung von Godot für die ROS2-Robotiksimulation.

---

*Stand: [Datum] — zur Abstimmung mit dem Betreuer/eingereicht bei:*
