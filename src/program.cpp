// Code from ChatGPT

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include "program.h"
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>


Program::Program() : m_program_id(0) {}

Program::Program(const std::string& vertex_path, const std::string& fragment_path) {
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

Program::~Program() {
    if (m_program_id != 0) {
        glDeleteProgram(m_program_id);
        m_program_id = 0;
    }
}

Program::Program(Program&& other) noexcept
    : m_program_id(other.m_program_id) {
    other.m_program_id = 0; 
}

Program& Program::operator=(Program&& other) noexcept {
    if (this != &other) {
        if (m_program_id != 0) {
            glDeleteProgram(m_program_id);
        }

        m_program_id = other.m_program_id;
        other.m_program_id = 0; 
    }
    return *this;
}

void Program::use() const {
    if (m_program_id == 0) {
        throw std::runtime_error("Tried to use uninitialised program");
    }

    glUseProgram(m_program_id);
}

GLuint Program::id() const {
    return m_program_id;
}

GLint Program::get_uniform_location(const char* name) {
    return glGetUniformLocation(m_program_id, name);
}

void Program::set_uniform_1i(const char* name, int i) {
    const GLint loc = get_uniform_location(name);
    glUniform1i(loc, i);
}

void Program::set_uniform_1f(const char* name, float f) {
    const GLint loc = get_uniform_location(name);
    glUniform1f(loc, f);
}

void Program::set_uniform_2f(const char* name, float f1, float f2) {
    const GLint loc = get_uniform_location(name);
    glUniform2f(loc, f1, f2);
}

void Program::set_uniform_3f(const char* name, float f1, float f2, float f3) {
    const GLint loc = get_uniform_location(name);
    glUniform3f(loc, f1, f2, f3);
}

void Program::set_uniform_4f(const char* name, float f1, float f2, float f3, float f4) {
    const GLint loc = get_uniform_location(name);
    glUniform4f(loc, f1, f2, f3, f4);
}

void Program::set_uniform_2f(const char* name, const glm::vec2& v) {
    const GLint loc = get_uniform_location(name);
    glUniform2f(loc, v.x, v.y);
}

void Program::set_uniform_3f(const char* name, const glm::vec3& v) {
    const GLint loc = get_uniform_location(name);
    glUniform3f(loc, v.x, v.y, v.z);
}

void Program::set_uniform_4f(const char* name, const glm::vec4& v) {
    const GLint loc = get_uniform_location(name);
    glUniform4f(loc, v.x, v.y, v.z, v.w);
}

void Program::set_uniform_mat3(const char* name, const glm::mat3& mat) {
    const GLint loc = get_uniform_location(name);
    glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

std::string Program::load_shader_source(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Failed to open shader file: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint Program::compile_shader(const std::string& source, GLenum shader_type) {
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
