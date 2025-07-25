#pragma once
#include <glfw/glfw3.h>
#include "imgui.h"

#include "canvas.h"

struct DebugState {
    double dt;
    ImVec2 mouse_pos;
};

// GUI class responsible for defining the interface layout in Dear ImGui.
class GUI {
private:
    ImVec2 m_canvas_window_pos;

public: 
    GUI(GLFWwindow* window);
    ~GUI();

    void define_interface(Canvas& canvas, DebugState debug_state);

    ImVec2 get_mouse_position_on_canvas_window(double mouse_x, double mouse_y);
};

