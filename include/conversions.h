#pragma once

#include <glm/glm.hpp>
#include <imgui.h>

inline glm::vec2 to_glm(const ImVec2& v) {
    return glm::vec2(v.x, v.y);
}

inline ImVec2 to_imvec(const glm::vec2& v) {
    return ImVec2(v.x, v.y);
}
