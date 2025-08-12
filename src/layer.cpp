#include <format>
#include <stdexcept>
#include <string>
#include <utility>

#include "glad/glad.h"
#include "imgui.h"

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

static void attach_gpu_texture_to_program(GLuint gpu_texture, Program& program) {
    program.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gpu_texture);

    program.set_uniform_1i("u_texture", 0);
}

static GLuint generate_fbo(GLuint texture_id) {
    GLuint fbo_id = 0;
    glGenFramebuffers(1, &fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Framebuffer incomplete: status = " + std::to_string(status));
    }

    return fbo_id;
}

Layer::Layer(size_t width, size_t height)
    : m_quad_program("../src/shaders/quad.vert", "../src/shaders/quad.frag"),
    m_round_brush_program("../src/shaders/quad.vert", "../src/shaders/draw_circle.frag"),
    m_width(width),
    m_height(height)
{
    static Id current_id = 0;
    current_id++;

    m_id = current_id;
    m_name = std::format("Layer {}", current_id);

    m_is_visible = true;

    if (!GLAD_GL_ARB_sparse_texture) {
        throw std::runtime_error("GL_ARB_sparse_texture not supported");
    }

    m_gpu_texture = generate_gpu_texture(width, height, num_mip_levels, texture_format);
    m_fbo = generate_fbo(m_gpu_texture);

    glGetInternalformativ(GL_TEXTURE_2D, texture_format, GL_VIRTUAL_PAGE_SIZE_X_ARB, 1, &m_tile_width);
    glGetInternalformativ(GL_TEXTURE_2D, texture_format, GL_VIRTUAL_PAGE_SIZE_Y_ARB, 1, &m_tile_height);

    allocateAllTiles();

    attach_gpu_texture_to_program(m_gpu_texture, m_quad_program);
    attach_gpu_texture_to_program(m_gpu_texture, m_round_brush_program);
}

Layer::~Layer() {
    if (m_gpu_texture != 0) {
        glDeleteTextures(1, &m_gpu_texture);
    }
}

Layer::Layer(Layer&& other) noexcept
    : m_gpu_texture(other.m_gpu_texture),
    m_fbo(other.m_fbo),
    m_width(other.m_width),
    m_height(other.m_height),
    m_tile_width(other.m_tile_width),
    m_tile_height(other.m_tile_height),
    m_id(other.m_id),
    m_name(std::move(other.m_name)),
    m_is_visible(other.m_is_visible),
    m_quad_program(std::move(other.m_quad_program)),
    m_round_brush_program(std::move(other.m_round_brush_program))
{
    other.m_gpu_texture = 0;  
    other.m_fbo = 0;  
}

Layer& Layer::operator=(Layer&& other) noexcept {
    if (this != &other) {
        m_gpu_texture = other.m_gpu_texture;
        m_fbo = other.m_fbo;
        m_width = other.m_width;
        m_height = other.m_height;
        m_tile_width = other.m_tile_width;
        m_tile_height = other.m_tile_height;
        m_id = other.m_id;
        m_name = std::move(other.m_name);
        m_is_visible = other.m_is_visible;
        m_quad_program = other.m_quad_program;

        other.m_gpu_texture = 0; 
        other.m_fbo = 0; 
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

void Layer::allocateAllTiles() {
    int num_pages_x = (m_width + m_tile_width - 1) / m_tile_width;
    int num_pages_y = (m_height + m_tile_height - 1) / m_tile_height;

    for (size_t x = 0; x < num_pages_x; x++) {
        for (size_t y = 0; y < num_pages_y; y++) {
            allocateTile(TileCoords{ x, y });
        }
    }
}

void Layer::freeTile(TileCoords coords) {
    commitTile(coords, false);
}

// TODO: Move this functionality inside the ShaderProgram class
void Layer::set_round_brush_program_uniforms(ImVec2 pos, ImVec4 color, float radius) {
    m_round_brush_program.use();

    attach_gpu_texture_to_program(m_gpu_texture, m_round_brush_program);

    m_round_brush_program.set_uniform_2f("u_tex_dim", m_width, m_height);
    m_round_brush_program.set_uniform_2f("u_circle_pos", pos.x, pos.y);
    m_round_brush_program.set_uniform_1f("u_radius", radius);
    m_round_brush_program.set_uniform_4f("u_color", color.x, color.y, color.z, color.w);
}

GLuint Layer::get_dummy_vao() const {
    // OpenGL requires a VAO to be bound in order for the call not
    // to be discarded. We attach a dummy one, even though the
    // vertex data is hardcoded into the vertex shader. 
    static GLuint dummy_vao = 0;
    if (dummy_vao == 0) {
        glGenVertexArrays(1, &dummy_vao);
    }
    return dummy_vao;
}

void Layer::draw_circle(ImVec2 pos, ImVec4 color, float radius) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);

    m_round_brush_program.use();
    set_round_brush_program_uniforms(pos, color, radius);

    GLuint dummy_vao = get_dummy_vao();
    glBindVertexArray(dummy_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); 
}

void Layer::render() {
    if (!m_is_visible) return;

    m_quad_program.use();

    GLuint dummy_vao = get_dummy_vao();
    glBindVertexArray(dummy_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
