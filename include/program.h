// Code from ChatGPT

#pragma once

#include <string>

#include "glad/glad.h"

class Program {
public:
    Program(const std::string& vertex_path, const std::string& fragment_path);
    ~Program();
    Program(const Program&) = default;
    Program& operator=(const Program&) = default;
    Program(Program&&) noexcept;
    Program& operator=(Program&&) noexcept;

    void use() const;
    GLuint id() const;

    GLint get_uniform_location(const char* name);
    void set_uniform_1i(const char* name, int i);
    void set_uniform_1f(const char* name, float f);
    void set_uniform_2f(const char* name, float f1, float f2);
    void set_uniform_3f(const char* name, float f1, float f2, float f3);
    void set_uniform_4f(const char* name, float f1, float f2, float f3, float f4);

private:
    GLuint m_program_id;

    std::string load_shader_source(const std::string& path);
    GLuint compile_shader(const std::string& source, GLenum shader_type);
};
