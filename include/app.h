#pragma once

#include <optional>
#include <queue>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "bounding_box.h"
#include "canvas.h"
#include "gui.h"
#include "window.h"

class App {
private:
    unsigned int m_screen_width;
    unsigned int m_screen_height;
    unsigned int m_canvas_display_width;
    unsigned int m_canvas_display_height;

    Window m_window;
    GUI m_gui;
    Canvas m_canvas;

    double m_last_update_time;
    double m_last_dt;

public:
    App(
        unsigned int screen_width,
        unsigned int screen_height,
        unsigned int canvas_width,
        unsigned int canvas_height
    );

    void run();

private:
    void handle_inputs();
    ImVec2 get_mouse_position_on_canvas();
    DebugState generate_debug_state();
    void render();
};

