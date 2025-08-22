#pragma once
#include <optional>

#include "brush.h"
#include "layer.h"

struct CursorState {
    glm::vec2 pos;
    float pressure;
    CursorState() {
        pos = glm::vec2( 0.0, 0.0 );
        pressure = 0.0;
    }
    CursorState(glm::vec2 _pos, float _pressure) {
        pos = _pos;
        pressure = _pressure;
    }
};

// UserState is used to inform how to draw to the screen.
// It is modified and displayed within the GUI.
struct UserState {
    BrushManager brush_manager;
    std::optional<Layer::Id> selected_layer;
    glm::vec3 selected_color;

    CursorState cursor;
    std::optional<CursorState> prev_cursor;

    bool is_color_picking;

    UserState() : brush_manager() {
        selected_color = glm::vec3( 0.0, 0.0, 0.0 );
        selected_layer = std::nullopt;
        cursor = CursorState();
        prev_cursor = std::nullopt;
        is_color_picking = false;
    };
};
