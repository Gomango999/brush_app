
#include <algorithm>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include "glad/glad.h"
#include "glm/glm.hpp"

#include "brush.h"
#include "canvas.h"
#include "layer.h"
#include "program.h"
#include "texture.h"
#include "vao.h"

Brush::Brush() {
    m_name = "Unnamed Brush";
    m_opacity = 1.0;
    m_size = 10.0;
    m_cursor_program = Program("../src/shaders/quad.vert", "../src/shaders/draw_circle_cursor.frag");
}

void Brush::set_program_uniforms(
    glm::vec2 image_size,
    glm::vec2 mouse_pos,
    float pressure,
    glm::vec3 color
) {
    m_brush_program.use();
    m_brush_program.set_uniform_2f("u_tex_dim", image_size);
    m_brush_program.set_uniform_2f("u_circle_pos", mouse_pos);
    m_brush_program.set_uniform_1f("u_radius", m_size * pressure);
    m_brush_program.set_uniform_1f("u_opacity", m_opacity);
    m_brush_program.set_uniform_3f("u_color", color);
}

void Brush::apply_program() {
    m_brush_program.use();

    GLuint dummy_vao = VAO::get_dummy();
    glBindVertexArray(dummy_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static Program load_brush_program(const char* shader_path) {
    return Program("../src/shaders/quad.vert", shader_path);
}

const float MIN_BRUSH_SIZE = 1.0f;
const float MAX_BRUSH_SIZE = 1000.0f;
const std::vector<float> BRUSH_SIZES{ 1, 1.5, 2, 2.5, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 17, 20, 25, 30, 40, 60, 70, 80, 100, 120, 150, 170, 200, 250, 300, 400, 500, 600, 700, 800, 1000 };

void Brush::on_mouse_down(Canvas& canvas, UserState& user_state) {
    Layer::Id layer_id = user_state.selected_layer.value();
    auto layer_opt = canvas.lookup_layer(layer_id);
    if (!layer_opt.has_value()) return;
    const Layer& layer = layer_opt.value().get();

    layer.bind_canvas_fbo();

    if (!user_state.prev_cursor.has_value()) {
        CursorState cursor = user_state.cursor;
        cursor.pos = canvas.screen_space_to_canvas_space(cursor.pos);
        draw_at_point(
            layer.size(),
            cursor.pos,
            cursor.pressure,
            user_state.selected_color,
            layer.is_alpha_locked()
        );
    } else {
        CursorState start = user_state.prev_cursor.value();
        CursorState end = user_state.cursor;
        start.pos = canvas.screen_space_to_canvas_space(start.pos);
        end.pos = canvas.screen_space_to_canvas_space(end.pos);
        draw_segment(layer.size(), start, end, user_state.selected_color, layer.is_alpha_locked());
    }

    layer.unbind_fbo();
}

// By default, brushes use the circular cursor program.
void Brush::render_cursor(const Canvas& canvas, const glm::vec2 cursor_pos) {
    const Texture2D& output_texture = canvas.screen_texture();
    output_texture.bind_to_0();
    
    m_cursor_program.use();
    m_cursor_program.set_uniform_1i("u_texture", 0);
    m_cursor_program.set_uniform_2f("u_tex_dim", output_texture.size());
    m_cursor_program.set_uniform_2f("u_mouse_pos", cursor_pos);
    float radius_in_pixels = canvas.canvas_space_to_screen_space(size());
    m_cursor_program.set_uniform_1f("u_radius", radius_in_pixels);

    GLuint dummy_vao = VAO::get_dummy();
    glBindVertexArray(dummy_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


void Brush::draw_at_point(
    glm::vec2 image_size,
    glm::vec2 mouse_pos,
    float pressure,
    glm::vec3 color,
    bool is_alpha_locked
) {
    set_program_uniforms(image_size, mouse_pos, pressure, color);
    set_blend_mode(is_alpha_locked);
    apply_program();
}

void Brush::draw_segment(
    glm::vec2 image_size, 
    CursorState start, 
    CursorState end, 
    glm::vec3 color, 
    bool is_alpha_locked
) {
    float dist = glm::length(start.pos - end.pos);
    float min_pressure = std::min(start.pressure, end.pressure);
    float min_size = size() * min_pressure;

    int num_segments = int(dist / min_size) * 8;
    num_segments = std::max(1, num_segments);
    num_segments = std::min(16, num_segments);

    for (unsigned int i = 1; i <= num_segments; i++) {
        float alpha = (float)i / num_segments;
        glm::vec2 pos = start.pos * alpha + end.pos * (1.0f - alpha);
        float pressure = start.pressure * alpha + end.pressure * (1.0f - alpha);

        draw_at_point(
            image_size,
            pos,
            pressure,
            color,
            is_alpha_locked
        );
    }
}

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
    glm::vec2 image_size,
    glm::vec2 mouse_pos,
    float pressure,
    glm::vec3 _color
) {
    m_brush_program.use();
    m_brush_program.set_uniform_2f("u_tex_dim", image_size);
    m_brush_program.set_uniform_2f("u_circle_pos", mouse_pos);
    m_brush_program.set_uniform_1f("u_radius", m_size * pressure);
    m_brush_program.set_uniform_1f("u_opacity", m_opacity);
}




