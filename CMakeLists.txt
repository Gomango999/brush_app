cmake_minimum_required(VERSION 3.15)
project(brush_app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui/backends
    C:/cpp_libs/include
)

set(GLAD_SRC
    C:/cpp_libs/src/glad/glad.c
)

file(GLOB IMGUI_SRC
    external/imgui/*.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

set(SOURCES
    src/main.cpp
    src/app.cpp
    src/canvas.cpp
    src/gui.cpp
    src/window.cpp
    ${GLAD_SRC}
    ${IMGUI_SRC}
)

add_executable(brush_app ${SOURCES})

target_link_directories(brush_app PRIVATE C:/cpp_libs/lib)

target_link_libraries(brush_app PRIVATE
    glfw3
    gdi32
)
