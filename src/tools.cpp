#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "tools.h"
#include "brush.h"

Tool::Tool() {
    static Id next_id = 0;
    m_id = next_id;
    next_id++;

    m_name = "Unnamed Tool";
}

ToolManager::ToolManager() {
    m_tools.push_back(std::make_unique<Pen>());
    m_tools.push_back(std::make_unique<Eraser>());

    m_selected_tool = !m_tools.empty() ? std::optional{ m_tools[0]->id() } : std::nullopt;
}

std::optional<std::reference_wrapper<Tool>> ToolManager::get_selected_tool() {
    if (!m_selected_tool) return std::nullopt;
    return get_tool_by_id(m_selected_tool.value());
}

void ToolManager::select_tool_by_id(Tool::Id tool_id) {
    if (get_tool_by_id(tool_id).has_value()) {
        m_prev_tool = m_selected_tool;
        m_selected_tool = tool_id;
    }
}

void ToolManager::select_tool_by_name(std::string name) {
    auto tool = get_tool_by_name(name);
    if (tool.has_value()) {
        m_prev_tool = m_selected_tool;
        m_selected_tool = tool.value().get().id();
    }
}

void ToolManager::select_previous_tool() {
    std::swap(m_prev_tool, m_selected_tool);
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


