cmake_minimum_required(VERSION 3.15)
project(brush_app)

set(CMAKE_CXX_STANDARD 20)

include_directories(
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/external/imgui"
    "${CMAKE_SOURCE_DIR}/external/imgui/backends"
    "C:/cpp_libs/include"
    "C:/cpp_libs/include/glad-4.3"
    "C:/cpp_libs/include/glad-4.3/glad"
    "C:/cpp_libs/lib/glfw-3.4.bin.WIN64/include"
)

link_directories(
    "C:/cpp_libs/lib/glfw-3.4.bin.WIN64/lib-vc2022"
)

set(GLAD_SRC
    "C:/cpp_libs/src/glad-4.3/glad.c"
)

file(GLOB IMGUI_SRC
    "external/imgui/*.cpp"
    "external/imgui/backends/imgui_impl_glfw.cpp"
    "external/imgui/backends/imgui_impl_opengl3.cpp"
)

set(APP_SOURCES 
    "src/main.cpp"
    "src/app.cpp"
    "src/canvas.cpp"
    "src/gui.cpp"
    "src/layer.cpp"
    "src/program.cpp"
    "src/window.cpp"
)

set (HEADER_FILES
    "include/app.h"
    "include/bounding_box.h"
    "include/canvas.h"
    "include/gui.h"
    "include/layer.h"
    "include/program.h"
    "include/window.h"
)

add_executable(brush_app 
    ${APP_SOURCES}
 )

target_sources(brush_app PRIVATE
    ${GLAD_SRC}
    ${IMGUI_SRC}
)

target_link_libraries(brush_app PRIVATE
    glfw3
    gdi32
)

