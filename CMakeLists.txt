cmake_minimum_required(VERSION 3.15)
project(brush_app)

set(CMAKE_CXX_STANDARD 20)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui/backends
    C:/cpp_libs/include
    C:/cpp_libs/lib/glfw-3.4.bin.WIN64/include
)

set(GLAD_SRC
    C:/cpp_libs/src/glad/glad.c
)

file(GLOB IMGUI_SRC
    external/imgui/*.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

set(APP_SOURCES 
    src/main.cpp
    src/app.cpp
    src/canvas.cpp
    src/gui.cpp
    src/window.cpp
)

set(SOURCES
    ${APP_SOURCES}
    ${GLAD_SRC}
    ${IMGUI_SRC}
)

set (HEADER_FILES
    "include/app.h"
    "include/bounding_box.h"
    "include/canvas.h"
    "include/gui.h"
    "include/window.h"
)

add_executable(
    brush_app 
    ${SOURCES}
    ${HEADER_FILES}
)

target_link_directories(brush_app PRIVATE
    C:/cpp_libs/lib
    C:/cpp_libs/lib/glfw-3.4.bin.WIN64/lib-vc2022
)

target_link_libraries(brush_app PRIVATE
    glfw3
    gdi32
)



set_property(SOURCE src/main.cpp PROPERTY VS_FOLDER "App Source")
set_property(SOURCE src/main.h PROPERTY VS_FOLDER "App Source")
set_property(SOURCE external/imgui/imgui.cpp PROPERTY VS_FOLDER "External/ImGui")
