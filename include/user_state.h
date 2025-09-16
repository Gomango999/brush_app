#pragma once
#include <optional>

#include <glm/fwd.hpp>

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
// It's fields are modified and displayed within the GUI.
struct UserState {
    std::optional<Layer::Id> selected_layer;
    glm::vec3 selected_color;

    CursorState cursor;
    std::optional<CursorState> prev_cursor;
    bool shift_down;
    bool is_using_temp_tool;

    UserState() {
        selected_color = glm::vec3( 0.0, 0.0, 0.0 );
        selected_layer = std::nullopt;
        cursor = CursorState();
        prev_cursor = std::nullopt;
        shift_down = false;
        is_using_temp_tool = false;
    };
};
