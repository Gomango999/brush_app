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

#include "brush.h"
#include "canvas.h"
#include "gui.h"
#include "layer.h"
#include "user_state.h"
#include "vec.h"

GUI::GUI(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(window, true);     
    ImGui_ImplOpenGL3_Init("#version 430 core");
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
    DebugState debug_state, 
    size_t canvas_display_width, 
    size_t canvas_display_height
) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

    define_color_picker_window(user_state.selected_color);
    define_brush_window(user_state.brush_manager);
    define_brush_properties_window(user_state.brush_manager);
    define_canvas_window(canvas, canvas_display_width, canvas_display_height);
    define_debug_window(debug_state, user_state);
    define_error_popup();
    define_layer_window(canvas, user_state.selected_layer);
}

void GUI::define_color_picker_window(Vec3& color) {
    ImGui::Begin("Color");
    ImGui::ColorPicker3(
        "ColorPicker",
        (float*)&color,
        ImGuiColorEditFlags_NoAlpha    
        | ImGuiColorEditFlags_DisplayHSV
    );
    ImGui::End();
}

void GUI::define_brush_window(BrushManager& brush_manager) {
    ImGui::Begin("Brush");

    ImGui::BeginChild("BrushList", ImVec2(0, 200), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    for (const std::unique_ptr<Brush>& brush : brush_manager.brushes()) {
        auto selected_brush_opt = brush_manager.get_selected_brush();
        bool is_selected = selected_brush_opt.has_value() ?
            brush->id() == selected_brush_opt.value().get().id() :
            false;
        if (ImGui::Selectable(brush->name().c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
            brush_manager.set_selected_brush(brush->id());
        }
    }
    ImGui::EndChild();

    ImGui::End();
}

void GUI::define_brush_properties_window(BrushManager& brush_manager) {
    ImGui::Begin("Properties");
    auto brush_opt = brush_manager.get_selected_brush();
    if (brush_opt.has_value()) {
        Brush& brush = brush_opt.value().get();

        ImGui::SliderFloat("Size", &brush.size(), 1.0f, 1000.0f, "%f");
        ImGui::SliderFloat("Opacity", &brush.opacity(), 0.0f, 1.0f);
    }
    ImGui::End();
}


void GUI::define_canvas_window(Canvas& canvas, size_t canvas_display_width, size_t canvas_display_height) {
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    ImGui::Begin("Canvas", nullptr, window_flags);

    m_canvas_window_pos = Vec<2>::from_ImVec2(ImGui::GetCursorScreenPos());

    ImGui::Image(
        (ImTextureID)canvas.output_texture(),
        ImVec2(canvas_display_width, canvas_display_height)
    );
    ImGui::End();
}

void GUI::define_debug_window(DebugState& debug_state, UserState& user_state) {
    ImGui::Begin("Debug");
    imgui_formatted_label_text("dt", "%.9f", debug_state.dt);
    imgui_formatted_label_text("fps", "%.9f", 1.0 / debug_state.dt);
    imgui_formatted_label_text("mouse position", "(%d, %d)", int(debug_state.mouse_pos.x()), int(debug_state.mouse_pos.y()));
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
    if (ImGui::Button("Delete")) {
        selected_layer = canvas.delete_selected_layer(selected_layer);
    }
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

Vec2 GUI::get_mouse_position_on_canvas_window(Vec2 mouse_pos) const {
    return mouse_pos - m_canvas_window_pos;
}
