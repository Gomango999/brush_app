#pragma once
#include <optional>

#include "brush.h"
#include "layer.h"
#include "vec.h"

struct CursorState {
    Vec2 pos;
    float pressure;
    CursorState() {
        pos = Vec2{ 0.0, 0.0 };
        pressure = 0.0;
    }
    CursorState(Vec2 _pos, float _pressure) {
        pos = _pos;
        pressure = _pressure;
    }
};

// UserState is used to inform how to draw to the screen.
// It is modified and displayed within the GUI.
struct UserState {
    BrushManager brush_manager;
    std::optional<Layer::Id> selected_layer;
    Vec3 selected_color;

    CursorState cursor;
    std::optional<CursorState> prev_cursor;

    UserState() : brush_manager() {
        selected_color = Vec3{ 0.0, 0.0, 0.0 };
        selected_layer = std::nullopt;
        cursor = CursorState();
        prev_cursor = std::nullopt;
    };
};
