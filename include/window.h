#include <GLFW/glfw3.h>

class Window {
private: 
    GLFWwindow* m_window;

public:
    Window(const char *title, size_t width, size_t height);
    ~Window(); 
    GLFWwindow* window();

    void get_cursor_pos(double* mouse_x, double* mouse_y);
    bool should_close();
};