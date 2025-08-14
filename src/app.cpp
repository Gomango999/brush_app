#include <chrono>
#include <optional>
#include <thread>

#include "glad/glad.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"

#include "app.h"
#include "brush.h"
#include "canvas.h"
#include "gui.h"
#include "layer.h"
#include "user_state.h"
#include "vec.h"

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
    m_canvas_display_width(1200),
    m_canvas_display_height(1200),
    m_window("Brush App", screen_width, screen_height),
    m_gui(m_window.window()),
    m_canvas(canvas_width, canvas_height),
    m_user_state()
{
    m_last_dt = 0.0;
    m_last_update_time = 0.0;

    Layer::Id new_layer_id = m_canvas.insert_new_layer_above_selected(m_user_state.selected_layer);
    m_user_state.selected_layer = new_layer_id;
}

void App::run() {
    while (!m_window.should_close()) {
        double loop_start_time = glfwGetTime();

        handle_inputs();

        DebugState debug_state = generate_debug_state();
        m_gui.define_interface(
            m_user_state,
            m_canvas,
            debug_state,
            m_canvas_display_width, m_canvas_display_height
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

    Vec2 mouse_pos = get_mouse_position_on_canvas();
    m_user_state.mouse_pos = mouse_pos;

    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        m_window.set_should_close(true);
    }

    if (ImGui::IsKeyPressed(ImGuiKey_1)) {
        auto brush = m_user_state.brush_manager.get_selected_brush();
        if (brush.has_value()) {
            brush.value().get().decrease_size();
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_3)) {
        auto brush = m_user_state.brush_manager.get_selected_brush();
        if (brush.has_value()) {
            brush.value().get().increase_size();
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_D)) {
        m_user_state.brush_manager.set_selected_brush_by_name("Pen");
    }

    if (ImGui::IsKeyPressed(ImGuiKey_E)) {
        m_user_state.brush_manager.set_selected_brush_by_name("Eraser");
    }

    if (ImGui::IsMouseDown(0)) {
        apply_brush_stroke();

        m_user_state.prev_mouse_pressed_pos = mouse_pos;
    }
    else {
        m_user_state.prev_mouse_pressed_pos = std::nullopt;
    }
}

void App::apply_brush_stroke() {
    if (!m_user_state.selected_layer.has_value()) return;

    Layer::Id layer_id = m_user_state.selected_layer.value();
    auto layer_opt = m_canvas.lookup_layer(layer_id);
    if (!layer_opt.has_value()) return;
    Layer& layer = layer_opt.value().get();

    auto brush_opt = m_user_state.brush_manager.get_selected_brush();
    if (!brush_opt.has_value()) return;
    Brush& brush = brush_opt.value().get();

    if (!m_user_state.prev_mouse_pressed_pos.has_value()) {
        m_canvas.draw_circle_at_pos(layer, brush, m_user_state.mouse_pos, m_user_state.selected_color);
    }
    else {
        Vec2 prev_mouse_pos = m_user_state.prev_mouse_pressed_pos.value();
        m_canvas.draw_circles_on_segment(layer, brush, m_user_state.mouse_pos, prev_mouse_pos, m_user_state.selected_color);
    }
}

Vec2 App::get_mouse_position_on_canvas() {
    Vec2 mouse_pos = m_window.get_cursor_pos();

    Vec2 pos_on_canvas_window =
        m_gui.get_mouse_position_on_canvas_window(mouse_pos);

    // SOMEDAY: When zoom is added, replace this calculation with something more
    // sophisticated.
    Vec2 pos_on_canvas{
        pos_on_canvas_window.x() / m_canvas_display_width * m_canvas.width(),
        pos_on_canvas_window.y() / m_canvas_display_height * m_canvas.width()
    };

    return pos_on_canvas;
}

DebugState App::generate_debug_state() {
    Vec2 mouse_pos = get_mouse_position_on_canvas();
    return DebugState{
        m_last_dt,
        mouse_pos
    };
}

void App::render() {
    m_canvas.render(m_user_state.brush_manager, m_user_state.mouse_pos);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window.window());
}

