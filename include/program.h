// Code from ChatGPT

#pragma once

#include <string>

#include "glad/glad.h"

class ShaderProgram {
public:
    ShaderProgram(const std::string& vertex_path, const std::string& fragment_path);
    ~ShaderProgram();

    void use() const;
    GLuint id() const;

private:
    GLuint m_program_id;

    std::string load_shader_source(const std::string& path);
    GLuint compile_shader(const std::string& source, GLenum shader_type);
};
