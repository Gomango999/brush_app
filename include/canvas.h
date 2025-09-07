#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "brush.h"
#include "canvas_view.h"
#include "frame_buffer.h"
#include "layer.h"
#include "program.h"
#include "texture.h"
#include "user_state.h"

// `Canvas` the canvas pixel data in both the CPU and GPU. It is
// responsible for updating both textures whenever something is
// drawn to the canvas.
class Canvas {
	glm::vec3 m_base_color;
	std::vector<Layer> m_layers;

	FrameBuffer m_output_frame_buffer;

	CanvasView m_canvas_view;

	Program m_cursor_program;

public:
	Canvas(size_t _width, size_t _height);
	~Canvas() = default;

	std::optional<std::reference_wrapper<Layer>> lookup_layer(Layer::Id layer_id);
	Layer::Id insert_new_layer_above_selected(std::optional<Layer::Id> selected_layer);
	std::optional<Layer::Id> delete_selected_layer(std::optional<Layer::Id> selected_layer);
	void move_layer_up(std::optional<Layer::Id> layer_id);
	void move_layer_down(std::optional<Layer::Id> layer_id);
	bool get_layer_visibility(Layer::Id layer_id);
	void set_layer_visibility(Layer::Id layer_id, bool is_visible);
	bool get_layer_alpha_lock(Layer::Id layer_id);
	void set_layer_alpha_lock(Layer::Id layer_id, bool is_alpha_locked);

	glm::vec2 screen_space_to_world_space(glm::vec2 point) const { return m_canvas_view.screen_space_to_world_space(point); }

	void zoom_into_point(glm::vec2 point) { m_canvas_view.zoom_into_point(point); }
	void zoom_out_of_point(glm::vec2 point) { m_canvas_view.zoom_out_of_point(point); }
	void rotate(float delta_radians) { m_canvas_view.rotate(delta_radians); };
	void move(glm::vec2 translation) { m_canvas_view.move(translation); };
	void flip() { m_canvas_view.flip(); };

	std::optional<glm::vec3> get_color_at_pos(glm::vec2 pos);

	void render(glm::vec2 screen_size, BrushManager& brush_manager, glm::vec2 mouse_pos);

	void save_as_png(const char* filename) const;

	size_t width() const { return m_output_frame_buffer.width(); }
	size_t height() const { return m_output_frame_buffer.height(); }
	glm::vec2 size() const { return glm::vec2(width(), height()); }

	bool layer_exists(Layer::Id layer_id);
	const std::vector<Layer>& get_layers() const { return m_layers; }

	const Texture2D& output_texture() const { return m_output_frame_buffer.texture(); }
	const Texture2D& screen_texture() const { return m_canvas_view.get_view_texture(); }

private:
	GLuint get_dummy_vao() const;
	void move_layer(std::optional<Layer::Id> layer_id, int delta);
	void render_cursor(BrushManager& brush_manager, glm::vec2 mouse_pos);
};


