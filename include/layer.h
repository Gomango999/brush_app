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
    GLuint m_fbo;

    ShaderProgram m_quad_program;
    ShaderProgram m_round_brush_program;
    void set_round_brush_program_uniforms(ImVec2 pos, ImVec4 color, float radius);

    TileCoords calculate_tile_coords_from_pixel_coords(size_t x, size_t y);
    void commitTile(TileCoords coords, bool commit);
    void allocateTile(TileCoords coords);
    void allocateAllTiles();
    void freeTile(TileCoords coords);

public:
    Layer(size_t width, size_t height);
    ~Layer();
    Layer(const Layer&) = delete;
    Layer& operator=(const Layer&) = delete;
    Layer(Layer&& other) noexcept;
    Layer& operator=(Layer&& other) noexcept;

    void draw_circle(ImVec2 pos, ImVec4 color, float radius);
    void render();

    Id id() const { return m_id; }

    const std::string& name() const { return m_name; }
    void set_name(const std::string& name) { m_name = name; }

    bool is_visible() const { return m_is_visible; }
    void set_visible(bool visible) { m_is_visible = visible; }

    GLuint gpu_texture() const { return m_gpu_texture; }
};
