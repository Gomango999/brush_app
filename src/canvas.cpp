#include <algorithm>
#include <cstdint>
#include <vector>

#include "bounding_box.hpp"
#include "color.hpp"
#include "canvas.hpp"

const size_t Canvas::N_CHANNELS = 4;
const unsigned int Canvas::MAX_BRUSH_RADIUS = 1000;

Canvas::Canvas(size_t _width, size_t _height) {
    m_width = _width;
    m_height = _height;
    m_canvas.assign(m_height * m_width * N_CHANNELS, 255);
}

void Canvas::set_pixel(size_t x, size_t y, Color color) {
    const int pixel_index = y * m_width * N_CHANNELS + x * N_CHANNELS;
    m_canvas[pixel_index] = color.r;
    m_canvas[pixel_index + 1] = color.g;
    m_canvas[pixel_index + 2] = color.b;
    m_canvas[pixel_index + 3] = color.a;
}

void Canvas::fill_circle(
    size_t center_x, 
    size_t center_y, 
    unsigned int radius,
    Color color
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
}

void Canvas::set_data_within_bounding_box(
    std::vector<uint8_t>& data,
    BoundingBox bbox
) {
    data.clear();

    for (int y = bbox.top; y < bbox.bottom; y++) {
        for (int x = bbox.left; x < bbox.right; x++) {
            int pixel_index = y * m_width * N_CHANNELS + x * N_CHANNELS;
            for (int i = 0; i < N_CHANNELS; i++) {
                data.push_back(m_canvas[pixel_index + i]);
            }
        }
    }
}

uint8_t* Canvas::data() {
    return m_canvas.data();
}