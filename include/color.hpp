#pragma once
#include <cstdint>
#undef Color

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

const Color BLACK = Color{0, 0, 0, 255};
const Color WHITE = Color{255, 255, 255, 255};
const Color RED = Color{255, 0, 0, 255};
const Color GREEN = Color{0, 255, 0, 255};
const Color BLUE = Color{0, 0, 255, 255};
const Color ERASE = Color{0, 0, 0, 0};