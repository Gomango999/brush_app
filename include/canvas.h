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

// TODO: Make these typedefs only exist within this header and canvas.cpp
typedef size_t LayerId;
typedef std::pair<ImVec4, LayerId> PixelInLayer;
typedef std::vector<PixelInLayer> PixelStack;
typedef std::vector<std::vector<PixelStack>> LayerStack;

// `Canvas` the canvas pixel data in both the CPU and GPU. It is
// responsible for updating both textures whenever something is
// drawn to the canvas.
class Canvas {
private:
    size_t m_width, m_height;
    LayerStack m_layer_stack;
    std::vector<uint8_t> m_output_image;
    GLuint m_gpu_texture;
    CanvasState m_state;

public:
    Canvas(size_t _width, size_t _height);

    void draw_circle_at_pos(ImVec2 pos, LayerId layer);

    size_t width();
    size_t height();
    CanvasState get_state();
    void set_state(CanvasState state);
    GLuint gpu_texture();

private:
    BoundingBox fill_circle_in_layer(
        int center_x, 
        int center_y, 
        unsigned int radius, 
        LayerId layer, 
        ImVec4 color
    );
    void set_pixel_in_layer(size_t x, size_t y, LayerId layer_id, ImVec4 color);
    ImVec4 calculate_output_pixel_color(size_t x, size_t y);
    void set_pixel_in_output_image(size_t x, size_t y, ImVec4 color);
    void update_output_image_within_bbox(BoundingBox bbox);
    void upload_pixel_data_within_bbox_to_gpu(BoundingBox bbox);
};


