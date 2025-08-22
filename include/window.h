#include <windows.h>

#include <glfw/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>

class Window {
private: 
    GLFWwindow* m_window;

    WNDPROC m_original_wnd_proc = nullptr;

    glm::vec2 m_pen_pos;
    float m_pen_pressure;
    bool m_pen_down;

    glm::vec2 m_mouse_pos;
    bool m_mouse_down;

public:
    Window(const char *title, size_t width, size_t height);
    ~Window(); 

    GLFWwindow* window() const { return m_window; }
    WNDPROC original_wnd_proc() const { return m_original_wnd_proc; };


    void set_pen_pos(const glm::vec2& pos) { m_pen_pos = pos; }
    void set_pen_pressure(float pressure) { m_pen_pressure = pressure; }
    void set_pen_down(bool down) { m_pen_down = down; }

    void set_mouse_pos(const glm::vec2& pos) { m_mouse_pos = pos; }
    void set_mouse_down(bool down) { m_mouse_down = down; }

    glm::vec2 get_mouse_pos() const { return m_pen_down ? m_pen_pos : m_mouse_pos; }
    float get_pressure() const { return m_pen_down ? m_pen_pressure : 1.0; }
    bool is_mouse_down() const { return m_mouse_down || m_pen_down; }

    void set_should_close(int value) { glfwSetWindowShouldClose(m_window, value); }
    bool should_close() { return glfwWindowShouldClose(m_window); };
};