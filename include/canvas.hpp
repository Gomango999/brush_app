#pragma once

#include <cstdint>
#include <vector>

#include "bounding_box.hpp"
#include "canvas.hpp"
#include "color.hpp"

class Canvas {
private:
    size_t m_width;
    size_t m_height;

    // Flattened canvas, stored in row major order.
    std::vector<uint8_t> m_canvas;

    static const size_t N_CHANNELS;
    static const unsigned int MAX_BRUSH_RADIUS;

public:
    Canvas(size_t _width, size_t _height);

    void set_pixel(size_t x, size_t y, Color color);

    void fill_circle(
        size_t center_x,
        size_t center_y,
        unsigned int radius,
        Color color
    );

    // Stores the pixel data within the bounding box inside `data` 
    void set_data_within_bounding_box(
        std::vector<uint8_t>& data,
        BoundingBox bbox
    );

    uint8_t* data();
};
