#pragma once
#include "canvas.h"
#include "gui.h"
#include "user_state.h"
#include "vec.h"
#include "window.h"

class App {
public:
    App(
        unsigned int screen_width,
        unsigned int screen_height,
        unsigned int canvas_width,
        unsigned int canvas_height
    );

    void run();


private:
    unsigned int m_screen_width;
    unsigned int m_screen_height;
    unsigned int m_canvas_display_width;
    unsigned int m_canvas_display_height;

    Window m_window;
    GUI m_gui;
    Canvas m_canvas;
    UserState m_user_state;

    double m_last_update_time;
    double m_last_dt;
    const double m_target_fps = 60.0;
    const double m_target_dt = 1.0 / m_target_fps;

    void handle_inputs();
    void apply_brush_stroke();
    Vec2 get_mouse_position_on_canvas();
    DebugState generate_debug_state();
    void render();
};

