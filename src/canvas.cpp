#include <algorithm>
#include <cstdint>
#include <cstdio> // required for stb_image_write.h to work
#include <exception>
#include <functional>
#include <iterator>
#include <optional>
#include <utility>
#include <vector>

#include "glad/glad.h"
#include "glm/glm.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "brush.h"
#include "canvas.h"
#include "layer.h"
#include "program.h"

const size_t N_CHANNELS = 4;
const float MAX_BRUSH_RADIUS = 1000.0;

Canvas::Canvas(size_t width, size_t height)
    : m_output_texture(width, height) 
{
    m_width = width;
    m_height = height;

    m_base_color = glm::vec3( 1.0, 1.0, 1.0 );

    glGenFramebuffers(1, &m_output_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_output_fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_output_texture.id(), 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::exception("Could not set up framebuffer");
    }

    m_cursor_program = Program("../src/shaders/quad.vert", "../src/shaders/draw_circle_cursor.frag");
}

Canvas::~Canvas() {
    if (m_output_fbo != 0) glDeleteFramebuffers(1, &m_output_fbo);
}

Layer::Id Canvas::insert_new_layer_above_selected(std::optional<Layer::Id> selected_layer) {
    Layer::Id target_layer_id = selected_layer.has_value() ?
        selected_layer.value() :
        m_layers.size() > 0 ? m_layers.back().id() : 0;

    auto insert_position = std::find_if(m_layers.begin(), m_layers.end(),
        [target_layer_id](const Layer& layer) {
            return layer.id() == target_layer_id;
        });

    Layer new_layer = Layer(m_width, m_height);
    Layer::Id new_layer_id = new_layer.id();

    if (insert_position == m_layers.end()) {
        m_layers.push_back(std::move(new_layer));
    }
    else {
        m_layers.insert(insert_position + 1, std::move(new_layer));
    }

    return new_layer_id;
}

std::optional<Layer::Id> Canvas::delete_selected_layer(std::optional<Layer::Id> selected_layer) {
    if (!selected_layer.has_value()) return selected_layer;

    Layer::Id target_layer_id = selected_layer.value();

    auto it = std::find_if(m_layers.begin(), m_layers.end(),
        [target_layer_id](const Layer& layer) {
            return layer.id() == target_layer_id;
        });
    if (it == m_layers.end()) return selected_layer;

    size_t index = std::distance(m_layers.begin(), it);
    m_layers.erase(it);

    if (m_layers.empty()) {
        return std::nullopt;
    } else if (index == 0) {
        return m_layers[0].id();
    } else {
        return m_layers[index-1].id();
    }
}

void Canvas::move_layer_up(std::optional<Layer::Id> layer_id) {
    move_layer(layer_id, 1);
}

void Canvas::move_layer_down(std::optional<Layer::Id> layer_id) {
    move_layer(layer_id, -1);
}

void Canvas::move_layer(std::optional<Layer::Id> layer_id, int delta) {
    if (!layer_id.has_value()) return; 

    // TODO: Move this search into a helper function
    Layer::Id target_id = layer_id.value();
    auto it = std::find_if(m_layers.begin(), m_layers.end(),
        [target_id](const Layer& layer) { return layer.id() == target_id; });
    if (it == m_layers.end()) return; 

    int index = std::distance(m_layers.begin(), it);
    int new_index = index + delta;

    new_index = std::min(new_index, int(m_layers.size() - 1));
    new_index = std::max(new_index, 0);
    if (index == new_index) return;

    Layer layer = std::move(m_layers[index]);
    m_layers.erase(m_layers.begin() + index);

    m_layers.insert(m_layers.begin() + new_index, std::move(layer));
}

std::optional<std::reference_wrapper<Layer>> Canvas::lookup_layer(Layer::Id layer_id) {
    auto layer = std::find_if(m_layers.begin(), m_layers.end(),
        [layer_id](const Layer& layer) {
            return layer.id() == layer_id;
        });

    if (layer != m_layers.end()) {
        return std::ref(*layer);
    }
    else {
        return std::nullopt;
    }
}

bool Canvas::get_layer_visibility(Layer::Id layer_id) {
    auto layer = lookup_layer(layer_id);
    if (!layer.has_value()) return false;
    return layer.value().get().is_visible();
}

void Canvas::set_layer_visibility(Layer::Id layer_id, bool is_visible) {
    auto layer = lookup_layer(layer_id);
    if (layer.has_value()) {
        layer.value().get().set_visible(is_visible);
    }
}

bool Canvas::get_layer_alpha_lock(Layer::Id layer_id) {
    auto layer = lookup_layer(layer_id);
    if (!layer.has_value()) return false;
    return layer.value().get().is_alpha_locked();
}

void Canvas::set_layer_alpha_lock(Layer::Id layer_id, bool is_alpha_locked) {
    auto layer = lookup_layer(layer_id);
    if (layer.has_value()) {
        layer.value().get().set_alpha_lock(is_alpha_locked);
    }
}

void Canvas::draw_circle_at_pos(Layer& layer, Brush& brush, CursorState cursor, glm::vec3 color) {
    layer.draw_with_brush(brush, cursor.pos, cursor.pressure, color);
}

void Canvas::draw_circles_on_segment(Layer& layer, Brush& brush, CursorState start, CursorState end, glm::vec3 color) {
    float dist = glm::length(start.pos - end.pos);
    float min_pressure = std::min(start.pressure, end.pressure);
    float min_size = brush.size() * min_pressure;

    int num_segments = int(dist / min_size) * 8;
    num_segments = std::max(1, num_segments);
    num_segments = std::min(16, num_segments); 

    for (unsigned int i = 1; i <= num_segments; i++) {
        float alpha = (float)i / num_segments;
        glm::vec2 pos = start.pos * alpha + end.pos * (1.0f - alpha);
        float pressure = start.pressure * alpha + end.pressure * (1.0f - alpha);

        draw_circle_at_pos(layer, brush, CursorState(pos, pressure), color);
    }
}

std::optional<glm::vec3> Canvas::get_color_at_pos(glm::vec2 point) {
    if (point.x < 0 || point.x >= m_width ||
        point.y < 0 || point.y >= m_height) {
        return std::nullopt;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_output_fbo);

    uint8_t pixel[4];
    glReadPixels(int(point.x), int(point.y), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);
    glm::vec3 color = glm::vec3{ float(pixel[0] / 255.0), float(pixel[1] / 255.0), float(pixel[2] / 255.0) };

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return color;
}

GLuint Canvas::get_dummy_vao() const {
    // OpenGL requires a VAO to be bound in order for the call not
    // to be discarded. We attach a dummy one, even though the
    // vertex data is hardcoded into the vertex shader. 
    static GLuint dummy_vao = 0;
    if (dummy_vao == 0) {
        glGenVertexArrays(1, &dummy_vao);
    }
    return dummy_vao;
}

// Combines all the layers together in a single framebuffer.
void Canvas::render(BrushManager& brush_manager, glm::vec2 mouse_pos) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_output_fbo);
    glViewport(0, 0, m_width, m_height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(m_base_color.r, m_base_color.g, m_base_color.b, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    for (Layer& layer : m_layers) {
        layer.render();
    }
    
    render_cursor(brush_manager, mouse_pos);

    // Unbind the framebuffer. If we don't do this, this causes
    // ImGUI to render a black screen.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// TODO: Eventually, this call should be deferred to the brush itself.
void Canvas::render_cursor(BrushManager& brush_manager, glm::vec2 mouse_pos) {
    auto brush_opt = brush_manager.get_selected_brush();
    if (!brush_opt.has_value()) return;
    Brush& brush = brush_opt.value();

    m_cursor_program.use();
    m_output_texture.bind_to_0();
    m_cursor_program.set_uniform_1i("u_texture", 0);
    m_cursor_program.set_uniform_2f("u_tex_dim", m_width, m_height);
    m_cursor_program.set_uniform_2f("u_mouse_pos", mouse_pos);
    m_cursor_program.set_uniform_1f("u_radius", brush.size());

    GLuint dummy_vao = get_dummy_vao();
    glBindVertexArray(dummy_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Canvas::load_output_image(std::vector<uint8_t>& pixels) const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_output_fbo);
    glViewport(0, 0, m_width, m_height);

    pixels.resize(m_width * m_height * 4);
    glReadPixels(
        0, 0,
        m_width, m_height,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixels.data()
    );

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Canvas::save_as_png(const char* filename) const {
    std::vector<uint8_t> pixels;
    load_output_image(pixels);
    stbi_write_png(filename, m_width, m_height, 4, pixels.data(), m_width * 4);
}


size_t Canvas::width() const {
    return m_width;
}

size_t Canvas::height() const {
    return m_height;
}

const std::vector<Layer>& Canvas::get_layers() const {
    return m_layers;
}

const Texture2D& Canvas::output_texture() const {
    return m_output_texture;
}

