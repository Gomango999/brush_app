@echo off
setlocal

:: Paths
set INCLUDES=-IC:\cpp_libs\include ^
    -I.\external\imgui ^
    -I.\external\imgui\backends ^
    -I.\include
set LIBS=-LC:\cpp_libs\lib -lglfw3 -lgdi32

:: Source files and output
set SOURCES=src\main.cpp ^
    src\app.cpp ^
    src\canvas.cpp ^
    C:\cpp_libs\src\glad\glad.c ^
    external\imgui\*.cpp ^
    external\imgui\backends\imgui_impl_glfw.cpp ^
    external\imgui\backends\imgui_impl_opengl3.cpp
set OUTFILE=a.exe

echo Compiling %OUTFILE%...
g++ %SOURCES% -o %OUTFILE% %INCLUDES% -std=c++17 %LIBS%

endlocal