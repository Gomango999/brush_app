#include <cstdarg>
#include <cstdio>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "gui.h"

GUI::GUI(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(window, true);     
    ImGui_ImplOpenGL3_Init();
}

GUI::~GUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

// Helper function to write formatted text to a buffer first before displaying.
// This is useful since sometimes, ImGui automatically truncates values before
// displaying them.
void imgui_formatted_label_text(const char* label, const char* fmt, ...) {
    char buffer[64];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    ImGui::LabelText(label, "%s", buffer);
}

void GUI::define_interface(CanvasState canvas_state, GLuint canvas_texture, DebugState debug_state) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    
    // BUG: Program crashes when this is pressed.
    ImGui::Begin("Color");
    ImGuiColorEditFlags base_flags = ImGuiColorEditFlags_None;
    ImGui::ColorEdit4("Color", (float*) &canvas_state.color, base_flags);
    ImGui::End();

    ImGui::Begin("Brush");
    ImGui::SliderFloat("Size", &canvas_state.radius, 1.0f, 1000.0f, "%f");
    ImGui::SliderFloat("Opacity", &canvas_state.opacity, 0.0f, 1.0f);
    ImGui::End();


    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    ImGui::Begin("Canvas", nullptr, window_flags);

    m_canvas_window_pos = ImGui::GetCursorScreenPos();

    // TODO: When panning/zoom/rotation is added, have a smarter algorithm 
    // for working out how to position the canvas on screen. 
    ImGui::Image(
        (ImTextureID) canvas_texture, 
        ImVec2(800, 800)
    );
    ImGui::End();

    ImGui::Begin("Debug");
    imgui_formatted_label_text("dt", "%.9f", debug_state.dt);
    imgui_formatted_label_text("fps", "%.9f", 1.0/debug_state.dt);
    imgui_formatted_label_text("mouse position", "(%d, %d)", int(debug_state.mouse_pos.x), int(debug_state.mouse_pos.y));
    ImGui::End();


    ImGui::Begin("Layers");
    ImGui::Text("Unimplemented");
    ImGui::End();
}

ImVec2 GUI::get_mouse_position_on_canvas_window(double mouse_x, double mouse_y) {
    return ImVec2(
        mouse_x - m_canvas_window_pos.x,
        mouse_y - m_canvas_window_pos.y
    );
}
