#include <algorithm>
#include <exception>
#include <functional>
#include <iterator>
#include <optional>
#include <utility>
#include <vector>

#include "glad/glad.h"

#include "brush.h"
#include "canvas.h"
#include "layer.h"
#include "program.h"
#include "vec.h"

const size_t N_CHANNELS = 4;
const float MAX_BRUSH_RADIUS = 1000.0;

Canvas::Canvas(size_t width, size_t height) {
    m_width = width;
    m_height = height;

    m_base_color = Vec3{ 1.0, 1.0, 1.0 };

    glGenFramebuffers(1, &m_output_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_output_fbo);

    glGenTextures(1, &m_output_texture);
    glBindTexture(GL_TEXTURE_2D, m_output_texture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA8,
        m_width, m_height,
        0, GL_RGBA, GL_UNSIGNED_BYTE,
        nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_output_texture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::exception("Could not set up framebuffer");
    }

    m_cursor_program = Program("../src/shaders/quad.vert", "../src/shaders/draw_circle_cursor.frag");
}

Canvas::~Canvas() {
    if (m_output_fbo != 0) glDeleteFramebuffers(1, &m_output_fbo);
    if (m_output_texture != 0) glDeleteFramebuffers(1, &m_output_texture);
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

void Canvas::draw_circle_at_pos(Layer& layer, Brush& brush, Vec2 mouse_pos, Vec3 color) {
    layer.draw_with_brush(brush, mouse_pos, color);
}

void Canvas::draw_circles_on_segment(Layer& layer, Brush& brush, Vec2 start, Vec2 end, Vec3 color, bool include_start, unsigned int num_segments) {
    unsigned int start_index = include_start ? 0 : 1;
    for (unsigned int i = start_index; i <= num_segments; i++) {
        float alpha = (float)i / num_segments;
        Vec2 pos = start * alpha + end * (1.0 - alpha);

        draw_circle_at_pos(layer, brush, pos, color);
    }
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
void Canvas::render(BrushManager& brush_manager, Vec2 mouse_pos) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_output_fbo);
    glViewport(0, 0, m_width, m_height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(m_base_color.r(), m_base_color.g(), m_base_color.b(), 1.0);
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
void Canvas::render_cursor(BrushManager& brush_manager, Vec2 mouse_pos) {
    auto brush_opt = brush_manager.get_selected_brush();
    if (!brush_opt.has_value()) return;
    Brush& brush = brush_opt.value();

    m_cursor_program.use();
    // TODO: Move shared code into a Texture/FBO class  
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_output_texture);
    m_cursor_program.set_uniform_1i("u_texture", 0);
    m_cursor_program.set_uniform_2f("u_tex_dim", m_width, m_height);
    m_cursor_program.set_uniform_2f("u_mouse_pos", mouse_pos.x(), mouse_pos.y());
    m_cursor_program.set_uniform_1f("u_radius", brush.size());

    GLuint dummy_vao = get_dummy_vao();
    glBindVertexArray(dummy_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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

GLuint Canvas::output_texture() const {
    return m_output_texture;
}

