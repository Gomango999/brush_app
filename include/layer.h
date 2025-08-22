#pragma once
#include <string>

#include "glad/glad.h"
#include "glm/glm.hpp"

#include "brush.h"
#include "frame_buffer.h"
#include "program.h"
#include "texture.h"

class Layer {
public:
    typedef unsigned int Id;

private:
    struct TileCoords {
        size_t x, y;
    };

    Id m_id;
    std::string m_name;
    bool m_is_visible;
    bool m_is_alpha_locked;

    Texture2D m_gpu_texture;
    FrameBuffer m_frame_buffer;

    Program m_quad_program;

public:
    Layer(size_t width, size_t height);
    ~Layer() = default;
    Layer(const Layer&) = delete;
    Layer& operator=(const Layer&) = delete;
    Layer(Layer&& other) noexcept;
    Layer& operator=(Layer&& other) noexcept;

    void draw_with_brush(Brush& brush, glm::vec2 mouse_pos, float pressure, glm::vec3 color);
    void render();

    Id id() const { return m_id; }

    const std::string& name() const { return m_name; }
    void set_name(const std::string& name) { m_name = name; }

    bool is_visible() const { return m_is_visible; }
    void set_visible(bool visible) { m_is_visible = visible; }

    bool is_alpha_locked() const { return m_is_alpha_locked; }
    void set_alpha_lock(bool locked) { m_is_alpha_locked = locked; }

    size_t width() const { return m_gpu_texture.width(); }
    size_t height() const { return m_gpu_texture.height(); }

    const Texture2D& gpu_texture() const { return m_gpu_texture; }
};
