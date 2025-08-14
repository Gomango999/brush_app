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

protected:

    Id m_id;
    std::string m_name;
    float m_size;
    float m_opacity;

    Program m_brush_program;

    Brush();

    virtual void init_program(GLuint texture, Vec2 image_size, Vec2 mouse_pos, Vec3 color);

    GLuint get_dummy_vao();
    void apply_program();
    Program load_brush_program(const char* shader_path);

public:
    void draw_at_point(GLuint texture, Vec2 image_size, Vec2 mouse_pos, Vec3 color);

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
};

class Eraser : public Brush {
public:
    Eraser();
    void init_program(GLuint texture, Vec2 image_size, Vec2 mouse_pos, Vec3 _color) override;
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

