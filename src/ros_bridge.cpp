#include "ros_bridge.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

RosBridge *RosBridge::singleton = nullptr;

RosBridge::RosBridge() {
    if (singleton == nullptr) {
        singleton = this;
    }
}

RosBridge::~RosBridge() {
    if (singleton == this) {
        singleton = nullptr;
    }
}

RosBridge *RosBridge::get_singleton() {
    return singleton;
}

void RosBridge::_ready() {
    UtilityFunctions::print("RosBridge::_ready() wurde aufgerufen");
    if (singleton != this) {
        UtilityFunctions::printerr("WARNUNG: RosBridge existiert bereits. Diese Instanz wird gelöscht.");
        queue_free();
        return;
    }

    if (!rclcpp::ok()) {
        rclcpp::init(0, nullptr);
    }

    ros_node = rclcpp::Node::make_shared("godot_ros_bridge");
    
    auto qos = rclcpp::QoS(rclcpp::KeepLast(10)).transient_local();
    lidar_publisher = ros_node->create_publisher<sensor_msgs::msg::LaserScan>("/scan_laeth_bachelor", qos);    
    
    // TransformBroadcaster initialisieren
    tf_broadcaster = std::make_shared<tf2_ros::TransformBroadcaster>(ros_node);

    ros_executor = std::make_shared<rclcpp::executors::SingleThreadedExecutor>();
    ros_executor->add_node(ros_node);
    
    is_spinning = true;
    spin_thread = std::thread(&RosBridge::spin_ros2, this);
    
    UtilityFunctions::print("ROS2 Bridge Node erfolgreich gestartet und Publisher auf /scan bereit!");
}

void RosBridge::spin_ros2() {
    while (rclcpp::ok() && is_spinning) {
        ros_executor->spin_some(std::chrono::milliseconds(10));
    }
}

void RosBridge::_exit_tree() {
    is_spinning = false;
    if (spin_thread.joinable()) {
        spin_thread.join();
    }
    if (rclcpp::ok()) {
        rclcpp::shutdown();
    }
    UtilityFunctions::print("ROS2 Bridge sauber beendet.");
}

void RosBridge::publish_lidar_data(const std::string& frame_id, const std::vector<float>& ranges,
                                   double angle_min, double angle_max, double angle_increment,
                                   double range_min, double range_max, double update_rate,
                                   Transform3D global_transform) { // HIER DIE KOMPLETTE NEUE FUNKTION
    if (!ros_node || !lidar_publisher) return;

    rclcpp::Time current_time = ros_node->now();

    // 1. LiDAR Daten publizieren
    auto msg = sensor_msgs::msg::LaserScan();
    msg.header.stamp = current_time;
    msg.header.frame_id = frame_id;
    msg.angle_min = angle_min;
    msg.angle_max = angle_max;
    msg.angle_increment = angle_increment;
    msg.time_increment = 0.0;
    msg.scan_time = update_rate;
    msg.range_min = range_min;
    msg.range_max = range_max;
    msg.ranges = ranges;

    lidar_publisher->publish(msg);

    // 2. TF Daten publizieren (Koordinaten-Transformation für RViz)
    if (tf_broadcaster) {
        geometry_msgs::msg::TransformStamped t;
        
        t.header.stamp = current_time;
        t.header.frame_id = "map"; // Welt-Ursprung
        t.child_frame_id = frame_id; // "lidar_link"

        // --- KOORDINATEN-TRANSFORMATION (Godot -> ROS2) ---
        // Godot: X=Rechts, Y=Oben, Z=Hinten (Y-Up)
        // ROS2:  X=Vorne,  Y=Links, Z=Oben (Z-Up nach REP 103 Standard)

        // 1. Position (Translation) konvertieren
        t.transform.translation.x = -global_transform.origin.z;
        t.transform.translation.y = -global_transform.origin.x;
        t.transform.translation.z =  global_transform.origin.y;

        // 2. Rotation (Quaternion) konvertieren
        // Wir extrahieren die Basis-Vektoren aus Godot...
        Basis b = global_transform.basis;
        Vector3 x_g = b.get_column(0); // Rechts
        Vector3 y_g = b.get_column(1); // Oben
        Vector3 z_g = b.get_column(2); // Hinten

        // ...und bauen daraus eine neue Rotationsmatrix für ROS2
        Basis ros_basis;
        ros_basis.set_column(0, Vector3( z_g.z,  z_g.x, -z_g.y)); // ROS X (Vorne) = Godot -Z
        ros_basis.set_column(1, Vector3( x_g.z,  x_g.x, -x_g.y)); // ROS Y (Links) = Godot -X
        ros_basis.set_column(2, Vector3(-y_g.z, -y_g.x,  y_g.y)); // ROS Z (Oben)  = Godot Y

        Quaternion q_ros = ros_basis.get_rotation_quaternion();
        t.transform.rotation.x = q_ros.x;
        t.transform.rotation.y = q_ros.y;
        t.transform.rotation.z = q_ros.z;
        t.transform.rotation.w = q_ros.w;

        // An das ROS2 Netzwerk senden
        tf_broadcaster->sendTransform(t);
    }
}

void RosBridge::_bind_methods() {}