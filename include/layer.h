#pragma once
#include <string>

#include <glm/fwd.hpp>

#include "frame_buffer.h"
#include "program.h"
#include "texture.h"

class Layer {
public:
    typedef unsigned int Id;

private:
    Id m_id;
    std::string m_name;
    bool m_is_visible;
    bool m_is_alpha_locked;

    FrameBuffer m_frame_buffer;

    Program m_quad_program;

public:
    Layer(size_t width, size_t height);
    ~Layer() = default;
    Layer(const Layer&) = delete;
    Layer& operator=(const Layer&) = delete;
    Layer(Layer&& other) noexcept;
    Layer& operator=(Layer&& other) noexcept;

    void bind_canvas_fbo() const;
    void unbind_fbo() const { FrameBuffer::unbind(); };

    void render();

    Id id() const { return m_id; }

    const std::string& name() const { return m_name; }
    void set_name(const std::string& name) { m_name = name; }

    bool is_visible() const { return m_is_visible; }
    void set_visible(bool visible) { m_is_visible = visible; }

    bool is_alpha_locked() const { return m_is_alpha_locked; }
    void set_alpha_lock(bool locked) { m_is_alpha_locked = locked; }

    size_t width() const { return m_frame_buffer.width(); }
    size_t height() const { return m_frame_buffer.height(); }
    glm::vec2 size() const { return m_frame_buffer.size(); }

    const Texture2D& gpu_texture() const { return m_frame_buffer.texture(); }
};
