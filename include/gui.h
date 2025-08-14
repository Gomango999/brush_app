#pragma once
#include <optional>
#include <string>

#include "imgui_impl_glfw.h"

#include "canvas.h"
#include "layer.h"
#include "user_state.h"
#include "vec.h"

struct DebugState {
    double dt;
    Vec2 mouse_pos;
};

// GUI class responsible for defining the interface layout in Dear ImGui.
class GUI {
private:
    Vec2 m_canvas_window_pos;

    std::optional<std::string> m_alert_message;

public: 
    GUI(GLFWwindow* window);
    ~GUI();

    void define_interface(
        UserState& user_state,
        Canvas& canvas, 
        DebugState debug_state, 
        size_t canvas_display_width, 
        size_t canvas_display_height
    );


    void define_color_picker_window(Vec3& color);
    void define_brush_window(BrushManager& brush_manager);
    void define_brush_properties_window(BrushManager& brush_manager);
    void define_debug_window(DebugState& debug_state, UserState& user_state);
    void define_error_popup();
    void define_layer_window(Canvas& canvas, std::optional<Layer::Id>& selected_layer);
    void define_layer_buttons(Canvas& canvas, std::optional<Layer::Id>& selected_layer);
    void define_layer_list(Canvas& canvas, std::optional<Layer::Id>& selected_layer);
    void define_canvas_window(Canvas& canvas, size_t canvas_display_width, size_t canvas_display_height);


    Vec2 get_mouse_position_on_canvas_window(Vec2 mouse_pos) const;
};

