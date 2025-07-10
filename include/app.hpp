#pragma once

#include <optional>
#include <queue>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "bounding_box.hpp"
#include "canvas.hpp"
#include "color.hpp"
#include "shader.hpp"

struct BrushState {
    ImVec4 color;
    float radius;
    float opacity;

    BrushState() {
        color = ImVec4(0.0, 0.0, 0.0, 1.0);
        radius = 200.0;
        opacity = 1.0;
    };
};

// TODO: Separate out the OpenGL Rendering part from the actual app functionality.
class App {
private:
    unsigned int m_screen_width;
    unsigned int m_screen_height;
    unsigned int m_canvas_display_width;
    unsigned int m_canvas_display_height;

    GLFWwindow* m_window;
    Shader m_shaders;
    Canvas m_canvas;
    GLuint m_gpu_canvas_texture;
    // TODO: Come up with a better name for this.
    std::vector<uint8_t> m_update_data;

    BrushState brush_state;

    ImVec2 canvas_window_pos;

    double m_last_update_time;
    double m_last_dt;

public:
    App(
        unsigned int screen_width,
        unsigned int screen_height,
        unsigned int canvas_width,
        unsigned int canvas_height
    );

    void run();

private:
    void update();
    void handle_inputs();
    std::optional<BoundingBox> handle_left_click(ImVec2 pos);
    void handle_update_bbox(BoundingBox bbox);
    void define_imgui_interface();
    void render();

    GLFWwindow* initialise_window();
    void create_and_bind_full_screen_quad();
    GLuint generate_gpu_canvas_texture();
    void initialise_imgui();
};

