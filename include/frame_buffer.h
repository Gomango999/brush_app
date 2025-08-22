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
    Texture2D m_texture;

public:
    FrameBuffer(size_t width, size_t height) : m_texture(width, height) {
        glGenFramebuffers(1, &m_id);
        if (m_id == 0) {
            throw std::runtime_error("Failed to generate framebuffer");
        }

        bind();
        reattach_texture();

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
        m_texture(std::move(other.m_texture)) 
    {}

    FrameBuffer& operator=(FrameBuffer&& other) noexcept {
        if (this != &other) {
            if (m_id != 0) {
                glDeleteFramebuffers(1, &m_id);
            }
            m_id = std::exchange(other.m_id, 0);
            m_texture = std::move(other.m_texture);
        }
        return *this;
    }

    void bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, m_id);
    }

    static void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void set_viewport() const {
        glViewport(0, 0, width(), height());
    }

    std::optional<glm::vec3> get_color_at_pos(glm::vec2 point) {
        if (point.x < 0 || point.x >= width() ||
            point.y < 0 || point.y >= height()) {
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

        pixels.resize(width() * height() * 4);
        glReadPixels(
            0, 0,
            width(), height(),
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            pixels.data()
        );

        unbind();
    }

    void reattach_texture() {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture.id(), 0);
    }

    void resize(size_t width, size_t height) {
        if (width == m_texture.width() && height == m_texture.height()) return;

        m_texture.bind();
        m_texture.resize(width, height);

        bind();
        reattach_texture();
    }

    GLuint id() const { return m_id; }
    const Texture2D& texture() const { return m_texture; }
    GLuint texture_id() const { return m_texture.id(); }
    size_t width() const { return m_texture.width(); }
    size_t height() const { return m_texture.height(); }
    glm::vec2 size() const { return glm::vec2(width(), height()); }

};
