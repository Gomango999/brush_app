cmake_minimum_required(VERSION 3.15)
project(brush_app)

set(CMAKE_CXX_STANDARD 20)

include_directories(
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/external/imgui"
    "${CMAKE_SOURCE_DIR}/external/imgui/backends"
    "${CMAKE_SOURCE_DIR}/external/stb"
    "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/include"
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

file(GLOB IMGUI_SRC_ALL
    "external/imgui/*.cpp"
    "external/imgui/backends/imgui_impl_glfw.cpp"
    "external/imgui/backends/imgui_impl_opengl3.cpp"
)
source_group("External\\ImGui" FILES ${IMGUI_SRC_ALL})

file(GLOB STB_SRC_ALL
    "external/stb/*.h"
)
source_group("External\\Stb" FILES ${STB_SRC_ALL})

file(GLOB SHADER_FILES "src/shaders/*")
source_group("Shaders" FILES ${SHADER_FILES})

file(GLOB APP_SOURCES "src/*.cpp")

file(GLOB HEADER_FILES "include/*.h")

add_executable(brush_app 
    ${APP_SOURCES}
    ${GLAD_SRC}
    ${IMGUI_SRC_ALL}
    ${STB_SRC_ALL}
    ${HEADER_FILES}
)

target_sources(brush_app PRIVATE ${SHADER_FILES})

target_link_libraries(brush_app PRIVATE
    glfw3
    gdi32
    user32
)
