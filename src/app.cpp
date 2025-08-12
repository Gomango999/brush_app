#include <chrono>
#include <optional>
#include <thread>

#include "glad.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"

#include "app.h"
#include "canvas.h"
#include "gui.h"
#include "layer.h"

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
    m_prev_mouse_pos = std::nullopt;

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
        double time_to_wait = m_target_dt - m_last_dt;
        if (time_to_wait > 0.0) {
            std::this_thread::sleep_for(
                std::chrono::duration<double>(time_to_wait)
            );
        }
    }
}

void App::handle_inputs() {
    glfwPollEvents();

    ImGuiIO& io = ImGui::GetIO();

    ImVec2 mouse_pos = get_mouse_position_on_canvas();
    UserState& user_state = m_canvas.user_state();
    user_state.mouse_pos = mouse_pos;

    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        m_window.set_should_close(true);
    }

    if (ImGui::IsKeyPressed(ImGuiKey_1)) {
        user_state.decrease_brush_size();
    }

    if (ImGui::IsKeyPressed(ImGuiKey_3)) {
        user_state.increase_brush_size();
    }

    if (ImGui::IsMouseDown(0)) {
        if (!m_prev_mouse_pos.has_value()) {
            m_canvas.draw_circle_at_pos(mouse_pos);
        } else {
            m_canvas.draw_circles_on_segment(m_prev_mouse_pos.value(), mouse_pos, false, 8);
        }

        m_prev_mouse_pos = mouse_pos;
    } else {
        m_prev_mouse_pos = std::nullopt;
    }
}

ImVec2 App::get_mouse_position_on_canvas() {
    double mouse_x, mouse_y;
    m_window.get_cursor_pos(&mouse_x, &mouse_y);

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
    m_canvas.render_output_image();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window.window());
}

