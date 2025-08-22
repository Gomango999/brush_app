#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>     
#include <vector>

#include <glad/glad.h>  
#include <glm/glm.hpp>  

#include "texture.h"

class FrameBuffer {
    GLuint m_id = 0;
    GLsizei m_width = 0;
    GLsizei m_height = 0;

public:
    FrameBuffer(GLsizei width, GLsizei height)
        : m_width(width), m_height(height)
    {
        glGenFramebuffers(1, &m_id);
        if (m_id == 0) {
            throw std::runtime_error("Failed to generate framebuffer");
        }

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("Framebuffer incomplete: status = " + std::to_string(status));
        }
    }

    ~FrameBuffer() {
        if (m_id != 0) {
            glDeleteFramebuffers(1, &m_id);
        }
    }

    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;

    FrameBuffer(FrameBuffer&& other) noexcept
        : m_id(std::exchange(other.m_id, 0)),
        m_width(std::exchange(other.m_width, 0)),
        m_height(std::exchange(other.m_height, 0)) {
    }

    FrameBuffer& operator=(FrameBuffer&& other) noexcept {
        if (this != &other) {
            if (m_id != 0) {
                glDeleteFramebuffers(1, &m_id);
            }
            m_id = std::exchange(other.m_id, 0);
            m_width = std::exchange(other.m_width, 0);
            m_height = std::exchange(other.m_height, 0);
        }
        return *this;
    }

    void bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, m_id);
    }

    static void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void attach_texture_to_color_output(const Texture2D& texture) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.id(), 0);
    }

    void set_viewport() const {
        glViewport(0, 0, m_width, m_height);
    }

    std::optional<glm::vec3> get_color_at_pos(glm::vec2 point) {
        if (point.x < 0 || point.x >= m_width ||
            point.y < 0 || point.y >= m_height) {
            return std::nullopt;
        }

        bind();

        uint8_t pixel[4];
        glReadPixels(int(point.x), int(point.y), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);
        glm::vec3 color = glm::vec3(pixel[0], pixel[1], pixel[2]) / 255.0f;

        unbind();

        return color;
    }

    void clear(glm::vec4 color) {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void get_pixel_data(std::vector<uint8_t>& pixels) const {
        bind();

        pixels.resize(m_width * m_height * 4);
        glReadPixels(
            0, 0,
            m_width, m_height,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            pixels.data()
        );

        unbind();
    }

    GLuint id() const { return m_id; }
    GLsizei width() const { return m_width; }
    GLsizei height() const { return m_height; }
    glm::vec2 size() const { return glm::vec2(m_width, m_height); }

};
