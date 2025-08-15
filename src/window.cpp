#include <cstdlib>
#include <iostream>
#include <windows.h>
#include <windowsx.h>

#include <glad/glad.h>
#include <glfw/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>

#include "vec.h"
#include "window.h"


// This is normally in winuser.h, but for some older SDKs, these 
// constants are missing. We redefine them here.
#ifndef WM_TABLET_QUERYSYSTEMGESTURESTATUS
#define WM_TABLET_QUERYSYSTEMGESTURESTATUS 0x02CC
#endif

#ifndef TABLET_DISABLE_PRESSANDHOLD
#define TABLET_DISABLE_PRESSANDHOLD 0x00000001
#endif

#ifndef TABLET_DISABLE_PENTAPFEEDBACK
#define TABLET_DISABLE_PENTAPFEEDBACK 0x00000008
#endif

#ifndef TABLET_DISABLE_PENBARRELFEEDBACK
#define TABLET_DISABLE_PENBARRELFEEDBACK 0x00000010
#endif

#ifndef TABLET_DISABLE_TOUCHUIFORCEON
#define TABLET_DISABLE_TOUCHUIFORCEON 0x00000100
#endif

static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param) {
    Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (msg) {
        case WM_POINTERUPDATE:
        case WM_POINTERDOWN:
        case WM_POINTERUP: {
            UINT32 pointer_id = GET_POINTERID_WPARAM(w_param);
            POINTER_PEN_INFO pen_info;
            if (GetPointerPenInfo(pointer_id, &pen_info)) {
                POINT m_pen_pos = pen_info.pointerInfo.ptPixelLocation;
                POINT screen_pos = { 0, 0 };
                ClientToScreen(hwnd, &screen_pos);
                Vec2 relative_pen_pos{ 
                    float(m_pen_pos.x - screen_pos.x), 
                    float(m_pen_pos.y - screen_pos.y) 
                };

                window->set_pen_pos(relative_pen_pos);
                window->set_pen_down((pen_info.pointerInfo.pointerFlags & POINTER_FLAG_INCONTACT) != 0);
                window->set_pen_pressure(pen_info.pressure / 1024.0f);
            }
            break;
        }

        case WM_MOUSEMOVE: {
            Vec2 mouse_pos{
                (float)GET_X_LPARAM(l_param),
                (float)GET_Y_LPARAM(l_param)
            };
            window->set_mouse_pos(mouse_pos);
            break;
        }
        case WM_LBUTTONDOWN: {
            window->set_mouse_down(true);
            break;
        }
        case WM_LBUTTONUP: {
            window->set_mouse_down(false);
            break;
        }
        
        // This disables windows hold-to-right-click feature on tablets.
        // Tapping to click still works using this method.
        case WM_TABLET_QUERYSYSTEMGESTURESTATUS: {
            return TABLET_DISABLE_PRESSANDHOLD |
                TABLET_DISABLE_PENTAPFEEDBACK |
                TABLET_DISABLE_PENBARRELFEEDBACK |
                TABLET_DISABLE_TOUCHUIFORCEON;
        }
    }


    return CallWindowProc(window->original_wnd_proc(), hwnd, msg, w_param, l_param);
}

static void error_callback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

Window::Window(const char* title, size_t width, size_t height) :
    m_pen_pos{0.0, 0.0},
    m_pen_pressure(0.0),
    m_pen_down(false),
    m_mouse_pos{0.0, 0.0},
    m_mouse_down(false)
{
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwSetErrorCallback(error_callback);

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (m_window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);

    HWND hwnd = glfwGetWin32Window(m_window);

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
    m_original_wnd_proc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)wnd_proc);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

}

Window::~Window() {
    glfwTerminate();
}


