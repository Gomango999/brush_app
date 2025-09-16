#include <chrono>
#include <corecrt.h>
#include <cstdlib>
#include <ctime>
#include <format>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>

#include "imgui.h"
#include "imgui_impl_opengl3.h"

#include <glm/detail/type_vec3.hpp>
#include <glm/fwd.hpp>

#include "app.h"
#include "brush.h"
#include "canvas.h"
#include "frame_buffer.h"
#include "gui.h"
#include "layer.h"
#include "tools.h"
#include "user_state.h"

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
    m_gui(m_window.window(), glm::vec2( canvas_display_width, canvas_display_height )),
    m_canvas(canvas_width, canvas_height),
    m_tool_manager(),
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

        // We multiply by 0.9 since if we wait until after we hit the target_dt,
        // we miss the next "frame cycle", so we are actually rendering at 30fps.
        if (glfwGetTime() - m_last_update_time > m_target_display_dt * 0.9) {
            DebugState debug_state = generate_debug_state();

            m_gui.define_interface(
                m_user_state,
                m_canvas,
                m_tool_manager,
                debug_state
            );

            handle_cursor();

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

    // TODO: Make a wrapper that generates our own ImGuiIO, but
    // overwrites it with our own mouse events from m_window.
    const ImGuiIO& io = ImGui::GetIO();
    update_user_state_cursor();

    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
        save_image_to_downloads();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        m_window.set_should_close(true);
    }

    if (io.MouseWheel > 0) {
        m_canvas.zoom_into_point(m_user_state.cursor.pos, 1.1f);
    } else if (io.MouseWheel < 0) {
        m_canvas.zoom_into_point(m_user_state.cursor.pos, 1.0 / 1.1f);
    };

    if (ImGui::IsKeyPressed(ImGuiKey_D)) {
        m_tool_manager.select_tool_by_name("Pen");
    }
    if (ImGui::IsKeyPressed(ImGuiKey_E)) {
        m_tool_manager.select_tool_by_name("Eraser");
    }

    std::optional<Tool::Id> temp_tool = resolve_temp_tool(io);
    if (temp_tool.has_value()) m_tool_manager.temp_select_tool_by_id(temp_tool.value());
    else m_tool_manager.deselect_temp_tool();

    auto tool_opt = m_tool_manager.get_selected_tool();
    if (tool_opt.has_value()) {
        Tool& tool = tool_opt.value().get();
        
        if (Brush* brush = dynamic_cast<Brush*>(&tool)) {
            if (ImGui::IsKeyPressed(ImGuiKey_1)) brush->decrease_size();
            if (ImGui::IsKeyPressed(ImGuiKey_3)) brush->increase_size();
            if (ImGui::IsKeyPressed(ImGuiKey_2)) brush->decrease_opacity();
            if (ImGui::IsKeyPressed(ImGuiKey_4)) brush->increase_opacity();
        }
        
        bool prev_mouse_down = m_user_state.prev_cursor.has_value();
        bool mouse_pressed = !prev_mouse_down && m_window.is_mouse_down();
        bool mouse_released = prev_mouse_down && !m_window.is_mouse_down();

        if (mouse_pressed) tool.on_mouse_press(m_canvas, m_user_state);
        else if (mouse_released) tool.on_mouse_release(m_canvas, m_user_state);
        else if (m_window.is_mouse_down()) tool.on_mouse_down(m_canvas, m_user_state);
    }
    
    if (m_window.is_mouse_down()) {
        m_user_state.prev_cursor = m_user_state.cursor;
    } else {
        m_user_state.prev_cursor = std::nullopt;
    }
}

std::optional<Tool::Id> App::resolve_temp_tool(const ImGuiIO& io) {
    std::optional<std::string> temp_tool_name = std::nullopt;

    if (io.KeyCtrl && ImGui::IsKeyDown(ImGuiKey_Space)) temp_tool_name = "Zoom";
    else if (io.KeyAlt) temp_tool_name = "Color Picker";

    std::optional<Tool::Id> temp_tool_id =
        temp_tool_name.has_value() ?
        m_tool_manager.lookup_tool_by_name(temp_tool_name.value()) :
        std::nullopt;
    return temp_tool_id;
}

void App::update_user_state_cursor() {
    glm::vec2 cursor_pos = get_mouse_pos_in_canvas_window();
    float pressure = m_window.get_pressure();
    m_user_state.cursor = CursorState(cursor_pos, pressure);
}

void App::handle_cursor() {
    ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);

    // TODO: Move the cursor logic into the specific tools themselves. 

    //glm::vec2 mouse_pos = m_window.get_mouse_pos();
    //if (m_gui.is_hovering_canvas(mouse_pos)) {
    //    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    //}
}

std::string App::get_new_image_filename() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);

    std::tm local_time{};
#if defined(_WIN32)
    localtime_s(&local_time, &t);
#else
    localtime_r(&t, &localTime);
#endif

    std::ostringstream oss;
    oss << std::put_time(&local_time, "%Y%m%d_%H%M%S");
    std::string time_str = oss.str();

    const char* user_profile = std::getenv("USERPROFILE");

    std::string filename = std::format("{}/Downloads/brush_{}.png", user_profile, time_str);
    return filename;
}

void App::save_image_to_downloads() {
    std::string filename = get_new_image_filename();
    m_canvas.save_as_png(filename.c_str());
}

DebugState App::generate_debug_state() {
    glm::vec2 mouse_pos = get_mouse_pos_in_canvas_window();
    glm::vec2 canvas_pos = m_canvas.screen_space_to_world_space(mouse_pos);
    return DebugState{
        m_last_dt,
        mouse_pos,
        canvas_pos
    };
}

glm::vec2 App::get_mouse_pos_in_canvas_window() {
    glm::vec2 mouse_pos = m_window.get_mouse_pos();
    glm::vec2 screen_pos = m_gui.get_mouse_position_on_canvas_window(mouse_pos);
    return screen_pos;
}

glm::vec2 App::get_mouse_pos_in_canvas() {
    glm::vec2 screen_pos = get_mouse_pos_in_canvas_window();
    glm::vec2 canvas_pos = m_canvas.screen_space_to_world_space(screen_pos);
    return canvas_pos;
}

void App::render() {
    m_canvas.render(
        m_gui.canvas_window_size(), 
        m_user_state.cursor.pos
    );
    FrameBuffer::unbind();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window.window());
}


