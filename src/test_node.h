#ifndef TEST_NODE_H
#define TEST_NODE_H

#include <godot_cpp/classes/node3d.hpp>

namespace godot {

class TestNode : public Node3D {
    GDCLASS(TestNode, Node3D); // Magisches Godot-Makro

protected:
    static void _bind_methods(); // Hier melden wir Funktionen an Godot an

public:
    TestNode();
    ~TestNode();

    // Diese Funktion wird von Godot aufgerufen, wenn der Node bereit ist
    void _ready() override; 
};

}

#endif