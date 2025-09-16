#include <cmath>
#include <numbers>
#include <optional>

#include <glad.h>
#include <glm/glm.hpp>

#include "canvas_view.h"
#include "frame_buffer.h"
#include "texture.h"
#include "vao.h"


static glm::vec4 canvas_bg_color = glm::vec4(0.0, 0.0, 0.0, 1.0f);

// Initially we don't know the window size, but we need to define
// a non-zero texture for the frame buffer. These will later be 
// resized on every frame to account for the window size.
static size_t INITIAL_WINDOW_WIDTH = 100;
static size_t INITIAL_WINDOW_HEIGHT = 100;

CanvasView::CanvasView(size_t canvas_width, size_t canvas_height):
    m_frame_buffer(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT),
    m_program("../src/shaders/canvas_view.vert", "../src/shaders/canvas_view.frag")
{
    m_canvas_width = canvas_width;
    m_canvas_height = canvas_height;

    m_scale = glm::vec2(1.0, 1.0);
    m_rotation = 0.0;
    m_translation = glm::vec2(0.0, 0.0);
}

static inline glm::mat3 translate_mat3(const glm::vec2& t) {
    glm::mat3 m(1.0f);
    m[2] = glm::vec3(t, 1.0f);
    return m;
}

static inline glm::mat3 scale_mat3(const glm::vec2& s) {
    glm::mat3 m(1.0f);
    m[0][0] = s.x;
    m[1][1] = s.y;
    return m;
}

static inline glm::mat3 rotate_mat3(float theta) {
    float c = cos(theta);
    float s = sin(theta);

    glm::mat3 m(1.0f);
    m[0][0] = c;  m[0][1] = -s;
    m[1][0] = s;  m[1][1] = c;
    return m;
}

// Fits the canvas within the screen dimensions, whilst keeping the aspect ratio.
// Returns the fitted canvas size.
static glm::vec2 fit_canvas_to_screen(glm::vec2 screen_size, glm::vec2 canvas_size) {
    float canvas_aspect = float(canvas_size.x) / float(canvas_size.y);
    float screen_aspect = float(screen_size.x) / float(screen_size.y);

    if (screen_aspect > canvas_aspect) {
        return glm::vec2(screen_size.y * canvas_aspect, screen_size.y);
    } else {
        return glm::vec2(screen_size.x, screen_size.x / canvas_aspect);
    }
}

void CanvasView::render(glm::vec2 screen_size, const Texture2D& canvas) {
    m_frame_buffer.resize(screen_size.x, screen_size.y);

    m_frame_buffer.bind();
    m_frame_buffer.set_viewport();
    m_frame_buffer.clear(canvas_bg_color);

    glm::mat3 transform = get_transform();

    m_program.use(); 
    m_program.set_uniform_mat3("u_transform", transform);

    canvas.bind_to_0();
    m_program.set_uniform_1i("u_canvas", 0); 

    GLuint dummy_vao = VAO::get_dummy();
    glBindVertexArray(dummy_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    VAO::unbind();
    Texture2D::unbind();
    FrameBuffer::unbind();
}

// Transforms from NDC coordinates of full screen in screen space into NDC coordinates of 
// screen space where canvas should render.
// Order of operations: scale (aspect_ratio) -> scale (zoom) -> rotate -> translate
glm::mat3 CanvasView::get_transform() const {
    glm::vec2 default_canvas_size = fit_canvas_to_screen(
        m_frame_buffer.size(), 
        glm::vec2(m_canvas_width, m_canvas_height)
    );
    glm::mat3 screen_to_default = scale_mat3(default_canvas_size / m_frame_buffer.size());
    return translate_mat3(m_translation) * rotate_mat3(m_rotation)
        * scale_mat3(m_scale) * screen_to_default;
}

static float projection_ratio(const glm::vec2& A, const glm::vec2& B) {
    return glm::dot(A, B) / glm::dot(B, B);
}

glm::vec2 CanvasView::screen_space_to_canvas_space(glm::vec2 point) const {

    glm::vec2 point_ndc = glm::vec2(
        ((point.x / m_frame_buffer.size().x) * 2.0f - 1.0f),
        -((point.y / m_frame_buffer.size().y) * 2.0f - 1.0f)
    );

    // We compare our point to the bottom left and top right corners 
    // of the canvas in NDC space. We use this to generate a x and y 
    // values in [0..1] in [normalised_point_on_canvas];

    glm::mat3 transform = get_transform();
    glm::vec2 canvas_bottom_left_ndc = transform * glm::vec3(-1.0, -1.0, 1.0);
    glm::vec2 canvas_top_left_ndc = transform * glm::vec3(-1.0, 1.0, 1.0);
    glm::vec2 canvas_bottom_right_ndc = transform * glm::vec3(1.0, -1.0, 1.0);

    glm::vec2 normalised_point_on_canvas = glm::vec2(
        projection_ratio(point_ndc - canvas_bottom_left_ndc, canvas_bottom_right_ndc - canvas_bottom_left_ndc),
        projection_ratio(point_ndc - canvas_bottom_left_ndc, canvas_top_left_ndc - canvas_bottom_left_ndc)
    );

    glm::vec2 canvas_pos = glm::vec2(
        normalised_point_on_canvas.x * m_canvas_width,
        normalised_point_on_canvas.y * m_canvas_height
    );
    return canvas_pos;
}

// Zooms into a point defined in screen-space.
void CanvasView::zoom_into_point(glm::vec2 point, float zoom_factor) {
    glm::vec2 screen_center = m_frame_buffer.size() * 0.5f;
    glm::vec2 screen_space_offset = screen_center - point;
    glm::vec2 ndc_offset = (screen_space_offset / m_frame_buffer.size()) * 2.0f;
    glm::vec2 point_translation = m_translation + ndc_offset;
    // [point_translation] is a translation in the same NDC space as 
    // [m_translation]. It stores the translation from [point] to the 
    // centre of the canvas, whereas [m_translation] stores the 
    // translation from the centre of the screen to the centre of the
    // canvas. 
    glm::vec2 zoom_offset = point_translation * (zoom_factor - 1.0f);
    
    m_translation += zoom_offset;
    m_scale *= zoom_factor;

    // TODO: Add a maximum zoom and minimum zoom
    // Min zoom should be something like width is 5% of screen space. 
    // Max zoom should be something like 10 pixels on the screen horizontally
}

// Returns an angle between (-pi, pi]
static float normalise_angle(float angle) {
    while (angle < -std::numbers::pi) angle += std::numbers::pi * 2.0f;
    while (angle >= std::numbers::pi) angle -= std::numbers::pi * 2.0f;
    return angle;
}

// Rotates around the center of the viewport.
// TODO: Add a version of this function that snaps to angles of 45 degrees.
// It cannot work on deltas, instead, it needs to work on a before/after model.
void CanvasView::rotate(float delta_radians) {
    m_rotation = normalise_angle(m_rotation + delta_radians);

    glm::vec2 screen_center(width() * 0.5f, height() * 0.5f);

    float c = cos(delta_radians);
    float s = sin(delta_radians);
    glm::vec2 new_translation = glm::vec2(
        c * m_translation.x - s * m_translation.y,
        s * m_translation.x + c * m_translation.y
    );

    m_translation = new_translation;
}

void CanvasView::set_rotation(float radians) {
    m_rotation = normalise_angle(radians);
}

void CanvasView::move(glm::vec2 translation) {
    glm::vec2 ndc_translation = glm::vec2(
        (translation.x / m_frame_buffer.size().x) * 2.0f,
        -(translation.y / m_frame_buffer.size().y) * 2.0f
    );

    m_translation += ndc_translation;
}

void CanvasView::flip() {
    m_scale.x = -m_scale.x;
    m_rotation = -m_rotation;
    m_translation.x = -m_translation.x;
}








