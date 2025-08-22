#include <cmath>

#include <glad.h>
#include <glm/glm.hpp>

#include "canvas_view.h"
#include "frame_buffer.h"
#include "texture.h"
#include "vao.h"

CanvasView::CanvasView(size_t window_width, size_t window_height) :
    m_texture(window_width, window_height),
    m_frame_buffer(m_texture.width(), m_texture.height()),
    m_program("../src/shaders/canvas_view.vert", "../src/shaders/canvas_view.frag")
{
    m_scale = 1.0;
    m_rotation = 0.0;
    m_translation = glm::vec2(0.0, 0.0);
}

void CanvasView::render(const Texture2D& canvas) {
    m_frame_buffer.bind();
    m_frame_buffer.set_viewport();
    const glm::vec4 black = glm::vec4(0.0, 0.0, 0.0, 1.0f);
    m_frame_buffer.clear(black);

    glm::mat3 transform = get_transform();

    m_program.use(); 
    m_program.set_uniform_mat3("u_transform", transform);

    canvas.bind_to_0();
    m_program.set_uniform_1i("u_canvas", 0); 

    GLuint dummy_vao = VAO::get_dummy();
    glBindVertexArray(dummy_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    canvas.unbind();
    m_frame_buffer.unbind();
}

glm::mat3 CanvasView::get_transform() const {
    // Canonical order: Scale -> Rotate -> Translate
    return translation_mat() * rotation_mat() * scale_mat();
}

// Zooms into a point defined in screen-space.
void CanvasView::zoom_into_point(glm::vec2 point) {
    const float zoom_factor = 1.1f;
    m_translation = point + (m_translation - point) / zoom_factor;
    m_scale *= zoom_factor;
}

// Zooms away from a point defined in screen-space.
void CanvasView::zoom_out_of_point(glm::vec2 point) {
    const float zoom_factor = 1.1f;
    m_translation = point + (m_translation - point) * zoom_factor;
    m_scale /= zoom_factor;
}

// Rotates around the center of the viewport.
// TODO: Add a version of this function that snaps to angles of 45 degrees.
void CanvasView::rotate(float delta_radians) {
    m_rotation += delta_radians;

    glm::vec2 screen_center(width() * 0.5f, height() * 0.5f);

    glm::vec2 offset = m_translation - screen_center;

    float c = cos(delta_radians);
    float s = sin(delta_radians);
    glm::vec2 rotated_offset = glm::vec2(
        c * offset.x - s * offset.y,
        s * offset.x + c * offset.y
    );

    m_translation = screen_center + rotated_offset;
}

void CanvasView::move(glm::vec2 translation) {
    m_translation += translation;
}

void CanvasView::flip() {
    m_scale = -m_scale;
    m_rotation = -m_rotation;
    m_translation = -m_translation;
}

glm::mat3 CanvasView::translation_mat() const {
    glm::mat3 mat(1.0f);
    mat[2] = glm::vec3(m_translation, 1.0f);
    return mat;
}

glm::mat3 CanvasView::scale_mat() const {
    glm::mat3 mat(1.0f);
    mat[0][0] = m_scale;
    mat[1][1] = m_scale;
    return mat;
}

glm::mat3 CanvasView::rotation_mat() const {
    glm::mat3 mat(1.0f);
    float c = cos(m_rotation);
    float s = sin(m_rotation);

    mat[0][0] = c;  mat[0][1] = -s;
    mat[1][0] = s;  mat[1][1] = c;
    return mat;
}

void CanvasView::resize(size_t width, size_t height) {
    m_texture.bind();
    m_texture.resize(width, height);
    m_frame_buffer.bind();
    m_frame_buffer.attach_texture_to_color_output(m_texture);
}




