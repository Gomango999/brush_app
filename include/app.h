#pragma once
#include <string>

#include <glm/fwd.hpp>
#include <imgui.h>

#include "canvas.h"
#include "gui.h"
#include "tools.h"
#include "user_state.h"
#include "window.h"
#include <optional>

class App {
public:
    App(
        unsigned int screen_width,
        unsigned int screen_height,
        unsigned int canvas_width,
        unsigned int canvas_height,
        unsigned int canvas_display_width,
        unsigned int canvas_display_height
    );

    void run();

private:
    unsigned int m_screen_width;
    unsigned int m_screen_height;

    Window m_window;
    GUI m_gui;
    Canvas m_canvas;
    ToolManager m_tool_manager;
    UserState m_user_state;

    double m_last_update_time;
    double m_last_dt;
    const double m_target_internal_fps = 120.0;
    const double m_target_internal_dt = 1.0 / m_target_internal_fps;
    const double m_target_display_fps = 60.0;
    const double m_target_display_dt = 1.0 / m_target_display_fps;

    void handle_inputs();
    std::optional<std::string> resolve_temp_tool(const ImGuiIO& io);
    void update_user_state_cursor();
    void handle_cursor();

    void render();

    std::string get_new_image_filename();
    void save_image_to_downloads();

    glm::vec2 get_mouse_pos_in_canvas_window();
    glm::vec2 get_mouse_pos_in_canvas();
    DebugState generate_debug_state();
};

