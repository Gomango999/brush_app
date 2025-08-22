#include <cmath>

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

    m_scale = 1.0;
    m_rotation = 0.0;
    m_translation = glm::vec2(0.0, 0.0);
}

// TODO: Move these into a helper header 
inline glm::mat3 translate_mat3(const glm::vec2& t) {
    glm::mat3 m(1.0f);
    m[2] = glm::vec3(t, 1.0f);
    return m;
}

static inline glm::mat3 scale_mat3(float s) {
    glm::mat3 m(1.0f);
    m[0][0] = s;
    m[1][1] = s;
    return m;
}

static inline glm::mat3 scale_mat3(float sx, float sy) {
    glm::mat3 m(1.0f);
    m[0][0] = sx;
    m[1][1] = sy;
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

// Transforms from NDC coordinates of full screen into NDC coordinates of 
// screen space where canvas should render.
// Order of operations: scale (aspect_ratio) -> scale (zoom) -> rotate -> translate
glm::mat3 CanvasView::get_transform() const {
    glm::vec2 default_canvas_size = fit_canvas_to_screen(m_frame_buffer.size(), glm::vec2(m_canvas_height, m_canvas_height));
    glm::mat3 screen_to_default = scale_mat3(
        default_canvas_size.x / m_frame_buffer.width(),
        default_canvas_size.y / m_frame_buffer.height()
    );
    return translate_mat3(m_translation) * rotate_mat3(m_rotation) * scale_mat3(m_scale) * screen_to_default;
}

glm::vec2 CanvasView::screen_space_to_world_space(glm::vec2 point) const {
    glm::vec3 screen_ndc = glm::vec3(
        ((point.x / m_frame_buffer.size().x) * 2.0f - 1.0f),
        -((point.y / m_frame_buffer.size().y) * 2.0f - 1.0f),
        1.0f
    );

    // We compare our point in screen space NDC to the bottom left and
    // top right corners of the canvas in screen space NDC. We use this
    // to generate a value between [0..1] in `canvas_normalised`

    glm::mat3 transform = get_transform();
    glm::vec3 canvas_bottom_left_ndc = transform * glm::vec3(-1.0, -1.0, 1.0);
    glm::vec3 canvas_top_right_ndc = transform * glm::vec3(1.0, 1.0, 1.0);

    glm::vec3 canvas_normalised = (screen_ndc - canvas_bottom_left_ndc)
        / (canvas_top_right_ndc - canvas_bottom_left_ndc);

    glm::vec2 world_pos = glm::vec2(
        canvas_normalised.x * m_canvas_width,
        (1.0f - canvas_normalised.y) * m_canvas_height
    );
    return world_pos;
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








