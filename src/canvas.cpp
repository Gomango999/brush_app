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
#include "frame_buffer.h"
#include "layer.h"
#include "program.h"

const size_t N_CHANNELS = 4;
const float MAX_BRUSH_RADIUS = 1000.0;

Canvas::Canvas(size_t width, size_t height)
    : m_output_frame_buffer(width, height),
    m_canvas_view(m_output_frame_buffer.width(), m_output_frame_buffer.height())
{
    m_base_color = glm::vec3( 1.0, 1.0, 1.0 );

    m_cursor_program = Program("../src/shaders/quad.vert", "../src/shaders/draw_circle_cursor.frag");
}

Layer::Id Canvas::insert_new_layer_above_selected(std::optional<Layer::Id> selected_layer) {
    Layer::Id target_layer_id = selected_layer.has_value() ?
        selected_layer.value() :
        m_layers.size() > 0 ? m_layers.back().id() : 0;

    auto insert_position = std::find_if(m_layers.begin(), m_layers.end(),
        [target_layer_id](const Layer& layer) {
            return layer.id() == target_layer_id;
        });

    Layer new_layer = Layer(width(), height());
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
    glm::vec2 canvas_pos = m_canvas_view.screen_space_to_world_space(cursor.pos);
    layer.draw_with_brush(brush, canvas_pos, cursor.pressure, color);
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
    return m_output_frame_buffer.get_color_at_pos(point);
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
void Canvas::render(glm::vec2 screen_area, BrushManager& brush_manager, glm::vec2 mouse_pos) {
    m_output_frame_buffer.bind();
    m_output_frame_buffer.set_viewport();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_output_frame_buffer.clear(glm::vec4(m_base_color, 1.0));

    for (Layer& layer : m_layers) {
        layer.render();
    }
    
    m_canvas_view.render(screen_area, m_output_frame_buffer.texture());

    // TODO: Temporarily disabled. 
    //render_cursor(brush_manager, mouse_pos);

    // Unbind the framebuffer. If we don't do this, this causes
    // ImGUI to render a black screen.
    FrameBuffer::unbind();
}

// TODO: Eventually, this call should be deferred to the brush itself.
void Canvas::render_cursor(BrushManager& brush_manager, glm::vec2 mouse_pos) {
    auto brush_opt = brush_manager.get_selected_brush();
    if (!brush_opt.has_value()) return;
    Brush& brush = brush_opt.value();

    const Texture2D& output_texture = m_output_frame_buffer.texture();

    m_cursor_program.use();
    output_texture.bind_to_0();
    m_cursor_program.set_uniform_1i("u_texture", 0);
    m_cursor_program.set_uniform_2f("u_tex_dim", output_texture.size());
    m_cursor_program.set_uniform_2f("u_mouse_pos", mouse_pos);
    m_cursor_program.set_uniform_1f("u_radius", brush.size());

    GLuint dummy_vao = get_dummy_vao();
    glBindVertexArray(dummy_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Canvas::save_as_png(const char* filename) const {
    std::vector<uint8_t> pixels;
    m_output_frame_buffer.get_pixel_data(pixels);
    stbi_write_png(filename, width(), height(), 4, pixels.data(), width() * 4);
}


