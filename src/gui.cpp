#include <cstdarg>
#include <cstdio>
#include <format>
#include <memory>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glm/glm.hpp"

#include "brush.h"
#include "canvas.h"
#include "conversions.h"
#include "gui.h"
#include "layer.h"
#include "user_state.h"

GUI::GUI(GLFWwindow* window, glm::vec2 canvas_size) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(window, true);     
    ImGui_ImplOpenGL3_Init("#version 430 core");

    m_canvas_display_size = canvas_size;
    m_canvas_window_pos = glm::vec2(0, 0);
    m_canvas_window_size = glm::vec2(0, 0);
}

GUI::~GUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

// Helper function to write formatted text to a buffer first before displaying.
// This is useful since sometimes, ImGui automatically truncates values before
// displaying them.
static void imgui_formatted_label_text(const char* label, const char* fmt, ...) {
    char buffer[64];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    ImGui::LabelText(label, "%s", buffer);
}

void GUI::define_interface(
    UserState& user_state,
    Canvas& canvas,
    ToolManager& tool_manager,
    DebugState debug_state
) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

    define_color_picker_window(user_state.selected_color);
    define_tool_window(tool_manager);
    define_tool_properties_window(tool_manager);
    define_canvas_window(canvas);
    define_debug_window(debug_state, user_state);
    define_error_popup();
    define_layer_window(canvas, user_state.selected_layer);
}

void GUI::define_color_picker_window(glm::vec3& color) {
    ImGui::Begin("Color");
    ImGui::ColorPicker3(
        "ColorPicker",
        (float*)&color,
        ImGuiColorEditFlags_NoAlpha    
        | ImGuiColorEditFlags_DisplayHSV
    );
    ImGui::End();
}

void GUI::define_tool_window(ToolManager& tool_manager) {
    ImGui::Begin("Tools");

    ImGui::BeginChild("ToolList", ImVec2(0, 200), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    for (auto& tool : tool_manager.tools()) {
        auto selected_tool_opt = tool_manager.get_selected_tool();
        bool is_selected = selected_tool_opt.has_value() ?
            tool->id() == selected_tool_opt.value().get().id() :
            false;
        if (ImGui::Selectable(tool->name().c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
            tool_manager.select_tool_by_id(tool->id());
        }
    }
    ImGui::EndChild();

    ImGui::End();
}

void GUI::define_tool_properties_window(ToolManager& tool_manager) {
    ImGui::Begin("Properties");
    // TODO: In future, every tool should define it's own GUI. For
    // now we only add hardcoded functionality for brushes.
    auto selected_tool_opt = tool_manager.get_selected_tool();
    if (selected_tool_opt.has_value()) {
        Tool& selected_tool = selected_tool_opt.value().get();
        
        if (Brush* brush = dynamic_cast<Brush*>(&selected_tool)) {
           ImGui::SliderFloat("Size", &brush->size(), 1.0f, 1000.0f, "%f");
            ImGui::SliderFloat("Opacity", &brush->opacity(), 0.0f, 1.0f);
        }
    }
    ImGui::End();
}


void GUI::define_canvas_window(Canvas& canvas) {
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    ImGui::Begin("Canvas", nullptr, window_flags);

    m_canvas_window_pos = to_glm(ImGui::GetCursorScreenPos());
    m_canvas_window_size = to_glm(ImGui::GetContentRegionAvail());

    ImGui::Image(
        (ImTextureID)canvas.screen_texture().id(),
        //(ImTextureID)canvas.output_texture().id(),
        to_imvec(m_canvas_window_size),
        ImVec2(0, 1), ImVec2(1, 0) // Flips image vertically, to match OpenGL convention
    );
    ImGui::End();
}

void GUI::define_debug_window(DebugState& debug_state, UserState& user_state) {
    ImGui::Begin("Debug");
    imgui_formatted_label_text("dt", "%.9f", debug_state.dt);
    imgui_formatted_label_text("fps", "%.9f", 1.0 / debug_state.dt);
    imgui_formatted_label_text("mouse position", "(%d, %d)", int(debug_state.mouse_pos.x), int(debug_state.mouse_pos.y));
    imgui_formatted_label_text("canvas position", "(%d, %d)", int(debug_state.canvas_pos.x), int(debug_state.canvas_pos.y));
    imgui_formatted_label_text("selected layer", "%d", user_state.selected_layer.has_value() ? user_state.selected_layer.value() : -1);
    ImGui::End();
}

void GUI::define_error_popup() {
    if (m_alert_message.has_value()) {
        ImGui::OpenPopup("Error");
    }

    if (ImGui::BeginPopupModal("Error", nullptr)) {
        ImGui::Text("Error when creating new layer:");
        if (m_alert_message.has_value()) {
            ImGui::Text(m_alert_message.value().c_str());
        } else {
            ImGui::Text("Reason Unknown");
        }

        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
            m_alert_message = std::nullopt;
        }

        ImGui::EndPopup();
    } else {
        m_alert_message = std::nullopt;
    }
}

void GUI::define_layer_window(Canvas& canvas, std::optional<Layer::Id>& selected_layer)
{
    ImGui::Begin("Layers");
    define_layer_buttons(canvas, selected_layer);
    define_layer_list(canvas, selected_layer);
    ImGui::End();
}

void GUI::define_layer_buttons(Canvas& canvas, std::optional<Layer::Id>& selected_layer) {
    if (ImGui::Button("New")) {
        try {
            Layer::Id new_layer_id = canvas.insert_new_layer_above_selected(selected_layer);
            selected_layer = new_layer_id;
        }
        catch (const std::runtime_error& e) {
            m_alert_message = e.what();
        }
    }
    ImGui::SameLine();

    ImGui::BeginDisabled(!selected_layer.has_value());

    if (ImGui::Button("Delete")) {
        selected_layer = canvas.delete_selected_layer(selected_layer);
    }
    ImGui::SameLine();
    if (ImGui::Button("^")) {
        canvas.move_layer_up(selected_layer);
    }
    ImGui::SameLine();
    if (ImGui::Button("v")) {
        canvas.move_layer_down(selected_layer);
    }
    ImGui::SameLine();
    bool alpha_locked = selected_layer.has_value() ?
        canvas.get_layer_alpha_lock(selected_layer.value()) :
        false;

    if (ImGui::Checkbox("##layer_alpha_locked_checkbox", &alpha_locked)) {
        if (selected_layer.has_value()) {
            canvas.set_layer_alpha_lock(selected_layer.value(), alpha_locked);
        }
    }

    ImGui::EndDisabled();
}

void GUI::define_layer_list(Canvas& canvas, std::optional<Layer::Id>& selected_layer) {
    ImGui::BeginChild("LayerList", ImVec2(0, 200), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    for (auto& layer : std::views::reverse(canvas.get_layers())) {

        bool visible = layer.is_visible();
        std::string checkbox_label = std::format("##layer_visible_checkbox{}", layer.id());
        if (ImGui::Checkbox(checkbox_label.c_str(), &visible)) {
            canvas.set_layer_visibility(layer.id(), visible);
        }

        ImGui::SameLine();

        bool is_selected = selected_layer.has_value() ?
            layer.id() == selected_layer.value() :
            false;
        if (ImGui::Selectable(layer.name().c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
            selected_layer = layer.id();
        }

    }
    ImGui::EndChild();
}

bool GUI::is_hovering_canvas(glm::vec2 mouse_pos) const {
    glm::vec2 top_left = m_canvas_window_pos;
    glm::vec2 bottom_right = m_canvas_window_pos + m_canvas_display_size;
    return mouse_pos.x >= top_left.x && mouse_pos.x < bottom_right.x
        && mouse_pos.y >= top_left.y && mouse_pos.y < bottom_right.y;
}

bool GUI::is_hovering_canvas_window(glm::vec2 mouse_pos) const {
    glm::vec2 top_left = m_canvas_window_pos;
    glm::vec2 bottom_right = m_canvas_window_pos + m_canvas_window_size;
    return mouse_pos.x >= top_left.x && mouse_pos.x < bottom_right.x
        && mouse_pos.y >= top_left.y && mouse_pos.y < bottom_right.y;
}

glm::vec2 GUI::get_mouse_position_on_canvas_window(glm::vec2 mouse_pos) const {
    return mouse_pos - m_canvas_window_pos;
}
