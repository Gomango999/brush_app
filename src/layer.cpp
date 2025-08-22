#include <algorithm>
#include <format>
#include <stdexcept>
#include <string>
#include <utility>

#include "glad/glad.h"
#include "glm/glm.hpp"

#include "brush.h"
#include "frame_buffer.h"
#include "layer.h"
#include "program.h"
#include "texture.h"
#include "vao.h"

Layer::Layer(size_t width, size_t height)
    : m_quad_program("../src/shaders/quad.vert", "../src/shaders/quad.frag"),
    m_frame_buffer(width, height)
{
    static Id current_id = 0;
    current_id++;

    m_id = current_id;
    m_name = std::format("Layer {}", current_id);

    m_is_visible = true;
    m_is_alpha_locked = false;
}

Layer::Layer(Layer&& other) noexcept
    : m_frame_buffer(std::move(other.m_frame_buffer)),
    m_id(other.m_id),
    m_name(std::move(other.m_name)),
    m_is_visible(other.m_is_visible),
    m_is_alpha_locked(other.m_is_alpha_locked),
    m_quad_program(std::move(other.m_quad_program))
{}

Layer& Layer::operator=(Layer&& other) noexcept {
    if (this != &other) {
        m_frame_buffer = std::move(other.m_frame_buffer);
        m_id = other.m_id;
        m_name = std::move(other.m_name);
        m_is_visible = other.m_is_visible;
        m_is_alpha_locked = other.m_is_alpha_locked;
        m_quad_program = std::move(other.m_quad_program);
    }
    return *this;
}

void Layer::draw_with_brush(Brush& brush, glm::vec2 mouse_pos, float pressure, glm::vec3 color) {
    m_frame_buffer.bind();
    m_frame_buffer.set_viewport();

    brush.draw_at_point(
        m_frame_buffer.size(),
        mouse_pos, 
        pressure,
        color, 
        m_is_alpha_locked
    );

    GLuint dummy_vao = VAO::get_dummy();
    glBindVertexArray(dummy_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Layer::render() {
    if (!m_is_visible) return;

    m_quad_program.use();
    m_frame_buffer.texture().bind_to_0();
    m_quad_program.set_uniform_1i("u_texture", 0);

    GLuint dummy_vao = VAO::get_dummy();
    glBindVertexArray(dummy_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
