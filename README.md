# Brush App
![Sample Painted Illustration](assets/field_ui.jpg)
> _Sample Illustration: 8Kx8K, 17 layers. Original art by [Denis Istomin](https://www.artstation.com/artwork/wrboNw), recreated in the app._

A toy project to learn about implementing a paint app similar to Clip Studio Paint. 
Our long term goal is to be mostly feature complete, whilst using GPU rendering
to offer a smooth 60fps even for large image sizes (8K x 8K) and high layer 
counts (200 layers). Currently, we are able to handle 8K x 8K and ~20 layers smoothly.

## Features

- Pen and eraser 
	- Smooth(ish) brush interpolation
	- Brush size indicator  
- Color picker tool
- Layers
	- Visibility toggling 
	- Alpha locking
- Pen tablet support
	- Pen pressure support
- Zooming, panning, rotating and flipping

## Build
> _It is possible to build locally, but it'll require a bit of work. In future, I'd like
to make this process more easily accessible with `vcpkg`._

Install GLAD, GLFW, OpenGL, GLM and stb, and update `CmakeLists.txt` to point to these 
dependencies. Then fetch submodules to get the correct version of ImGui, and finally
build with `cmake`.

```sh
git submodule update --init --recursive
cmake -B build
cmake --build build
```