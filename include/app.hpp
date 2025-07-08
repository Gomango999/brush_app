#pragma once

#include <optional>
#include <queue>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "bounding_box.hpp"
#include "canvas.hpp"
#include "color.hpp"
#include "shader.hpp"

class App {
private:
    unsigned int m_screen_width;
    unsigned int m_screen_height;

    GLFWwindow* m_window;
    Shader m_shaders;
    Canvas m_canvas;
    GLuint m_gpu_canvas_texture;

    double m_last_update_time;

public:
    App(
        unsigned int screen_width,
        unsigned int screen_height,
        unsigned int canvas_width,
        unsigned int canvas_height
    );

    void run();

private:
    void update(std::vector<uint8_t>& data_to_update);
    std::queue<BoundingBox> handle_inputs();
    std::optional<BoundingBox> handle_left_click(double x_pos, double y_pos);
    void handle_update_bboxes(std::vector<uint8_t>& update_data, std::queue<BoundingBox>& update_bboxes);
    void draw();

    GLFWwindow* init_window();
    void create_and_bind_full_screen_quad();
    GLuint generate_gpu_canvas_texture();
};

