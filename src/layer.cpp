#include <algorithm>
#include <format>
#include <stdexcept>
#include <string>
#include <utility>

#include "glad/glad.h"
#include "glm/glm.hpp"

#include "brush.h"
#include "layer.h"
#include "program.h"
#include "texture.h"

static GLuint generate_fbo(const Texture2D& texture) {
    GLuint fbo_id = 0;
    glGenFramebuffers(1, &fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.id(), 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Framebuffer incomplete: status = " + std::to_string(status));
    }

    return fbo_id;
}

Layer::Layer(size_t width, size_t height)
    : m_quad_program("../src/shaders/quad.vert", "../src/shaders/quad.frag"),
    m_gpu_texture(width, height)
{
    static Id current_id = 0;
    current_id++;

    m_id = current_id;
    m_name = std::format("Layer {}", current_id);

    m_is_visible = true;
    m_is_alpha_locked = false;

    if (!GLAD_GL_ARB_sparse_texture) {
        throw std::runtime_error("GL_ARB_sparse_texture not supported");
    }

    m_fbo = generate_fbo(m_gpu_texture);
}

Layer::Layer(Layer&& other) noexcept
    : m_gpu_texture(std::move(other.m_gpu_texture)),
    m_fbo(other.m_fbo),
    m_id(other.m_id),
    m_name(std::move(other.m_name)),
    m_is_visible(other.m_is_visible),
    m_is_alpha_locked(other.m_is_alpha_locked),
    m_quad_program(std::move(other.m_quad_program))
{
    other.m_fbo = 0;
}

Layer& Layer::operator=(Layer&& other) noexcept {
    if (this != &other) {
        m_gpu_texture = std::move(other.m_gpu_texture);
        m_fbo = other.m_fbo;
        m_id = other.m_id;
        m_name = std::move(other.m_name);
        m_is_visible = other.m_is_visible;
        m_is_alpha_locked = other.m_is_alpha_locked;
        m_quad_program = std::move(other.m_quad_program);

        other.m_fbo = 0;
    }
    return *this;
}

GLuint Layer::get_dummy_vao() const {
    // OpenGL requires a VAO to be bound in order for the call not
    // to be discarded. We attach a dummy one, even though the
    // vertex data is hardcoded into the vertex shader. 
    static GLuint dummy_vao = 0;
    if (dummy_vao == 0) {
        glGenVertexArrays(1, &dummy_vao);
    }
    return dummy_vao;
}

void Layer::draw_with_brush(Brush& brush, glm::vec2 mouse_pos, float pressure, glm::vec3 color) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_gpu_texture.width(), m_gpu_texture.height());

    brush.draw_at_point(
        glm::vec2(m_gpu_texture.width(), m_gpu_texture.height()),
        mouse_pos, 
        pressure,
        color, 
        m_is_alpha_locked
    );

    GLuint dummy_vao = get_dummy_vao();
    glBindVertexArray(dummy_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Layer::render() {
    if (!m_is_visible) return;

    m_quad_program.use();
    m_gpu_texture.bind_to_0();
    m_quad_program.set_uniform_1i("u_texture", 0);

    GLuint dummy_vao = get_dummy_vao();
    glBindVertexArray(dummy_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
