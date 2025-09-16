#pragma once
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "user_state.h"

class Canvas;

class Tool {
public:
    typedef unsigned long long Id;

protected:
    Id m_id;
    std::string m_name;

    Tool(); 

public:
    // TODO: Separate out user_state into mouse_state and canvas_state. These functions
    // should not be able to modify the mouse positions.
    virtual void on_mouse_press(Canvas& canvas, UserState& user_state) {}
    virtual void on_mouse_down(Canvas& canvas, UserState& user_state) {}
    virtual void on_mouse_release(Canvas& canvas, UserState& user_state) {}
    virtual void set_mouse_cursor() {}

    // TODO: Tools should be responsible for creating their on ImGui UI settings.

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
    std::optional<Tool::Id> lookup_tool_by_name(std::string name);

    void temp_select_tool_by_id(Tool::Id tool_id);
    void temp_select_tool_by_name(std::string name);
    void deselect_temp_tool();

    const std::vector<std::unique_ptr<Tool>>& tools() const;

private:
    const std::optional<std::reference_wrapper<Tool>> get_tool_by_id(Tool::Id tool_id);
    const std::optional<std::reference_wrapper<Tool>> get_tool_by_name(std::string name);

    template<typename Predicate>
    const std::optional<std::reference_wrapper<Tool>> find_tool(Predicate&& pred);
};

class ColorPicker final : public Tool {
public:
    ColorPicker();
    void on_mouse_down(Canvas& canvas, UserState& user_state) override;
};

class Zoom : public Tool {
    float m_zoom_sensitivity = 1.1f;
public:
    Zoom();
    void on_mouse_down(Canvas& canvas, UserState& user_state) override;
};