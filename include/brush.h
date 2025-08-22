#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "glad/glad.h"
#include "glm/glm.hpp"

#include "program.h"

class Brush {
public:
    typedef unsigned long long Id;

    Id id() const { return m_id; }
    std::string name() const { return m_name; }
    float& size() { return m_size; }
    float& opacity() { return m_opacity; }

    void draw_at_point(
        glm::vec2 image_size, 
        glm::vec2 mouse_pos, float pressure, glm::vec3 color, 
        bool is_alpha_locked
    );

    void decrease_size();
    void increase_size();
    void decrease_opacity() { m_opacity = std::max(0.0, m_opacity - 0.1); }
    void increase_opacity() { m_opacity = std::min(1.0, m_opacity + 0.1); }

protected:

    Id m_id;
    std::string m_name;
    float m_size;
    float m_opacity;

    Program m_brush_program;

    Brush();

    virtual void set_program_uniforms(
        glm::vec2 image_size, 
        glm::vec2 mouse_pos, float pressure, glm::vec3 color
    );
    virtual void set_blend_mode(bool is_alpha_locked) = 0;

    void apply_program();
    Program load_brush_program(const char* shader_path);
};

class Pen : public Brush {
public:
    Pen();
    void set_blend_mode(bool is_alpha_locked);
};

class Eraser : public Brush {
public:
    Eraser();
    void set_blend_mode(bool _is_alpha_locked);
    void set_program_uniforms(
        glm::vec2 image_size, 
        glm::vec2 mouse_pos, float pressure, glm::vec3 color
    );
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

