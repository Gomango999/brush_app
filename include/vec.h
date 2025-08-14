#pragma once
#include <initializer_list>

#include "imgui.h"

template <size_t N>
struct Vec {
    float data[N];

    Vec() {
        for (size_t i = 0; i < N; ++i) data[i] = 0.0f;
    }
    explicit Vec(float val) {
        for (size_t i = 0; i < N; ++i) data[i] = val;
    }
    Vec(std::initializer_list<float> vals) {
        size_t i = 0;
        for (float v : vals) {
            if (i < N) data[i++] = v;
        }
        for (; i < N; ++i) data[i] = 0.0f;
    }

    float& operator[](size_t i) { return data[i]; }
    const float& operator[](size_t i) const { return data[i]; }

    Vec operator+(const Vec& other) const {
        Vec r;
        for (size_t i = 0; i < N; ++i) r[i] = data[i] + other[i];
        return r;
    }
    Vec operator-(const Vec& other) const {
        Vec r;
        for (size_t i = 0; i < N; ++i) r[i] = data[i] - other[i];
        return r;
    }
    Vec& operator+=(const Vec& other) {
        for (size_t i = 0; i < N; ++i) data[i] += other[i];
        return *this;
    }
    Vec& operator-=(const Vec& other) {
        for (size_t i = 0; i < N; ++i) data[i] -= other[i];
        return *this;
    }

    Vec operator+(float s) const {
        Vec r;
        for (size_t i = 0; i < N; ++i) r[i] = data[i] + s;
        return r;
    }
    Vec operator-(float s) const {
        Vec r;
        for (size_t i = 0; i < N; ++i) r[i] = data[i] - s;
        return r;
    }
    Vec operator*(float s) const {
        Vec r;
        for (size_t i = 0; i < N; ++i) r[i] = data[i] * s;
        return r;
    }
    Vec operator/(float s) const {
        Vec r;
        for (size_t i = 0; i < N; ++i) r[i] = data[i] / s;
        return r;
    }
    Vec& operator+=(float s) {
        for (size_t i = 0; i < N; ++i) data[i] += s;
        return *this;
    }
    Vec& operator-=(float s) {
        for (size_t i = 0; i < N; ++i) data[i] -= s;
        return *this;
    }
    Vec& operator*=(float s) {
        for (size_t i = 0; i < N; ++i) data[i] *= s;
        return *this;
    }
    Vec& operator/=(float s) {
        for (size_t i = 0; i < N; ++i) data[i] /= s;
        return *this;
    }

    float& x() { return data[0]; }
    float& y() { return data[1]; }
    float& z() { return data[2]; }
    float& w() { return data[3]; }
    Vec<2> xy() const { 
        static_assert(N >= 3, "xy() requires at least 2 components");
        return { data[0], data[1] }; 
    }
    Vec<3> xyz() const { 
        static_assert(N >= 3, "xyz() requires at least 3 components");
        return { data[0], data[1], data[2] }; 
    }
    Vec<4> xyzw() const { return *this; }

    float& r() { return data[0]; }
    float& g() { return data[1]; }
    float& b() { return data[2]; }
    float& a() { return data[3]; }
    Vec<3> rgb() const {
        static_assert(N >= 3, "rgb() requires at least 3 components");
        return Vec<3>({ data[0], data[1], data[2] });
    }
    Vec<4> rgba() const {
        static_assert(N >= 4, "rgba() requires at least 4 components");
        return Vec<4>({ data[0], data[1], data[2], data[3] });
    }

    ImVec2 to_ImVec2() const { return ImVec2(data[0], data[1]); }
    ImVec4 to_ImVec4() const {
        static_assert(N >= 4, "to_ImVec4() requires at least 4 components");
        if constexpr (N >= 4) return ImVec4(data[0], data[1], data[2], data[3]);
    }
    static Vec<2> from_ImVec2(const ImVec2& v) { return Vec<2>({ v.x, v.y }); }
    static Vec<4> from_ImVec4(const ImVec4& v) { return Vec<4>({ v.x, v.y, v.z, v.w }); }
};

using Vec2 = Vec<2>;
using Vec3 = Vec<3>;
using Vec4 = Vec<4>;