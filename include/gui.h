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

    std::optional<std::string> m_alert_message;

public: 
    GUI(GLFWwindow* window);
    ~GUI();

    void define_interface(Canvas& canvas, DebugState debug_state);

    void define_color_picker_window(UserState& user_state);
    void define_brush_window(UserState& user_state);
    void define_brush_properties_window(UserState& user_state);
    void define_layer_window(Canvas& canvas);
    void define_canvas_window(Canvas& canvas);
    void define_debug_window(DebugState& debug_state, UserState& user_state);
    void define_error_popup();
    void define_layer_buttons(Canvas& canvas);
    void define_layer_list(Canvas& canvas);

    ImVec2 get_mouse_position_on_canvas_window(double mouse_x, double mouse_y);
};

