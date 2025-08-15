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
    unsigned int canvas_height,
    unsigned int canvas_display_width,
    unsigned int canvas_display_height
)
    : m_screen_width(screen_width),
    m_screen_height(screen_height),
    // SOMEDAY: To be removed when we implement zooming. For now we just
    // hard code the canvas's display size on the screen.
    m_window("Brush App", screen_width, screen_height),
    m_gui(m_window.window(), Vec2{ float(canvas_display_width), float(canvas_display_height) }),
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
            debug_state
        );

        handle_cursor();

        // TODO: Think about why multiplying by 0.9 makes it feel smoother
        if (glfwGetTime() - m_last_update_time > m_target_display_dt * 0.9) {
            render();
            m_last_update_time = glfwGetTime();
        }

        // Calling `std::this_thread::sleep_for()` has a minimum sleep time
        // of 16ms. To achieve a higher internal framerate, we hotloop 
        // instead.
        while (glfwGetTime() - loop_start_time < m_target_internal_dt) {}
        m_last_dt = glfwGetTime() - loop_start_time;
    }
}

void App::handle_inputs() {
    glfwPollEvents();

    ImGuiIO& io = ImGui::GetIO();

    Vec2 cursor_pos = get_mouse_pos_on_canvas();
    float pressure = m_window.get_pressure();    
    m_user_state.cursor = CursorState(cursor_pos, pressure);

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

    if (ImGui::IsKeyPressed(ImGuiKey_2)) {
        auto brush = m_user_state.brush_manager.get_selected_brush();
        if (brush.has_value()) {
            brush.value().get().decrease_opacity();
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_4)) {
        auto brush = m_user_state.brush_manager.get_selected_brush();
        if (brush.has_value()) {
            brush.value().get().increase_opacity();
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_D)) {
        m_user_state.brush_manager.set_selected_brush_by_name("Pen");
    }

    if (ImGui::IsKeyPressed(ImGuiKey_E)) {
        m_user_state.brush_manager.set_selected_brush_by_name("Eraser");
    }

    bool is_drawing = false;
    m_user_state.is_color_picking = io.KeyAlt;

    if (m_window.is_mouse_down()) {
        if (io.KeyAlt) {
            std::optional<Vec3> color_opt = m_canvas.get_color_at_pos(m_user_state.cursor.pos);
            if (color_opt.has_value()) {
                m_user_state.selected_color = color_opt.value();
            }
        } else {
            apply_brush_stroke(m_user_state);
            is_drawing = true;
        }
    }
    
    if (is_drawing) {
        m_user_state.prev_cursor = m_user_state.cursor;
    } else {
        m_user_state.prev_cursor = std::nullopt;
    }
}

void App::handle_cursor() {
    ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);

    Vec2 mouse_pos = m_window.get_mouse_pos();
    if (m_gui.is_hovering_canvas(mouse_pos)) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_None);

        if (m_user_state.is_color_picking) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
        }
    }
}

void App::apply_brush_stroke(UserState& user_state) {
    if (!user_state.selected_layer.has_value()) return;

    Layer::Id layer_id = user_state.selected_layer.value();
    auto layer_opt = m_canvas.lookup_layer(layer_id);
    if (!layer_opt.has_value()) return;
    Layer& layer = layer_opt.value().get();

    auto brush_opt = user_state.brush_manager.get_selected_brush();
    if (!brush_opt.has_value()) return;
    Brush& brush = brush_opt.value().get();

    if (!user_state.prev_cursor.has_value()) {
        m_canvas.draw_circle_at_pos(layer, brush, user_state.cursor, user_state.selected_color);
    }
    else {
        m_canvas.draw_circles_on_segment(layer, brush, m_user_state.cursor, m_user_state.prev_cursor.value(), m_user_state.selected_color);
    }
}

DebugState App::generate_debug_state() {
    Vec2 mouse_pos = get_mouse_pos_on_canvas();
    return DebugState{
        m_last_dt,
        mouse_pos
    };
}

Vec2 App::get_mouse_pos_on_canvas() {
    Vec2 mouse_pos = m_window.get_mouse_pos();

    Vec2 normalised_canvas_pos = m_gui.get_normalised_mouse_pos_on_canvas(mouse_pos);
    Vec2 canvas_pos = Vec2{
        normalised_canvas_pos.x() * m_canvas.width(),
        normalised_canvas_pos.y() * m_canvas.height()
    };
    return canvas_pos;
}

void App::render() {
    m_canvas.render(m_user_state.brush_manager, m_user_state.cursor.pos);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window.window());
}

