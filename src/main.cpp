#include <KHR/khrplatform.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "shader.h"
#include <optional>
#include <cassert>

GLFWwindow *init_window();
void create_full_screen_quad();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void error_callback(int error, const char* description);

// settings
const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 800;
const unsigned int CANVAS_WIDTH = 8000;
const unsigned int CANVAS_HEIGHT = 8000;
const double TARGET_FPS = 60.0;
const double FRAME_DURATION = 1.0 / TARGET_FPS; // in seconds

struct BoundingBox {
    size_t top;
    size_t bottom;
    size_t left;
    size_t right;

    size_t width() const {
        return right - left;
    }

    size_t height() const {
        return bottom - top;
    }

    size_t area() const {
        return width() * height();
    }
};

int main() {
    GLFWwindow *window = init_window();

    
    create_full_screen_quad();

    // load shaders
    Shader shaders("src/shaders/vertex.glsl", "src/shaders/fragment.glsl");

    // generate canvas texture
    std::vector<uint8_t> canvas(CANVAS_WIDTH * CANVAS_HEIGHT * 4, 255);

    GLuint g_canvas;
    glGenTextures(1, &g_canvas);
    glBindTexture(GL_TEXTURE_2D, g_canvas);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CANVAS_WIDTH, CANVAS_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, canvas.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


    int t = 0;
    double last_update_time = 0.0;

    std::vector<uint8_t> data_to_update;
    data_to_update.reserve(CANVAS_WIDTH * CANVAS_HEIGHT * 4);

    while(!glfwWindowShouldClose(window))
    {
        double loop_start_time = glfwGetTime();

        // process inputs
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        std::optional<BoundingBox> bbox_to_update_opt;

        int left_mouse_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        if (left_mouse_state == GLFW_PRESS) {
            double x_pos, y_pos;
            glfwGetCursorPos(window, &x_pos, &y_pos);
            x_pos = (x_pos / SCREEN_WIDTH) * CANVAS_WIDTH;
            y_pos = (y_pos / SCREEN_HEIGHT) * CANVAS_HEIGHT;

            // fill in circle in canvas with black.
            double CIRCLE_RADIUS = 200;
            double CIRCLE_RADIUS_SQUARED = CIRCLE_RADIUS * CIRCLE_RADIUS;

            size_t y_start = std::max(0u, (unsigned int)(y_pos - CIRCLE_RADIUS));
            size_t y_end = std::min(CANVAS_HEIGHT, (unsigned int)(y_pos + CIRCLE_RADIUS + 1));
            size_t x_start = std::max(0u, (unsigned int)(x_pos - CIRCLE_RADIUS));
            size_t x_end = std::min(CANVAS_WIDTH, (unsigned int)(x_pos + CIRCLE_RADIUS + 1));

            data_to_update.clear();

            for (int y = y_start; y < y_end; y++) {
                for (int x = x_start; x < x_end; x++) {
                    int curr_pixel_index = y * CANVAS_WIDTH * 4 + x * 4;

                    bool should_be_filled = (x_pos - x) * (x_pos - x) + (y_pos - y) * (y_pos - y) <= CIRCLE_RADIUS_SQUARED;
                    if (should_be_filled) {
                        for (int i = 0; i < 3; i++) {
                            canvas[curr_pixel_index+i] = 0u;
                        }
                    }

                    for (int i = 0; i < 4; i++) {
                        data_to_update.push_back(canvas[curr_pixel_index+i]);
                    }
                }
            }

            bbox_to_update_opt = {y_start, y_end, x_start, x_end};
        }

        shaders.use();

        // if there is something to update, update the texture.
        if (bbox_to_update_opt.has_value()) {
            BoundingBox bbox_to_update = bbox_to_update_opt.value();
            glBindTexture(GL_TEXTURE_2D, g_canvas);
            glTexSubImage2D(
                GL_TEXTURE_2D, 0,
                bbox_to_update.left, bbox_to_update.top,
                bbox_to_update.width(), bbox_to_update.height(),
                GL_RGBA, GL_UNSIGNED_BYTE,
                data_to_update.data()
            );
        }

        // draw the canvas
        glBindTexture(GL_TEXTURE_2D, g_canvas);
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CANVAS_WIDTH, CANVAS_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, canvas.data());
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        double dt = glfwGetTime() - loop_start_time;
        printf("dt: %.9f\n", dt);

        if (glfwGetTime() - last_update_time >= FRAME_DURATION) {

            glfwSwapBuffers(window);
            glfwPollEvents();    

            last_update_time = glfwGetTime();
        }
    }

    glfwTerminate();
    return 0;
}


void create_full_screen_quad() {
    float vertices[] = {
        // positions       texture_coords
        -1.0, -1.0, 0.0f,  0.0, 1.0, // bottom left
         1.0, -1.0, 0.0f,  1.0, 1.0, // bottom right
         1.0,  1.0, 0.0f,  1.0, 0.0, // top right
        -1.0,  1.0, 0.0f,  0.0, 0.0, // top left
    };    
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0,
    };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);  
    glGenBuffers(1, &VBO);  
    glGenBuffers(1, &EBO);  
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*) 0);
    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1); 

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Error callback function
void error_callback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

GLFWwindow *init_window() {
    // load glfw and configure
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwSetErrorCallback(error_callback);

    // create window
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "My First Window", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  

    // load glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }    
    return window;
}