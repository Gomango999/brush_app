#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "app.hpp"

App::App(
    unsigned int screen_width,
    unsigned int screen_height,
    unsigned int canvas_width,
    unsigned int canvas_height
)
    : m_screen_width(screen_width),
      m_screen_height(screen_height),
      m_canvas(canvas_width, canvas_height)
{
    m_window = initialise_window();
    create_and_bind_full_screen_quad();
    m_shaders.init("src/shaders/vertex.glsl", "src/shaders/fragment.glsl");
    m_gpu_canvas_texture = generate_gpu_canvas_texture();
    initialise_imgui();
}

void App::run() {
    std::vector<uint8_t> data_to_update;
    data_to_update.reserve(m_canvas.width() * m_canvas.height() * 4);

    while (!glfwWindowShouldClose(m_window)) {
        double loop_start_time = glfwGetTime();


        update(data_to_update);


        double dt = glfwGetTime() - loop_start_time;
        printf("dt: %.9f\n", dt);
    }


    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}

void App::update(std::vector<uint8_t>& data_to_update) {
    double loop_start_time = glfwGetTime();
    std::queue<BoundingBox> update_bboxes = handle_inputs();
    handle_update_bboxes(data_to_update, update_bboxes);
    display_interface();
    draw();
}

std::queue<BoundingBox> App::handle_inputs() {
    std::queue<BoundingBox> update_bboxes;

    glfwPollEvents();

    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window, true);
    }

    int left_mouse_state = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT);
    if (left_mouse_state == GLFW_PRESS) {
        double x_pos, y_pos;
        glfwGetCursorPos(m_window, &x_pos, &y_pos);
        std::optional<BoundingBox> bbox = handle_left_click(x_pos, y_pos);
        if (bbox.has_value()) {
            update_bboxes.push(bbox.value());
        }
    }

    return update_bboxes;
}

std::optional<BoundingBox> App::handle_left_click(double x_pos, double y_pos) {
    // BUG: `m_screen_width` and `m_screen_height` are not changed on window resize
    // leading to incorrect calculations here. 
    x_pos = (x_pos / m_screen_width) * m_canvas.width();
    y_pos = (y_pos / m_screen_height) * m_canvas.height();

    double CIRCLE_RADIUS = 200;
    m_canvas.fill_circle(x_pos, y_pos, CIRCLE_RADIUS, BLACK);

    size_t y_start = std::max(0, int(y_pos - CIRCLE_RADIUS));
    size_t y_end = std::min(int(m_canvas.height()), int(y_pos + CIRCLE_RADIUS + 1));
    size_t x_start = std::max(0, int(x_pos - CIRCLE_RADIUS));
    size_t x_end = std::min(int(m_canvas.width()), int(x_pos + CIRCLE_RADIUS + 1));

    BoundingBox bbox = {y_start, y_end, x_start, x_end};
    return bbox;
}

void App::handle_update_bboxes(
    std::vector<uint8_t>& update_data,
    std::queue<BoundingBox>& update_bboxes
) {
    while (!update_bboxes.empty()) {
        BoundingBox update_bbox = update_bboxes.front();
        update_bboxes.pop();

        m_canvas.set_data_within_bounding_box(update_data, update_bbox);

        glBindTexture(GL_TEXTURE_2D, m_gpu_canvas_texture);
        glTexSubImage2D(
            GL_TEXTURE_2D, 0,
            update_bbox.left, update_bbox.top,
            update_bbox.width(), update_bbox.height(),
            GL_RGBA, GL_UNSIGNED_BYTE,
            update_data.data()
        );
    }
}

void App::display_interface() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow(); 
}

void App::draw() {
    m_shaders.use();

    glBindTexture(GL_TEXTURE_2D, m_gpu_canvas_texture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (glfwGetTime() - m_last_update_time >= (1.0 / 60.0)) {
        glfwSwapBuffers(m_window);

        m_last_update_time = glfwGetTime();
    }
}

void error_callback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

GLFWwindow* App::initialise_window() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwSetErrorCallback(error_callback);

    GLFWwindow* window = glfwCreateWindow(m_screen_width, m_screen_height, "Brush", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    return window;
}

void App::create_and_bind_full_screen_quad() {
    float vertices[] = {
        -1.0, -1.0, 0.0f,  0.0, 1.0,
         1.0, -1.0, 0.0f,  1.0, 1.0,
         1.0,  1.0, 0.0f,  1.0, 0.0,
        -1.0,  1.0, 0.0f,  0.0, 0.0,
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

GLuint App::generate_gpu_canvas_texture() {
    GLuint gpu_canvas_texture;
    glGenTextures(1, &gpu_canvas_texture);
    glBindTexture(GL_TEXTURE_2D, gpu_canvas_texture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA,
        m_canvas.width(), m_canvas.height(),
        0,
        GL_RGBA, GL_UNSIGNED_BYTE,
        m_canvas.data()
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    return gpu_canvas_texture;
}

void App::initialise_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);     
    ImGui_ImplOpenGL3_Init();
}