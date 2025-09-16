#pragma once

#include <functional>
#include <optional>
#include <vector>

#include <glad/glad.h>
#include <glm/fwd.hpp>

#include "canvas_view.h"
#include "frame_buffer.h"
#include "layer.h"
#include "program.h"
#include "texture.h"

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

	bool layer_exists(Layer::Id layer_id);
	std::optional<std::reference_wrapper<Layer>> lookup_layer(Layer::Id layer_id);

	Layer::Id insert_new_layer_above_selected(std::optional<Layer::Id> selected_layer);
	std::optional<Layer::Id> delete_selected_layer(std::optional<Layer::Id> selected_layer);
	void move_layer_up(std::optional<Layer::Id> layer_id);
	void move_layer_down(std::optional<Layer::Id> layer_id);
	bool get_layer_visibility(Layer::Id layer_id);
	void set_layer_visibility(Layer::Id layer_id, bool is_visible);
	bool get_layer_alpha_lock(Layer::Id layer_id);
	void set_layer_alpha_lock(Layer::Id layer_id, bool is_alpha_locked);

	// Should not need to call this. Only exposed for debugging purposes.
	glm::vec2 screen_space_to_canvas_space(glm::vec2 point) const { return m_canvas_view.screen_space_to_canvas_space(point); }

	// All arguments are given in screen space
	void zoom_into_point(glm::vec2 point, float zoom_factor) { m_canvas_view.zoom_into_point(point, zoom_factor); }
	void zoom_into_center(float zoom_factor) { m_canvas_view.zoom_into_center(zoom_factor); };
	void rotate(float delta_radians) { m_canvas_view.rotate(delta_radians); };
	void set_rotation(float radians) { m_canvas_view.set_rotation(radians); };
	float get_rotation() { return m_canvas_view.rotation(); };
	void move(glm::vec2 translation) { m_canvas_view.move(translation); };
	void flip() { m_canvas_view.flip(); };

	std::optional<glm::vec3> get_color_at_pos(glm::vec2 pos);

	void bind_fbo();
	void unbind_fbo() { FrameBuffer::unbind(); }
	void render(glm::vec2 screen_size, glm::vec2 mouse_pos);

	void save_as_png(const char* filename) const;


	size_t width() const { return m_output_frame_buffer.width(); }
	size_t height() const { return m_output_frame_buffer.height(); }
	glm::vec2 size() const { return glm::vec2(width(), height()); }
	glm::vec2 window_size() const { return m_canvas_view.size(); }

	const std::vector<Layer>& get_layers() const { return m_layers; }

	const Texture2D& output_texture() const { return m_output_frame_buffer.texture(); }
	const Texture2D& screen_texture() const { return m_canvas_view.get_view_texture(); }

private:
	void move_layer(std::optional<Layer::Id> layer_id, int delta);
};


