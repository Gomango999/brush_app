#include <iostream>

#include "app.hpp"

App::App(
    unsigned int screen_width,
    unsigned int screen_height,
    unsigned int canvas_width,
    unsigned int canvas_height
)
    : m_screen_width(screen_width),
      m_screen_height(screen_height),
      m_canvas(canvas_width, canvas_height),
      // TODO: To be removed when we implement zooming. For now we just
      // hard code the canvas's size on the screen.
      m_canvas_display_width(800),
      m_canvas_display_height(800)
{
    m_window = initialise_window();
    create_and_bind_full_screen_quad();
    m_shaders.init("src/shaders/vertex.glsl", "src/shaders/fragment.glsl");
    m_gpu_canvas_texture = generate_gpu_canvas_texture();
    m_update_data.reserve(m_canvas.width() * m_canvas.height() * 4);
    initialise_imgui();
}

void App::run() {
    std::vector<uint8_t> data_to_update;

    while (!glfwWindowShouldClose(m_window)) {
        double loop_start_time = glfwGetTime();

        update();

        m_last_dt = glfwGetTime() - loop_start_time;
    }


    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}

void App::update() {
    handle_inputs();
    define_imgui_interface();
    render();
}

void App::handle_inputs() {
    glfwPollEvents();

    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window, true);
    }
}


void App::define_imgui_interface() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    
    ImGui::Begin("Color");
    ImGuiColorEditFlags base_flags = ImGuiColorEditFlags_None;
    ImGui::ColorEdit4("Color", (float*) &brush_state.color, base_flags);
    ImGui::End();

    ImGui::Begin("Brush");
    ImGui::SliderFloat("Size", &brush_state.radius, 1.0f, 1000.0f, "%f");
    ImGui::SliderFloat("Opacity", &brush_state.opacity, 0.0f, 1.0f);
    ImGui::End();


    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    ImGui::Begin("Canvas", nullptr, window_flags);

    int canvas_width, canvas_height;
    glBindTexture(GL_TEXTURE_2D, m_gpu_canvas_texture);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &canvas_width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &canvas_height);

    // TODO: Probably don't want ImGUI to handle this input, rather I should dispatch 
    // it to my underlying app.

    // Draw to texture if mouse is clicked 
    ImVec2 mouse_pos = ImVec2(0, 0);
    if (ImGui::IsWindowHovered()) {
        ImVec2 absolute_mouse_pos = ImGui::GetMousePos();
        ImVec2 window_pos = ImGui::GetCursorScreenPos();
        mouse_pos = ImVec2(
            absolute_mouse_pos.x - window_pos.x,
            absolute_mouse_pos.y - window_pos.y
        );
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            std::optional<BoundingBox> bbox = handle_left_click(mouse_pos);
            if (bbox.has_value()) {
                handle_update_bbox(bbox.value());
            }
        }
    }

    ImTextureID canvas_texture = (ImTextureID)(intptr_t)m_gpu_canvas_texture;
    ImGui::Image(canvas_texture, ImVec2(m_canvas_display_width, m_canvas_display_height));
    ImGui::End();

    ImGui::Begin("Debug");
    // We write to a buffer first, since ImGui doesn't provide enough precision
    // otherwise
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.9f", m_last_dt);
    ImGui::LabelText("dt", buffer);
    snprintf(buffer, sizeof(buffer), "%.1f", 1.0 / m_last_dt);
    ImGui::LabelText("fps", buffer);

    snprintf(buffer, sizeof(buffer), "(%d, %d)", (int)mouse_pos.x, (int)mouse_pos.y);
    ImGui::LabelText("mouse position", buffer);
    ImGui::End();


    ImGui::Begin("Layers");
    ImGui::Text("Unimplemented");
    ImGui::End();
}

std::optional<BoundingBox> App::handle_left_click(ImVec2 pos) {
    // convert coordinates into coordinates on the canvas.
    pos = ImVec2(
        (pos.x / m_canvas_display_width) * m_canvas.width(),
        (pos.y / m_canvas_display_height) * m_canvas.height()
    );

    double CIRCLE_RADIUS = brush_state.radius;
    m_canvas.fill_circle(pos.x, pos.y, CIRCLE_RADIUS, brush_state.color);

    size_t y_start = std::max(0, int(pos.y - CIRCLE_RADIUS));
    size_t y_end = std::min(int(m_canvas.height()), int(pos.y + CIRCLE_RADIUS + 1));
    size_t x_start = std::max(0, int(pos.x - CIRCLE_RADIUS));
    size_t x_end = std::min(int(m_canvas.width()), int(pos.x + CIRCLE_RADIUS + 1));

    BoundingBox bbox = {y_start, y_end, x_start, x_end};
    return bbox;
}

void App::handle_update_bbox(BoundingBox bbox) {
    m_canvas.set_data_within_bounding_box(m_update_data, bbox);

    glBindTexture(GL_TEXTURE_2D, m_gpu_canvas_texture);
    glTexSubImage2D(
        GL_TEXTURE_2D, 0,
        bbox.left, bbox.top,
        bbox.width(), bbox.height(),
        GL_RGBA, GL_UNSIGNED_BYTE,
        m_update_data.data()
    );
}

void App::render() {
    // m_shaders.use();

    // glBindTexture(GL_TEXTURE_2D, m_gpu_canvas_texture);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);     
    ImGui_ImplOpenGL3_Init();
}