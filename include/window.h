#include <GLFW/glfw3.h>

class Window {
private: 
    GLFWwindow* m_window;

public:
    Window(const char *title, size_t width, size_t height);
    ~Window(); 
    GLFWwindow* window();
    bool should_close();
};