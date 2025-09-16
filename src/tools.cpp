#include <cmath>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <glm/fwd.hpp>

#include "brush.h"
#include "canvas.h"
#include "tools.h"
#include "user_state.h"

Tool::Tool() {
    static Id next_id = 0;
    m_id = next_id;
    next_id++;

    m_name = "Unnamed Tool";
}

ToolManager::ToolManager() {
    m_tools.push_back(std::make_unique<Pen>());
    m_tools.push_back(std::make_unique<Eraser>());
    m_tools.push_back(std::make_unique<ColorPicker>());
    m_tools.push_back(std::make_unique<Zoom>());
    m_tools.push_back(std::make_unique<Pan>());
    m_tools.push_back(std::make_unique<Rotate>());

    m_selected_tool = !m_tools.empty() ? std::optional{ m_tools[0]->id() } : std::nullopt;
}

std::optional<std::reference_wrapper<Tool>> ToolManager::get_selected_tool() {
    if (!m_selected_tool) return std::nullopt;
    return get_tool_by_id(m_selected_tool.value());
}

void ToolManager::select_tool_by_id(Tool::Id tool_id) {
    // Cannot change tools while temp selecting another tool
    if (m_prev_tool.has_value()) return;

    if (get_tool_by_id(tool_id).has_value()) {
        m_selected_tool = tool_id;
    }
}

void ToolManager::select_tool_by_name(std::string name) {
    // Cannot change tools while temp selecting another tool
    if (m_prev_tool.has_value()) return;

    auto tool = get_tool_by_name(name);
    if (tool.has_value()) {
        m_selected_tool = tool.value().get().id();
    }
}

std::optional<Tool::Id> ToolManager::lookup_tool_by_name(std::string name) {
    auto tool = get_tool_by_name(name);
    if (tool.has_value()) return tool.value().get().id();
    else return std::nullopt;
}

void ToolManager::temp_select_tool_by_id(Tool::Id tool_id) {
    if (!m_prev_tool.has_value()) m_prev_tool = m_selected_tool;

    if (get_tool_by_id(tool_id).has_value()) {
        m_selected_tool = tool_id;
    }
}

void ToolManager::temp_select_tool_by_name(std::string name) {
    if (!m_prev_tool.has_value()) m_prev_tool = m_selected_tool;

    auto tool = get_tool_by_name(name);
    if (tool.has_value()) {
        m_selected_tool = tool.value().get().id();
    }
}

void ToolManager::deselect_temp_tool() {
    if (!m_prev_tool.has_value()) return;
    m_selected_tool = m_prev_tool.value();
    m_prev_tool = std::nullopt;
}

const std::optional<std::reference_wrapper<Tool>> ToolManager::get_tool_by_id(Tool::Id tool_id) {
    return find_tool([tool_id](const auto& tool) {
        return tool->id() == tool_id;
    });
}

const std::optional<std::reference_wrapper<Tool>> ToolManager::get_tool_by_name(std::string name) {
    return find_tool([&name](const auto& tool) {
        return tool->name() == name;
    });
}

template <typename Predicate>
const std::optional<std::reference_wrapper<Tool>> ToolManager::find_tool(Predicate&& pred) {
    auto it = std::find_if(m_tools.begin(), m_tools.end(),
        std::forward<Predicate>(pred));

    return (it != m_tools.end())
        ? std::optional{ std::ref(**it) }
    : std::nullopt;
}

const std::vector<std::unique_ptr<Tool>>& ToolManager::tools() const {
    return m_tools;
}



ColorPicker::ColorPicker() {
    m_name = "Color Picker";
}

void ColorPicker::on_mouse_down(Canvas& canvas, UserState& user_state) {
    glm::vec2 cursor_pos = canvas.screen_space_to_canvas_space(user_state.cursor.pos);
    std::optional<glm::vec3> color_opt = canvas.get_color_at_pos(cursor_pos);
    if (color_opt.has_value()) {
        user_state.selected_color = color_opt.value();
    }
}

Zoom::Zoom() {
    m_name = "Zoom";
}

void Zoom::on_mouse_down(Canvas& canvas, UserState& user_state) {
    if (!user_state.prev_cursor.has_value()) return;
    float distance = user_state.cursor.pos.x - user_state.prev_cursor.value().pos.x;
    float zoom_factor = pow(m_zoom_sensitivity, distance / 25);
    canvas.zoom_into_center(zoom_factor);
}

Pan::Pan() {
    m_name = "Pan";
}

void Pan::on_mouse_down(Canvas& canvas, UserState& user_state) {
    if (!user_state.prev_cursor.has_value()) return;
    glm::vec2 offset = user_state.cursor.pos - user_state.prev_cursor.value().pos;
    canvas.move(offset);
}

Rotate::Rotate() {
    m_name = "Rotate";
}

static float angle_between(const glm::vec2& u, const glm::vec2& v) {
    return std::atan2(u.x * v.y - u.y * v.x, glm::dot(u, v));
}

void Rotate::on_mouse_down(Canvas& canvas, UserState& user_state) {
    if (!user_state.prev_cursor.has_value()) return;
    
    glm::vec2 u = user_state.prev_cursor.value().pos - (canvas.window_size() / 2.0f);
    glm::vec2 v = user_state.cursor.pos - (canvas.window_size() / 2.0f);
    float angle = angle_between(u, v);

    canvas.rotate(angle);
}


// The basic template for creating a new tool:
//class Tool {
//    std::string m_name;
//    Tool();
//    virtual void on_mouse_press(Canvas& canvas, UserState& user_state) {}
//    virtual void on_mouse_down(Canvas& canvas, UserState& user_state) {}
//    virtual void on_mouse_release(Canvas& canvas, UserState& user_state) {}
//    virtual void set_mouse_cursor() {}
//};

