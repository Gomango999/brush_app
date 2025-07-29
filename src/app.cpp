
#include "imgui.h"
#include "imgui_impl_opengl3.h"

#include "app.h"
#include "canvas.h"
#include "gui.h"

App::App(
    unsigned int screen_width,
    unsigned int screen_height,
    unsigned int canvas_width,
    unsigned int canvas_height
)
    : m_screen_width(screen_width),
      m_screen_height(screen_height),
      // SOMEDAY: To be removed when we implement zooming. For now we just
      // hard code the canvas's display size on the screen.
      m_canvas_display_width(800),
      m_canvas_display_height(800),
      m_window("Brush App", screen_width, screen_height),
      m_gui(m_window.window()),
      m_canvas(canvas_width, canvas_height)
{
    m_last_dt = 0.0;
    m_last_update_time = 0.0;

    Layer::Id new_layer_id = m_canvas.insert_new_layer_above_selected();
    m_canvas.user_state().selected_layer = new_layer_id;
}

void App::run() {
    while (!m_window.should_close()) {
        double loop_start_time = glfwGetTime();

        handle_inputs();

        DebugState debug_state = generate_debug_state();
        m_gui.define_interface(
            m_canvas,
            debug_state
        );

        render();

        m_last_dt = glfwGetTime() - loop_start_time;
    }
}

void App::handle_inputs() {
    glfwPollEvents();

    if (glfwGetKey(m_window.window(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window.window(), true);
    }

    if (glfwGetMouseButton(m_window.window(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        ImVec2 pos = get_mouse_position_on_canvas();
        m_canvas.draw_circle_at_pos(pos);
    }
}

ImVec2 App::get_mouse_position_on_canvas() {
    double mouse_x, mouse_y;
    // TODO: This function feels like it doesn't belong in app.cpp
    // Maybe in m_window.cpp?
    glfwGetCursorPos(m_window.window(), &mouse_x, &mouse_y);

    ImVec2 pos_on_canvas_window = 
        m_gui.get_mouse_position_on_canvas_window(mouse_x, mouse_y);

    // SOMEDAY: When zoom is added, replace this calculation with something more
    // sophisticated.
    ImVec2 pos_on_canvas = ImVec2(
        pos_on_canvas_window.x / m_canvas_display_width * m_canvas.width(),
        pos_on_canvas_window.y / m_canvas_display_height * m_canvas.width()
    );

    return pos_on_canvas; 
}

DebugState App::generate_debug_state() {
    ImVec2 mouse_pos = get_mouse_position_on_canvas();
    return DebugState {
        m_last_dt,
        mouse_pos
    };
}

void App::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (glfwGetTime() - m_last_update_time >= (1.0 / 60.0)) {
        glfwSwapBuffers(m_window.window());

        m_last_update_time = glfwGetTime();
    }
}




