#pragma once
#include <optional>
#include <string>

#include <imgui_impl_glfw.h>
#include <glm/fwd.hpp>

#include "canvas.h"
#include "layer.h"
#include "tools.h"
#include "user_state.h"

struct DebugState {
    double dt;
    glm::vec2 mouse_pos;
    glm::vec2 canvas_pos;
    bool is_flipped;
};

// GUI class responsible for defining the interface layout in Dear ImGui.
class GUI {
private:
    glm::vec2 m_canvas_display_size;

    glm::vec2 m_canvas_window_pos;
    glm::vec2 m_canvas_window_size;

    std::optional<std::string> m_alert_message;

public: 
    GUI(GLFWwindow* window, glm::vec2 canvas_size);
    ~GUI();

    void define_interface(
        UserState& user_state,
        Canvas& canvas,
        ToolManager& tool_manager,
        DebugState debug_state
    );

    void define_color_picker_window(glm::vec3& color);
    void define_tool_window(ToolManager& tool_manager);
    void define_tool_properties_window(ToolManager& tool_manager);
    void define_debug_window(DebugState& debug_state, UserState& user_state);
    void define_error_popup();
    void define_layer_window(Canvas& canvas, std::optional<Layer::Id>& selected_layer);
    void define_layer_buttons(Canvas& canvas, std::optional<Layer::Id>& selected_layer);
    void define_layer_list(Canvas& canvas, std::optional<Layer::Id>& selected_layer);
    void define_canvas_window(Canvas& canvas);


    bool is_hovering_canvas_window(glm::vec2 mouse_pos) const;
    glm::vec2 get_mouse_position_on_canvas_window(glm::vec2 mouse_pos) const;
    glm::vec2 canvas_window_pos() const { return m_canvas_window_pos; };
    glm::vec2 canvas_window_size() const { return m_canvas_window_size; };
};

