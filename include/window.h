#include <windows.h>

#include <glfw/glfw3.h>

#include "vec.h"

class Window {
private: 
    GLFWwindow* m_window;

    WNDPROC m_original_wnd_proc = nullptr;

    Vec2 m_pen_pos;
    float m_pen_pressure;
    bool m_pen_down;

    Vec2 m_mouse_pos;
    bool m_mouse_down;

public:
    Window(const char *title, size_t width, size_t height);
    ~Window(); 

    GLFWwindow* window() const;
    WNDPROC original_wnd_proc() const;

    Vec2 get_pen_pos() const { return m_pen_pos; }
    float get_pen_pressure() const { return m_pen_pressure; }
    bool is_pen_down() const { return m_pen_down; }

    void set_pen_pos(const Vec2& pos) { m_pen_pos = pos; }
    void set_pen_pressure(float pressure) { m_pen_pressure = pressure; }
    void set_pen_down(bool down) { m_pen_down = down; }

    Vec2 get_mouse_pos() const { return m_mouse_pos; }
    bool is_mouse_down() const { return m_mouse_down; }

    void set_mouse_pos(const Vec2& pos) { m_mouse_pos = pos; }
    void set_mouse_down(bool down) { m_mouse_down = down; }

    void set_should_close(int value);
    bool should_close();
};