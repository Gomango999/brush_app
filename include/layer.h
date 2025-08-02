#pragma once
#include <string>

#include "glad/glad.h"
#include "imgui.h"

#include "program.h"

class Layer {
public:
    typedef unsigned int Id;

private:
    struct TileCoords {
        size_t x, y;
    };

    Id m_id;
    std::string m_name;
    bool m_is_visible;

    static const GLenum texture_format;
    static const GLint num_mip_levels;

    size_t m_width, m_height;
    GLint m_tile_width, m_tile_height;
    GLuint m_gpu_texture;
    ShaderProgram m_shaders;

    TileCoords calculate_tile_coords_from_pixel_coords(size_t x, size_t y);
    void commitTile(TileCoords coords, bool commit);
    void allocateTile(TileCoords coords);
    void freeTile(TileCoords coords);

    void unpack_imvec4_color(ImVec4 color, GLubyte out[4]);
    void write_pixel_on_allocated_tile(size_t x, size_t y, ImVec4 color);

public:
    Layer(size_t width, size_t height);
    ~Layer();
    Layer(const Layer&) = delete;
    Layer& operator=(const Layer&) = delete;
    Layer(Layer&& other) noexcept;
    Layer& operator=(Layer&& other) noexcept;

    void write_pixel(size_t x, size_t y, ImVec4 color);
    void render();

    Id id() const { return m_id; }

    const std::string& name() const { return m_name; }
    void set_name(const std::string& name) { m_name = name; }

    bool is_visible() const { return m_is_visible; }
    void set_visible(bool visible) { m_is_visible = visible; }

    GLuint gpu_texture() const { return m_gpu_texture; }
};
