#pragma once
#include <cstddef>

struct BoundingBox {
    size_t top;
    size_t bottom;
    size_t left;
    size_t right;

    size_t width() const {
        return right - left;
    }

    size_t height() const {
        return bottom - top;
    }

    size_t area() const {
        return width() * height();
    }
};