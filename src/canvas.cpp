#include <algorithm>
#include <exception>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

#include "glad/glad.h"
#include "imgui.h"

#include "canvas.h"
#include "layer.h"

const size_t N_CHANNELS = 4;
const unsigned int MAX_BRUSH_RADIUS = 1000;

Canvas::Canvas(size_t width, size_t height) {
    m_width = width;
    m_height = height;

    m_base_color = ImVec4(1.0, 1.0, 1.0, 1.0);

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
        // TODO: Handle this
        throw std::exception("Could not set up framebuffer");
    }

    m_user_state = UserState();
}

Canvas::~Canvas() {
    if (m_output_fbo != 0) glDeleteFramebuffers(1, &m_output_fbo);
    if (m_output_texture != 0) glDeleteFramebuffers(1, &m_output_texture);
}

Layer::Id Canvas::insert_new_layer_above_selected() {
    Layer::Id target_layer_id = m_user_state.selected_layer.has_value() ?
        m_user_state.selected_layer.value() :
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

void Canvas::delete_selected_layer() {
    if (!m_user_state.selected_layer.has_value()) return;

    Layer::Id target_layer_id = m_user_state.selected_layer.value();

    std::erase_if(m_layers,
        [target_layer_id](const Layer& layer) {
            return layer.id() == target_layer_id;
        });

    // TODO: Set this to the layer below the deleted one. 
    m_user_state.selected_layer = std::nullopt;
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

void Canvas::draw_circle_at_pos(ImVec2 mouse_pos) {
    if (!m_user_state.selected_layer.has_value()) return;

    draw_circle_in_layer(
        int(mouse_pos.x),
        int(mouse_pos.y),
        m_user_state.radius,
        m_user_state.selected_layer.value(),
        m_user_state.color
    );

    // TODO: Add code to deallocate tiles within the Layer class.
}

void Canvas::draw_circle_in_layer(
    int center_x,
    int center_y,
    unsigned int radius,
    Layer::Id layer_id,
    ImVec4 color
) {
    radius = std::min(radius, MAX_BRUSH_RADIUS);
    unsigned int circle_radius_squared = radius * radius;

    auto clamp = [](int x, int minimum, int maximum) {
        return std::max(minimum, std::min(x, maximum));
        };

    size_t y_start = clamp(center_y - int(radius), 0, m_height);
    size_t y_end = clamp(center_y + int(radius) + 1, 0, m_height);
    size_t x_start = clamp(center_x - int(radius), 0, m_width);
    size_t x_end = clamp(center_x + int(radius) + 1, 0, m_width);

    auto layer_opt = lookup_layer(layer_id);
    if (!layer_opt.has_value()) return;
    Layer& layer = layer_opt.value().get();

    for (int y = y_start; y < y_end; y++) {
        for (int x = x_start; x < x_end; x++) {
            bool should_be_filled = (center_x - x) * (center_x - x)
                + (center_y - y) * (center_y - y) <= circle_radius_squared;
            if (should_be_filled) {
                layer.write_pixel(x, y, color);
            }
        }
    }
}

// Combines all the layers together in a single framebuffer.
void Canvas::render_output_image() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_output_fbo);
    glViewport(0, 0, m_width, m_height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glClearColor(m_base_color.x, m_base_color.y, m_base_color.z, 1.0);
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    for (Layer& layer : m_layers) {
        layer.render();
    }


    // Unbind the framebuffer. If we don't do this, this causes
    // ImGUI to render a black screen.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

size_t Canvas::width() const {
    return m_width;
}

size_t Canvas::height() const {
    return m_height;
}

UserState& Canvas::user_state() {
    return m_user_state;
}

const std::vector<Layer>& Canvas::get_layers() {
    return m_layers;
}

GLuint Canvas::output_texture() const {
    return m_output_texture;
}


