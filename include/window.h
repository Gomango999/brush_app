#include <GLFW/glfw3.h>

#include "vec.h"

class Window {
private: 
    GLFWwindow* m_window;

public:
    Window(const char *title, size_t width, size_t height);
    ~Window(); 
    GLFWwindow* window();


    Vec2 get_cursor_pos();

    void set_should_close(int value);
    bool should_close();
};