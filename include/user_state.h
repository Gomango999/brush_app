#pragma once
#include <optional>

#include "brush.h"
#include "layer.h"
#include "vec.h"

// UserState is used to inform how to draw to the screen.
// It is modified and displayed within the GUI.
struct UserState {
    BrushManager brush_manager;
    std::optional<Layer::Id> selected_layer;
    Vec3 selected_color;

    Vec2 mouse_pos;
    std::optional<Vec2> prev_mouse_pressed_pos;

    UserState() : brush_manager() {
        selected_color = Vec3{ 0.0, 0.0, 0.0 };
        selected_layer = std::nullopt;
        mouse_pos = Vec2{ 0.0, 0.0 };
        prev_mouse_pressed_pos = std::nullopt;
    };
};
