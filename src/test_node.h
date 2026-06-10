#ifndef TEST_NODE_H
#define TEST_NODE_H

#include <godot_cpp/classes/node3d.hpp>

namespace godot {

class TestNode : public Node3D {
    GDCLASS(TestNode, Node3D);

protected:
    static void _bind_methods();

public:
    TestNode();
    ~TestNode();

    void _ready() override; 
};

}

#endif