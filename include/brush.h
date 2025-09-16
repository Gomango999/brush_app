#pragma once
#include <algorithm>

#include <glm/fwd.hpp>

#include "program.h"
#include "texture.h"
#include "tools.h"
#include "user_state.h"

class Canvas;

class Brush : public Tool {
public:
    float& size() { return m_size; }
    float& opacity() { return m_opacity; }

    void on_mouse_down(Canvas& canvas, UserState& user_state) override;
    void render_cursor(const Canvas& canvas, const glm::vec2 cursor_pos) override;

    void decrease_size();
    void increase_size();
    void decrease_opacity() { m_opacity = std::max(0.0f, m_opacity - 0.1f); }
    void increase_opacity() { m_opacity = std::min(1.0f, m_opacity + 0.1f); }

protected:
    float m_size;
    float m_opacity;

    Program m_brush_program;
    Program m_cursor_program;


    Brush();

    void draw_at_point(glm::vec2 image_size, glm::vec2 mouse_pos, float pressure, glm::vec3 color, bool is_alpha_locked);
    void draw_segment(glm::vec2 image_size, CursorState start, CursorState end, glm::vec3 color, bool is_alpha_locked);

    virtual void set_program_uniforms(
        glm::vec2 image_size, 
        glm::vec2 mouse_pos, float pressure, glm::vec3 color
    );
    virtual void set_blend_mode(bool is_alpha_locked) = 0;

    void apply_program();
};

class Pen final : public Brush {
public:
    Pen();
    void set_blend_mode(bool is_alpha_locked);
};

class Eraser final : public Brush {
public:
    Eraser();
    void set_blend_mode(bool _is_alpha_locked);
    void set_program_uniforms(
        glm::vec2 image_size, 
        glm::vec2 mouse_pos, float pressure, glm::vec3 color
    );
};


