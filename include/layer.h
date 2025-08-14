#pragma once
#include <string>

#include "glad/glad.h"

#include "brush.h"
#include "program.h"
#include "vec.h"

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
    bool m_is_alpha_locked;

    static const GLenum texture_format;
    static const GLint num_mip_levels;

    size_t m_width, m_height;
    GLint m_tile_width, m_tile_height;
    GLuint m_gpu_texture;
    GLuint m_fbo;

    Program m_quad_program;

    TileCoords calculate_tile_coords_from_pixel_coords(size_t x, size_t y);
    void commitTile(TileCoords coords, bool commit);
    void allocateTile(TileCoords coords);
    void allocateAllTiles();
    void freeTile(TileCoords coords);

    GLuint get_dummy_vao() const;


public:
    Layer(size_t width, size_t height);
    ~Layer();
    Layer(const Layer&) = delete;
    Layer& operator=(const Layer&) = delete;
    Layer(Layer&& other) noexcept;
    Layer& operator=(Layer&& other) noexcept;

    void draw_with_brush(Brush& brush, Vec2 mouse_pos, Vec3 color);
    void render();

    Id id() const { return m_id; }

    const std::string& name() const { return m_name; }
    void set_name(const std::string& name) { m_name = name; }

    bool is_visible() const { return m_is_visible; }
    void set_visible(bool visible) { m_is_visible = visible; }

    bool is_alpha_locked() const { return m_is_alpha_locked; }
    void set_alpha_lock(bool locked) { m_is_alpha_locked = locked; }

    GLuint gpu_texture() const { return m_gpu_texture; }
};
