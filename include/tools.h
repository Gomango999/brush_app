#pragma once
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "canvas.h"
#include "user_state.h"

class Tool {
public:
    typedef unsigned long long Id;

private:
    Id m_id;
    std::string m_name;

protected:
    Tool() = default; // TODO: Assign our own ID here.
    ~Tool() = default;

public:
    // TODO: Separate out user_state into mouse_state and canvas_state. These functions
    // should not be able to modify the mouse positions.
    virtual void on_mouse_press(Canvas& canvas, UserState& user_state) {}
    virtual void on_mouse_down(Canvas& canvas, UserState& user_state) {}
    virtual void on_mouse_release(Canvas& canvas, UserState& user_state) {}
    virtual void set_mouse_cursor() {}

    Id id() const { return m_id; }
    std::string name() const { return m_name; }
};

class ToolManager {
    std::vector<std::unique_ptr<Tool>> m_tools;
    std::optional<Tool::Id> m_selected_tool;
    std::optional<Tool::Id> m_prev_tool;

public:
    ToolManager();

    ToolManager(const ToolManager&) = delete;
    ToolManager& operator=(const ToolManager&) = delete;

    std::optional<std::reference_wrapper<Tool>> get_selected_tool();
    void select_tool_by_id(Tool::Id tool_id);
    void select_tool_by_name(std::string name);
    void select_previous_tool();

    // TODO: Do we need more const in here?
    const std::vector<std::unique_ptr<Tool>>& tools() const;

private:
    const std::optional<std::reference_wrapper<Tool>> get_tool_by_id(Tool::Id tool_id);
    const std::optional<std::reference_wrapper<Tool>> get_tool_by_name(std::string name);

    template<typename Predicate>
    const std::optional<std::reference_wrapper<Tool>> find_tool(Predicate&& pred);
};