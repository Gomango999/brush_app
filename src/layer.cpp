#include <chrono>
#include <format>
#include <stdexcept>

#include "glad/glad.h"

#include "layer.h"
#include "program.h"

const GLenum Layer::texture_format = GL_RGBA8;
const GLint Layer::num_mip_levels = 1;

static GLuint generate_gpu_texture(size_t width, size_t height, GLint num_mip_levels, GLenum texture_format) {
    GLuint texture_id = 0;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SPARSE_ARB, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_VIRTUAL_PAGE_SIZE_INDEX_ARB, 0);

    glTexStorage2D(GL_TEXTURE_2D, num_mip_levels, texture_format, width, height);
    return texture_id;
}

static void attach_gpu_texture_to_shader(GLuint gpu_texture, ShaderProgram shaders) {
    shaders.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gpu_texture);

    GLint loc = glGetUniformLocation(shaders.id(), "u_texture");
    glUniform1i(loc, 0);
}

Layer::Layer(size_t width, size_t height)
    // TODO: Find a more robust way to find these files. 
    : m_shaders("../src/shaders/quad_vert.glsl", "../src/shaders/quad_frag.glsl"),
    m_width(width),
    m_height(height)
{
    static Id current_id = 0;
    current_id++;

    m_id = current_id;
    m_name = std::format("Layer {}", current_id);

    m_is_visible = true;

    // TODO: Handle this exception
    if (!GLAD_GL_ARB_sparse_texture) {
        throw std::runtime_error("GL_ARB_sparse_texture not supported");
    }

    m_gpu_texture = generate_gpu_texture(width, height, num_mip_levels, texture_format);

    glGetInternalformativ(GL_TEXTURE_2D, texture_format, GL_VIRTUAL_PAGE_SIZE_X_ARB, 1, &m_tile_width);
    glGetInternalformativ(GL_TEXTURE_2D, texture_format, GL_VIRTUAL_PAGE_SIZE_Y_ARB, 1, &m_tile_height);
    
    attach_gpu_texture_to_shader(m_gpu_texture, m_shaders);
}

Layer::~Layer() {
    if (m_gpu_texture != 0) {
        glDeleteTextures(1, &m_gpu_texture);
    }
}

Layer::Layer(Layer&& other) noexcept
    : m_gpu_texture(other.m_gpu_texture),
    m_width(other.m_width),
    m_height(other.m_height),
    m_tile_width(other.m_tile_width),
    m_tile_height(other.m_tile_height),
    m_id(other.m_id),
    m_name(std::move(other.m_name)),
    m_is_visible(other.m_is_visible),
    m_shaders("../src/shaders/quad_vert.glsl", "../src/shaders/quad_frag.glsl")
{
    other.m_gpu_texture = 0;  
}

Layer& Layer::operator=(Layer&& other) noexcept {
    if (this != &other) {
        m_gpu_texture = other.m_gpu_texture;
        m_width = other.m_width;
        m_height = other.m_height;
        m_tile_width = other.m_tile_width;
        m_tile_height = other.m_tile_height;
        m_id = other.m_id;
        m_name = std::move(other.m_name);
        m_is_visible = other.m_is_visible;

        other.m_gpu_texture = 0; // prevent double delete
    }
    return *this;
}

Layer::TileCoords Layer::calculate_tile_coords_from_pixel_coords(size_t x, size_t y) {
    return TileCoords{ x / m_tile_width, y / m_tile_height };
}

void Layer::commitTile(TileCoords coords, bool commit) {
    size_t x_offset = coords.x * m_tile_width;
    size_t y_offset = coords.y * m_tile_height;

    glBindTexture(GL_TEXTURE_2D, m_gpu_texture);
    glTexPageCommitmentARB(
        GL_TEXTURE_2D,
        0,
        x_offset, y_offset, 0,
        m_tile_width, m_tile_height, 1,
        commit
    );
}

void Layer::allocateTile(TileCoords coords) {
    commitTile(coords, true);
}

void Layer::freeTile(TileCoords coords) {
    commitTile(coords, false);
}

void Layer::unpack_imvec4_color(ImVec4 color, GLubyte out[4]) {
    ImU32 packed = (ImU32)(ImColor)color;

    out[0] = (packed >> IM_COL32_R_SHIFT) & 0xFF;
    out[1] = (packed >> IM_COL32_G_SHIFT) & 0xFF;
    out[2] = (packed >> IM_COL32_B_SHIFT) & 0xFF;
    out[3] = (packed >> IM_COL32_A_SHIFT) & 0xFF;
}

void Layer::write_pixel_on_allocated_tile(size_t x, size_t y, ImVec4 color) {
    GLubyte pixel_color[4];
    unpack_imvec4_color(color, pixel_color);

    glBindTexture(GL_TEXTURE_2D, m_gpu_texture);
    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        x, y,
        1, 1,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixel_color
    );
}

void Layer::write_pixel(size_t x, size_t y, ImVec4 color) {
    TileCoords tile_coords = calculate_tile_coords_from_pixel_coords(x, y);
    allocateTile(tile_coords);
    write_pixel_on_allocated_tile(x, y, color);

    // TODO: In future, we'll need to deallocate tiles.
    // Running a compute shader at the end of a brush stroke, then
    // deallocating the necessary tiles seems like a good strategy.
}

void Layer::render() {
    if (!m_is_visible) return;

    // OpenGL requires a VAO to be bound in order for the call not
    // to be discarded. We attach a dummy one, even though the
    // vertex data is hardcoded into the vertex shader. 
    static GLuint dummy_vao = 0;
    if (dummy_vao == 0) {
        glGenVertexArrays(1, &dummy_vao);
    }

    m_shaders.use();

    glBindVertexArray(dummy_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
