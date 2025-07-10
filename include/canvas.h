#pragma once

#include <cstdint>
#include <vector>

#include <glad/glad.h>

#include "bounding_box.h"

struct CanvasState {
    ImVec4 color;
    float radius;
    float opacity;

    CanvasState() {
        color = ImVec4(0.0, 0.0, 0.0, 1.0);
        radius = 200.0;
        opacity = 1.0;
    };
};

// `Canvas` the canvas pixel data in both the CPU and GPU. It is
// responsible for updating both textures whenever something is
// drawn to the canvas.
class Canvas {
private:
    size_t m_width;
    size_t m_height;

    // `m_canvas` is the flattened canvas, stored in row major order.
    std::vector<uint8_t> m_canvas;
    GLuint m_gpu_texture;
    CanvasState m_state;

public:
    Canvas(size_t _width, size_t _height);

    void draw_circle_at_pos(ImVec2 pos);

    size_t width();
    size_t height();
    CanvasState state();
    GLuint gpu_texture();

private:
    BoundingBox fill_circle(
        int center_x, 
        int center_y, 
        unsigned int radius, 
        ImColor color
    );
    void set_pixel(size_t x, size_t y, ImColor color);
    void upload_pixel_data_within_bbox_to_gpu(BoundingBox bbox);
};

