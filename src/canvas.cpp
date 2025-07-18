#include <algorithm>
#include <cstdint>
#include <vector>
#include <iostream>

#include "imgui.h"

#include "bounding_box.h"
#include "canvas.h"

const size_t N_CHANNELS = 4;
const unsigned int MAX_BRUSH_RADIUS = 1000;

LayerStack initialise_layer_stack(size_t width, size_t height) {
    ImVec4 white = ImVec4(1.0, 1.0, 1.0, 1.0);
    size_t initial_layer_id = 0;
    PixelInLayer pixel(white, initial_layer_id);

    // SOMEDAY: Consider initialising a pixel stack of capacity 4 to reduce 
    // reallocation.
    PixelStack initial_pixel_stack{pixel};
    std::vector<PixelStack> initial_row(width, initial_pixel_stack);
    LayerStack intial_layer_stack(height, initial_row);
    return intial_layer_stack;
}

GLuint generate_gpu_texture(size_t width, size_t height, uint8_t data[]) {
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
    m_layer_stack = initialise_layer_stack(width, height);

    m_output_image.assign(m_width * m_height * N_CHANNELS, 255);
    BoundingBox entire_image = {0, height, 0, width};
    update_output_image_within_bbox(entire_image);

    m_gpu_texture = generate_gpu_texture(
        m_width, 
        m_height, 
        m_output_image.data()
    );

    m_state = CanvasState();
}

void Canvas::draw_circle_at_pos(ImVec2 mouse_pos, LayerId layer_id) {
    BoundingBox bbox = fill_circle_in_layer(
        int(mouse_pos.x), 
        int(mouse_pos.y), 
        m_state.radius,
        layer_id,
        m_state.color
    );

    update_output_image_within_bbox(bbox);
    upload_pixel_data_within_bbox_to_gpu(bbox);
}

BoundingBox Canvas::fill_circle_in_layer(
    int center_x, 
    int center_y, 
    unsigned int radius,
    LayerId layer_id,
    ImVec4 color
) {
    radius = std::min(radius, MAX_BRUSH_RADIUS);
    unsigned int circle_radius_squared = radius * radius;

    auto clamp = [](int x, int minimum, int maximum) {
        return std::max(minimum, std::min(x, maximum));
    };

    size_t y_start = clamp(int(center_y) - int(radius), 0, m_height);
    size_t y_end = clamp(int(center_y) + int(radius) + 1, 0, m_height);
    size_t x_start = clamp(int(center_x) - int(radius), 0, m_width);
    size_t x_end = clamp(int(center_x) + int(radius) + 1, 0, m_width);

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

// TODO: Write tests for this function
void Canvas::set_pixel_in_layer(size_t x, size_t y, LayerId layer_id, ImVec4 color) {
    bool pixel_outside = x >= m_width || y >= m_height;
    if (pixel_outside) return;

    PixelStack& pixel_stack = m_layer_stack[y][x];

    // If a layer with id matching `layer_id` exists, then `pixel_it`
    // will point to it. Otherwise, `pixel_it` refers to the first
    // layer above.
    auto pixel_it = std::find_if(
        pixel_stack.begin(), 
        pixel_stack.end(), 
        [=](PixelInLayer pixel_layer) {
            return pixel_layer.second >= layer_id;
        }
    );

    bool layer_exists = 
        pixel_it != pixel_stack.end() && 
        pixel_it->second == layer_id;
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

// TODO: Write tests for this
ImVec4 blend_colors(ImVec4 color_1, ImVec4 color_2) {
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
    ImVec4 final_color = ImVec4(0.0, 0.0, 0.0, 0.0);
    for (auto [color, _] : m_layer_stack[y][x]) {
        final_color = blend_colors(final_color, color);
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

void Canvas::update_output_image_within_bbox(BoundingBox bbox) {
    for (int y = bbox.top; y < bbox.bottom; y++) {
        for (int x = bbox.left; x < bbox.right; x++) {
            ImVec4 color = calculate_output_pixel_color(x, y);
            set_pixel_in_output_image(x, y, color);
        }
    }
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

size_t Canvas::width() {
    return m_width;
}

size_t Canvas::height() {
    return m_height;
}

CanvasState Canvas::get_state() {
    return m_state;
}

void Canvas::set_state(CanvasState state) {
    m_state = state;
}

GLuint Canvas::gpu_texture() {
    return m_gpu_texture;
}


