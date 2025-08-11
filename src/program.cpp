// Code from ChatGPT

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include "program.h"


ShaderProgram::ShaderProgram(const std::string& vertex_path, const std::string& fragment_path) {
    std::string vertex_code = load_shader_source(vertex_path);
    std::string fragment_code = load_shader_source(fragment_path);

    GLuint vertex_shader = compile_shader(vertex_code, GL_VERTEX_SHADER);
    GLuint fragment_shader = compile_shader(fragment_code, GL_FRAGMENT_SHADER);

    m_program_id = glCreateProgram();
    glAttachShader(m_program_id, vertex_shader);
    glAttachShader(m_program_id, fragment_shader);
    glLinkProgram(m_program_id);

    GLint success;
    glGetProgramiv(m_program_id, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(m_program_id, 512, nullptr, info_log);
        throw std::runtime_error(std::string("Shader program linking failed: ") + info_log);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

ShaderProgram::~ShaderProgram() {
    if (m_program_id != 0) {
        glDeleteProgram(m_program_id);
        m_program_id = 0;
    }
}

void ShaderProgram::use() const {
    glUseProgram(m_program_id);
}

GLuint ShaderProgram::id() const {
    return m_program_id;
}

std::string ShaderProgram::load_shader_source(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Failed to open shader file: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint ShaderProgram::compile_shader(const std::string& source, GLenum shader_type) {
    GLuint shader = glCreateShader(shader_type);
    const char* src_ptr = source.c_str();
    glShaderSource(shader, 1, &src_ptr, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, nullptr, info_log);
        std::string stage = (shader_type == GL_VERTEX_SHADER) ? "vertex" : "fragment";
        throw std::runtime_error("Shader compilation failed (" + stage + "): " + info_log);
    }

    return shader;
}
