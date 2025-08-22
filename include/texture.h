#pragma once
#include <stdexcept>
#include <utility>      

#include <glad/glad.h>  
#include <glm/glm.hpp>

class Texture2D {
    const GLuint mip_map_level = 0;

public:
    Texture2D(size_t width, size_t height) {
        glGenTextures(1, &m_id);
        if (m_id == 0) {
            throw std::runtime_error("Failed to generate OpenGL texture!");
        }

        glBindTexture(GL_TEXTURE_2D, m_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        assign_texture(width, height);
    }

    ~Texture2D() {
        if (m_id != 0) {
            glDeleteTextures(1, &m_id);
        }
    }

    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    Texture2D(Texture2D&& other) noexcept
        : m_width(std::exchange(other.m_width, 0)),
        m_height(std::exchange(other.m_height, 0)),
        m_id(std::exchange(other.m_id, 0)) {}

    Texture2D& operator=(Texture2D&& other) noexcept {
        if (this != &other) {
            if (m_id != 0) {
                glDeleteTextures(1, &m_id);
            }
            m_id = std::exchange(other.m_id, 0);
            m_width = std::exchange(other.m_width, 0);
            m_height = std::exchange(other.m_height, 0);
        }
        return *this;
    }

    void bind() const {
        glBindTexture(GL_TEXTURE_2D, m_id);
    }

    static void unbind() {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    static void set_active(size_t slot) {
        glActiveTexture(GL_TEXTURE0 + slot);
    }

    void bind_to_0() const {
        set_active(0);
        bind();
    }

    void assign_texture(size_t width, size_t height) {
        m_width = width;
        m_height = height;
        glTexImage2D(
            GL_TEXTURE_2D,
            mip_map_level,
            GL_RGBA8,
            width, height,
            0,
            GL_RGBA, GL_UNSIGNED_BYTE,
            nullptr
        );
    }

    void resize(size_t width, size_t height) {
        if (width != m_width || height != m_height) {
            assign_texture(width, height);
        }
    }

    GLuint id() const { return m_id; }
    size_t width() const { return m_width; }
    size_t height() const { return m_height; }
    glm::vec2 size() const { return glm::vec2(m_width, m_height); }

private:
    size_t m_width, m_height;
    GLuint m_id = 0;
};