#include <cstdarg>
#include <cstdio>
#include <format>
#include <ranges>
#include <string>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "canvas.h"
#include "gui.h"
#include "layer.h"

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

void GUI::define_interface(Canvas& canvas, DebugState debug_state) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    
    UserState& user_state = canvas.user_state();

    ImGui::Begin("Color");

    ImGui::ColorPicker4(
        "ColorPicker", 
        (float*)&user_state.color,
        ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Float
    );

    ImGui::End();

    ImGui::Begin("Brush");
    ImGui::SliderFloat("Size", &user_state.radius, 1.0f, 1000.0f, "%f");
    ImGui::SliderFloat("Opacity", &user_state.opacity, 0.0f, 1.0f);
    ImGui::End();

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    ImGui::Begin("Canvas", nullptr, window_flags);

    m_canvas_window_pos = ImGui::GetCursorScreenPos();

    ImGui::Image(
        (ImTextureID)canvas.output_texture(),
        ImVec2(800, 800)
    );
    ImGui::End();

    ImGui::Begin("Debug");
    imgui_formatted_label_text("dt", "%.9f", debug_state.dt);
    imgui_formatted_label_text("fps", "%.9f", 1.0 / debug_state.dt);
    imgui_formatted_label_text("mouse position", "(%d, %d)", int(debug_state.mouse_pos.x), int(debug_state.mouse_pos.y));
    imgui_formatted_label_text("selected layer", "%d", user_state.selected_layer.has_value() ? user_state.selected_layer.value() : -1);
    ImGui::End();



    ImGui::Begin("Layers");

    static bool show_alert = false;
    static std::string alert_message;
    if (show_alert) {
        ImGui::OpenPopup("Error");
    }

    if (ImGui::BeginPopupModal("Error", &show_alert)) {
        ImGui::Text("Error when creating new layer:");
        ImGui::Text(alert_message.c_str());

        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
            show_alert = false;
        }

        ImGui::EndPopup();
    }

    if (ImGui::Button("New")) {
        try {
            Layer::Id new_layer_id = canvas.insert_new_layer_above_selected();
            user_state.selected_layer = new_layer_id;
        } catch (const std::runtime_error& e) {
            show_alert = true;
            alert_message = e.what();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete")) {
        canvas.delete_selected_layer();
    }

    ImGui::BeginChild("LayerList", ImVec2(0, 200), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    for (auto& layer : std::views::reverse(canvas.get_layers())) {

        bool visible = layer.is_visible();
        std::string checkbox_label = std::format("##layer_visible_checkbox{}", layer.id());
        if (ImGui::Checkbox(checkbox_label.c_str(), &visible)) {
            canvas.set_layer_visibility(layer.id(), visible);
        }

        ImGui::SameLine();

        bool is_selected = user_state.selected_layer.has_value() ?
            layer.id() == user_state.selected_layer.value() :
            false;
        if (ImGui::Selectable(layer.name().c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
            user_state.selected_layer = layer.id();
        }

    }
    ImGui::EndChild();
    ImGui::End();
}

ImVec2 GUI::get_mouse_position_on_canvas_window(double mouse_x, double mouse_y) {
    return ImVec2(
        mouse_x - m_canvas_window_pos.x,
        mouse_y - m_canvas_window_pos.y
    );
}
