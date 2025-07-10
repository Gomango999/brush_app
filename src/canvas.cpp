#include <algorithm>
#include <cstdint>
#include <vector>
#include <iostream>

#include "imgui.h"

#include "bounding_box.h"
#include "canvas.h"

const size_t N_CHANNELS = 4;
const unsigned int MAX_BRUSH_RADIUS = 1000;

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

    m_canvas.assign(m_height * m_width * N_CHANNELS, 255);

    m_gpu_texture = generate_gpu_texture(
        m_width, 
        m_height, 
        m_canvas.data()
    );

    m_state = CanvasState();
}

void Canvas::draw_circle_at_pos(ImVec2 mouse_pos) {
    BoundingBox bbox = fill_circle(
        int(mouse_pos.x), 
        int(mouse_pos.y), 
        m_state.radius,
        m_state.color
    );

    upload_pixel_data_within_bbox_to_gpu(bbox);
}

BoundingBox Canvas::fill_circle(
    int center_x, 
    int center_y, 
    unsigned int radius,
    ImColor color
) {
    radius = std::min(radius, MAX_BRUSH_RADIUS);
    unsigned int circle_radius_squared = radius * radius;

    size_t y_start = std::max(0, int(center_y) - int(radius));
    size_t y_end = std::min(int(m_height), int(center_y + radius + 1));
    size_t x_start = std::max(0, int(center_x) - int(radius));
    size_t x_end = std::min(int(m_width), int(center_x + radius + 1));

    for (int y = y_start; y < y_end; y++) {
        for (int x = x_start; x < x_end; x++) {
            bool should_be_filled = (center_x - x) * (center_x - x) 
                + (center_y - y) * (center_y - y) <= circle_radius_squared;
            if (should_be_filled) {
                set_pixel(x, y, color);
            }
        }
    }

    return BoundingBox{y_start, y_end, x_start, x_end};
}

void Canvas::set_pixel(size_t x, size_t y, ImColor color) {
    ImU32 packed = (ImU32)color;

    unsigned char r = (packed >> IM_COL32_R_SHIFT) & 0xFF;
    unsigned char g = (packed >> IM_COL32_G_SHIFT) & 0xFF;
    unsigned char b = (packed >> IM_COL32_B_SHIFT) & 0xFF;
    unsigned char a = (packed >> IM_COL32_A_SHIFT) & 0xFF;;

    const int pixel_index = y * m_width * N_CHANNELS + x * N_CHANNELS;
    m_canvas[pixel_index] = r;
    m_canvas[pixel_index + 1] = g;
    m_canvas[pixel_index + 2] = b;
    m_canvas[pixel_index + 3] = a;
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
                pixels.push_back(m_canvas[pixel_index + i]);
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

CanvasState Canvas::state() {
    return m_state;
}

GLuint Canvas::gpu_texture() {
    return m_gpu_texture;
}


