#include <KHR/khrplatform.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <windows.h>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "shader.h"

GLFWwindow *init_window();
void create_full_screen_quad();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void process_input(GLFWwindow *window);
void error_callback(int error, const char* description);

// settings
const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;
const double TARGET_FPS = 60.0;
const double FRAME_DURATION = 1.0 / TARGET_FPS; // in seconds

int main() {
    GLFWwindow *window = init_window();

    create_full_screen_quad();

    Shader shaders("src/shaders/vertex.glsl", "src/shaders/fragment.glsl");

    std::vector<uint8_t> canvas(SCREEN_HEIGHT * SCREEN_WIDTH * 4, 255);
    // TODO: Remove checkerboard pattern test
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            if ((x + y) % 2 == 0) {
                for (int c = 0; c < 3; c++) {
                    canvas[y*SCREEN_WIDTH*4 + x*4 + c] = 0;
                }
            }
        }
    }

    // generate texture
    GLuint canvas_texture;
    glGenTextures(1, &canvas_texture);
    glBindTexture(GL_TEXTURE_2D, canvas_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, canvas.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    while(!glfwWindowShouldClose(window))
    {
        double frame_start_time = glfwGetTime();

        process_input(window);

        shaders.use();
        // TODO: Change this to reload every frame
        glBindTexture(GL_TEXTURE_2D, canvas_texture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();    

        double dt = frame_start_time - glfwGetTime();
        if (dt < FRAME_DURATION) {
            double sleep_time = FRAME_DURATION - dt;
            std::this_thread::sleep_for(std::chrono::duration<double>(sleep_time));
        }
    }

    glfwTerminate();
    return 0;
}


void create_full_screen_quad() {
    float vertices[] = {
        // positions       texture_coords
        -1.0, -1.0, 0.0f,  0.0, 0.0, // bottom left
         1.0, -1.0, 0.0f,  0.0, 1.0, // bottom right
         1.0,  1.0, 0.0f,  1.0, 1.0, // top right
        -1.0,  1.0, 0.0f,  1.0, 0.0, // top left
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

void process_input(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
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