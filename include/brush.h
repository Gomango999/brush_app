#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "glad/glad.h"
#include "imgui.h"

#include "program.h"

class Brush {
public:
    typedef unsigned long long Id;

protected:

    Id m_id;
    std::string m_name;
    float m_size;
    float m_opacity;

    Program m_brush_program;

    Brush();

    virtual void init_program(
        GLuint texture,
        ImVec2 image_size,
        ImVec2 mouse_pos,
        ImVec4 color
    ) = 0;

    GLuint get_dummy_vao();
    void apply_program();
    Program load_brush_program(const char* shader_path);

public:
    void draw_at_point(
        GLuint texture,
        ImVec2 image_size,
        ImVec2 mouse_pos,
        ImVec4 color
    );

    void draw_on_segment(
        GLuint fbo,
        GLuint texture,
        ImVec2 start,
        ImVec2 end,
        ImVec4 color,
        bool include_start = false
    );

    void decrease_size();
    void increase_size();

    Id id() const;
    std::string name() const;
    float& size();
    float& opacity();
};

class Pen : public Brush {
public:
    Pen();
    void init_program(GLuint texture, ImVec2 image_size, ImVec2 mouse_pos, ImVec4 color) override;
};

class Eraser : public Brush {
public:
    Eraser();
    void init_program(GLuint texture, ImVec2 image_size, ImVec2 mouse_pos, ImVec4 color) override;
};


// TODO: Move this to it's own file
class BrushManager {
    std::vector<std::unique_ptr<Brush>> m_brushes;
    std::optional<Brush::Id> m_selected_brush;

public:
    BrushManager();

    BrushManager(const BrushManager&) = delete;
    BrushManager& operator=(const BrushManager&) = delete;

    const std::optional<std::reference_wrapper<Brush>> get_brush(Brush::Id brush_id);

    void set_selected_brush(Brush::Id brush_id);
    void set_selected_brush_by_name(std::string name);
    std::optional<std::reference_wrapper<Brush>> get_selected_brush();

    const std::vector<std::unique_ptr<Brush>>& brushes() const;
};

