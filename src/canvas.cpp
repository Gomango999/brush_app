#include <algorithm>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "imgui.h"
#include <glad/glad.h>

#include "bounding_box.h"
#include "canvas.h"

const size_t N_CHANNELS = 4;
const unsigned int MAX_BRUSH_RADIUS = 1000;

static void initialise_layer_stack(size_t width, size_t height, LayerStack &layer_stack) {
    layer_stack.clear();

    layer_stack = LayerStack(
        height,
        std::vector<PixelStack>(
            width,
            PixelStack{}
        )
    );
}

static GLuint generate_gpu_texture(size_t width, size_t height, uint8_t data[]) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA,
        width, height,
        0,
        GL_RGBA, GL_UNSIGNED_BYTE,
        data 
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    return texture;
}

Canvas::Canvas(size_t width, size_t height) {
    m_width = width;
    m_height = height;
    
    m_layers = std::vector<Layer>{};
    m_lookup_layer_by_id = std::unordered_map<Layer::Id, std::vector<Layer>::iterator>{};

    m_base_color = ImVec4(1.0, 1.0, 1.0, 1.0);
    initialise_layer_stack(m_width, m_height, m_layer_stack);

    m_output_image.assign(m_width * m_height * N_CHANNELS, 255);
    update_full_output_image();

    m_gpu_texture = generate_gpu_texture(
        m_width, 
        m_height, 
        m_output_image.data()
    );

    m_user_state = UserState();
}

Layer::Id Canvas::insert_new_layer_above_selected() {
    Layer::Id target_layer_id = m_user_state.selected_layer.has_value() ?
        m_user_state.selected_layer.value() :
        m_layers.size() > 0 ? m_layers.back().id : 0;

    auto insert_position = std::find_if(m_layers.begin(), m_layers.end(),
        [target_layer_id](const Layer& layer) {
            return layer.id == target_layer_id;
        });

    Layer new_layer = Layer(0);

    if (insert_position == m_layers.end()) {
        m_layers.push_back(new_layer);
    } else {
        m_layers.insert(insert_position + 1, new_layer);
    }

    // TODO: Abstract this one day, as well as the corresponding code 
    // in delete_selected_layer
    m_lookup_layer_by_id.clear();
    int i = 0;
    for (auto it = m_layers.begin(); it != m_layers.end(); it++, i++) {
        it->height = i;
        m_lookup_layer_by_id[it->id] = it;
    }

    return new_layer.id;
}

void Canvas::delete_selected_layer() {
    if (!m_user_state.selected_layer.has_value()) return;

    Layer::Id target_layer_id = m_user_state.selected_layer.value();

    std::erase_if(m_layers,
        [target_layer_id](const Layer& layer) {
            return layer.id == target_layer_id;
        });
    
    m_lookup_layer_by_id.clear();
    int i = 0;
    for (auto it = m_layers.begin(); it != m_layers.end(); it++, i++) {
        it->height = i;
        m_lookup_layer_by_id[it->id] = it;
    }

    // TODO: Extract this functionality to a LayerStacks class
    // remove all pixels within the layer stacks
    for (size_t y = 0; y < m_height; y++) {
        for (size_t x = 0; x < m_width; x++) {
            PixelStack& pixel_stack = m_layer_stack[y][x];
            std::erase_if(pixel_stack,
                [target_layer_id](const PixelInLayer& pixel) {
                    // TODO: Make this a struct, instead of a pair
                    return pixel.second == target_layer_id;
                }
            );
        }
    }

    update_full_output_image();
}

bool Canvas::get_layer_visibility(Layer::Id layer_id) {
    auto layer = m_lookup_layer_by_id.find(layer_id);
    if (layer == m_lookup_layer_by_id.end()) return false;
    return layer->second->is_visible;
}

void Canvas::set_layer_visibility(Layer::Id layer_id, bool is_visible) {
    auto layer = m_lookup_layer_by_id.find(layer_id);
    if (layer == m_lookup_layer_by_id.end()) return;

    bool was_visible = layer->second->is_visible;
    layer->second->is_visible = is_visible;
    
    bool visibility_has_changed = is_visible != was_visible;
    if (visibility_has_changed) {
        update_full_output_image();
        upload_full_pixel_data_to_gpu();
    }
}

void Canvas::draw_circle_at_pos(ImVec2 mouse_pos) {
    if (!m_user_state.selected_layer.has_value()) return;

    BoundingBox bbox = fill_circle_in_layer(
        int(mouse_pos.x),
        int(mouse_pos.y),
        m_user_state.radius,
        m_user_state.selected_layer.value(),
        m_user_state.color
    );

    update_output_image_within_bbox(bbox);
    upload_pixel_data_within_bbox_to_gpu(bbox);
}

BoundingBox Canvas::fill_circle_in_layer(
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

    for (int y = y_start; y < y_end; y++) {
        for (int x = x_start; x < x_end; x++) {
            bool should_be_filled = (center_x - x) * (center_x - x) 
                + (center_y - y) * (center_y - y) <= circle_radius_squared;
            if (should_be_filled) {
                set_pixel_in_layer(x, y, layer_id, color);
            }
        }
    }

    return BoundingBox{y_start, y_end, x_start, x_end};
}

// SOMEDAY: Write tests for this function
void Canvas::set_pixel_in_layer(size_t x, size_t y, Layer::Id layer_id, ImVec4 color) {
    bool pixel_outside = x >= m_width || y >= m_height;
    if (pixel_outside) return;

    PixelStack& pixel_stack = m_layer_stack[y][x];

    // find the layer if it exists, otherwise, find the insertion point (i.e. the 
    // position of the first layer that is higher than it)
    size_t layer_height = m_lookup_layer_by_id[layer_id]->height;
    auto pixel_it = std::find_if(
        pixel_stack.begin(), 
        pixel_stack.end(), 
        [this, layer_height](PixelInLayer pixel_layer) {
            return m_lookup_layer_by_id[pixel_layer.second]->height >= layer_height;
        }
    );

    bool layer_exists = pixel_it != pixel_stack.end() 
        && pixel_it->second == layer_id;
    bool is_transparent = (color.w == 0.0);
    if (is_transparent) {
        // transparent pixels should be removed from the pixel stack.
        if (layer_exists) {
            pixel_stack.erase(pixel_it);
        }
    } else {
        if (layer_exists) {
            pixel_it->first = color;
        } else {
            PixelInLayer pixel(color, layer_id);
            pixel_stack.insert(pixel_it, pixel);
        }
    }
}

// SOMEDAY: Write tests for this function
static ImVec4 blend_colors(ImVec4 color_1, ImVec4 color_2) {
    float blend_1 = 1.0 - color_2.w;
    float blend_2 = color_2.w;
    return ImVec4(
        color_1.x * blend_1 + color_2.x * blend_2,
        color_1.y * blend_1 + color_2.y * blend_2,
        color_1.z * blend_1 + color_2.z * blend_2,
        color_1.w * blend_1 + color_2.w * blend_2
    );
}

ImVec4 Canvas::calculate_output_pixel_color(size_t x, size_t y) {
    ImVec4 final_color = m_base_color;
    for (auto& [layer_color, layer_id] : m_layer_stack[y][x]) {
        if (m_lookup_layer_by_id[layer_id]->is_visible) {
            final_color = blend_colors(final_color, layer_color);
        }
    }
    return final_color;
}

void Canvas::set_pixel_in_output_image(size_t x, size_t y, ImVec4 color) {
    ImU32 packed = (ImU32)(ImColor)color;

    unsigned char r = (packed >> IM_COL32_R_SHIFT) & 0xFF;
    unsigned char g = (packed >> IM_COL32_G_SHIFT) & 0xFF;
    unsigned char b = (packed >> IM_COL32_B_SHIFT) & 0xFF;
    unsigned char a = (packed >> IM_COL32_A_SHIFT) & 0xFF;;

    const int pixel_index = y * m_width * N_CHANNELS + x * N_CHANNELS;
    m_output_image[pixel_index] = r;
    m_output_image[pixel_index + 1] = g;
    m_output_image[pixel_index + 2] = b;
    m_output_image[pixel_index + 3] = a;
}

void Canvas::update_full_output_image()
{
    BoundingBox entire_image = { 0, m_height, 0, m_width };
    update_output_image_within_bbox(entire_image);
}

void Canvas::update_output_image_within_bbox(BoundingBox bbox) {
    for (int y = bbox.top; y < bbox.bottom; y++) {
        for (int x = bbox.left; x < bbox.right; x++) {
            ImVec4 color = calculate_output_pixel_color(x, y);
            set_pixel_in_output_image(x, y, color);
        }
    }
}

void Canvas::upload_full_pixel_data_to_gpu() {
    BoundingBox entire_image = { 0, m_height, 0, m_width };
    upload_pixel_data_within_bbox_to_gpu(entire_image);
}

void Canvas::upload_pixel_data_within_bbox_to_gpu(BoundingBox bbox) {
    // collect pixel data from cpu side array
    static std::vector<uint8_t> pixels;
    pixels.reserve(m_width * m_height * N_CHANNELS);
    pixels.clear();

    for (int y = bbox.top; y < bbox.bottom; y++) {
        for (int x = bbox.left; x < bbox.right; x++) {
            int pixel_index = y * m_width * N_CHANNELS + x * N_CHANNELS;
            for (int i = 0; i < N_CHANNELS; i++) {
                pixels.push_back(m_output_image[pixel_index + i]);
            }
        }
    }

    // upload to gpu texture
    glBindTexture(GL_TEXTURE_2D, m_gpu_texture);
    glTexSubImage2D(
        GL_TEXTURE_2D, 0,
        bbox.left, bbox.top,
        bbox.width(), bbox.height(),
        GL_RGBA, GL_UNSIGNED_BYTE,
        pixels.data()
    );
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

GLuint Canvas::get_gpu_texture() const {
    return m_gpu_texture;
}


