#pragma once
#include <stdexcept>
#include <utility>      

#include <glad/glad.h>  

class Texture2D {
public:
    Texture2D(size_t width, size_t height) {
        m_width = width;
        m_height = height;

        glGenTextures(1, &m_id);
        if (m_id == 0) {
            throw std::runtime_error("Failed to generate OpenGL texture!");
        }

        glBindTexture(GL_TEXTURE_2D, m_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        const GLuint mip_map_level = 0;
        glTexImage2D(
            GL_TEXTURE_2D, 
            mip_map_level, GL_RGBA8, 
            m_width, m_height, 0, 
            GL_RGBA, GL_UNSIGNED_BYTE, 
            nullptr
        );
    }

    ~Texture2D() {
        if (m_id != 0) {
            glDeleteTextures(1, &m_id);
        }
    }

    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    Texture2D(Texture2D&& other) noexcept
        : m_width(other.m_width),
        m_height(other.m_height),
        m_id(std::exchange(other.m_id, 0)) {}

    Texture2D& operator=(Texture2D&& other) noexcept {
        if (this != &other) {
            if (m_id != 0) {
                glDeleteTextures(1, &m_id);
            }
            m_id = std::exchange(other.m_id, 0);
            m_width = other.m_width;
            m_height = other.m_height;
        }
        return *this;
    }

    void bind() const {
        glBindTexture(GL_TEXTURE_2D, m_id);
    }

    void unbind() const {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    static void set_active(size_t slot) {
        glActiveTexture(GL_TEXTURE0 + slot);
    }

    void bind_to_0() const {
        set_active(0);
        bind();
    }

    GLuint id() const { return m_id; }
    size_t width() const { return m_width; }
    size_t height() const { return m_height; }

private:
    size_t m_width, m_height;
    GLuint m_id = 0;
};