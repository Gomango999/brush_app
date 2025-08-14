#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "glad/glad.h"

#include "program.h"
#include "vec.h"

class Brush {
public:
    typedef unsigned long long Id;

    Id id() const;
    std::string name() const;
    float& size();
    float& opacity();

    void draw_at_point(GLuint texture, Vec2 image_size, Vec2 mouse_pos, Vec3 color, float pressure, bool is_alpha_locked);

    void decrease_size();
    void increase_size();

protected:

    Id m_id;
    std::string m_name;
    float m_size;
    float m_opacity;

    Program m_brush_program;

    Brush();

    virtual void set_program_uniforms(GLuint texture, Vec2 image_size, Vec2 mouse_pos, Vec3 color, float pressure);
    virtual void set_blend_mode(bool is_alpha_locked) = 0;

    GLuint get_dummy_vao();
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
    void set_program_uniforms(GLuint texture, Vec2 image_size, Vec2 mouse_pos, Vec3 _color, float pressure) override;
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

