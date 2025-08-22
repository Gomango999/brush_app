#include <algorithm>
#include <iterator>
#include <memory>
#include <vector>
#include <string>

#include "glad/glad.h"
#include "glm/glm.hpp"

#include "brush.h"
#include "program.h"

Brush::Brush() {
    static Id next_id = 0;
    m_id = next_id;
    
    m_name = "";
    m_opacity = 1.0;
    m_size = 10.0;

    next_id++;
}


GLuint Brush::get_dummy_vao() {
    static GLuint dummy_vao = 0;
    if (dummy_vao == 0) {
        glGenVertexArrays(1, &dummy_vao);
    }
    return dummy_vao;
}

void Brush::set_program_uniforms(
    GLuint texture,
    glm::vec2 image_size,
    glm::vec2 mouse_pos,
    float pressure,
    glm::vec3 color
) {
    m_brush_program.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    m_brush_program.set_uniform_2f("u_tex_dim", image_size);
    m_brush_program.set_uniform_2f("u_circle_pos", mouse_pos);
    m_brush_program.set_uniform_1f("u_radius", m_size * pressure);
    m_brush_program.set_uniform_1f("u_opacity", m_opacity);
    m_brush_program.set_uniform_3f("u_color", color);
}

void Brush::apply_program() {
    m_brush_program.use();

    GLuint dummy_vao = get_dummy_vao();
    glBindVertexArray(dummy_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

Program Brush::load_brush_program(const char* shader_path) {
    return Program("../src/shaders/quad.vert", shader_path);
}

void Brush::draw_at_point(
    GLuint texture,
    glm::vec2 image_size,
    glm::vec2 mouse_pos,
    float pressure,
    glm::vec3 color,
    bool is_alpha_locked
) {
    set_program_uniforms(texture, image_size, mouse_pos, pressure, color);
    set_blend_mode(is_alpha_locked);
    apply_program();
}

const float MIN_BRUSH_SIZE = 1.0f;
const float MAX_BRUSH_SIZE = 1000.0f;
const std::vector<float> BRUSH_SIZES{ 1, 1.5, 2, 2.5, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 17, 20, 25, 30, 40, 60, 70, 80, 100, 120, 150, 170, 200, 250, 300, 400, 500, 600, 700, 800, 1000 };

void Brush::decrease_size() {
    auto it = std::lower_bound(BRUSH_SIZES.begin(), BRUSH_SIZES.end(), m_size);
    if (it == BRUSH_SIZES.begin()) m_size = MIN_BRUSH_SIZE;
    else m_size = *prev(it);
}

void Brush::increase_size() {
    auto it = std::upper_bound(BRUSH_SIZES.begin(), BRUSH_SIZES.end(), m_size);
    if (it == BRUSH_SIZES.end()) m_size = MAX_BRUSH_SIZE;
    else m_size = *it;
}

void Brush::decrease_opacity() {
    m_opacity = std::max(0.0, m_opacity - 0.1);
}

void Brush::increase_opacity() {
    m_opacity = std::min(1.0, m_opacity + 0.1);
}

Brush::Id Brush::id() const {
    return m_id;
}

std::string Brush::name() const {
    return m_name;
}

float& Brush::size() {
    return m_size;
}

float& Brush::opacity() {
    return m_opacity;
}


Pen::Pen() {
    m_name = "Pen";
    m_size = 150.0f;
    m_opacity = 1.0f;

    m_brush_program = load_brush_program("../src/shaders/brush_pen.frag");
}

void Pen::set_blend_mode(bool is_alpha_locked) {
    if (!is_alpha_locked) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
    }
}


Eraser::Eraser() {
    m_name = "Eraser";
    m_size = 200.0f;
    m_opacity = 1.0f;

    m_brush_program = load_brush_program("../src/shaders/brush_eraser.frag");
}

void Eraser::set_blend_mode(bool _is_alpha_locked) {
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
}

void Eraser::set_program_uniforms(
    GLuint texture,
    glm::vec2 image_size,
    glm::vec2 mouse_pos,
    float pressure,
    glm::vec3 _color
) {
    m_brush_program.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    m_brush_program.set_uniform_1i("u_texture", 0);
    m_brush_program.set_uniform_2f("u_tex_dim", image_size);
    m_brush_program.set_uniform_2f("u_circle_pos", mouse_pos);
    m_brush_program.set_uniform_1f("u_radius", m_size * pressure);
    m_brush_program.set_uniform_1f("u_opacity", m_opacity);
}



BrushManager::BrushManager() {
    m_brushes.push_back(std::make_unique<Pen>());
    m_brushes.push_back(std::make_unique<Eraser>());

    if (!m_brushes.empty()) {
        m_selected_brush = m_brushes[0]->id();
    } else {
        m_selected_brush = std::nullopt;
    }
}

const std::optional<std::reference_wrapper<Brush>> BrushManager::get_brush(Brush::Id brush_id) {
    for (const auto& brush : m_brushes) {
        if (brush->id() == brush_id) {
            return std::ref(*brush);
        }
    }
    return std::nullopt;
}

void BrushManager::set_selected_brush(Brush::Id brush_id) {
    for (const auto& brush : m_brushes) {
        if (brush->id() == brush_id) {
            m_selected_brush = brush_id;
            return;
        }
    }
}

void BrushManager::set_selected_brush_by_name(std::string name) {
    for (const auto& brush : m_brushes) {
        if (brush->name() == name) {
            m_selected_brush = brush->id();
            return;
        }
    }
}

std::optional<std::reference_wrapper<Brush>> BrushManager::get_selected_brush() {
    if (!m_selected_brush) return std::nullopt;
    return get_brush(m_selected_brush.value());
}

const std::vector<std::unique_ptr<Brush>>& BrushManager::brushes() const {
    return m_brushes;
}
