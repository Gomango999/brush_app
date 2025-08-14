#pragma once

#include <functional>
#include <optional>
#include <vector>

#include <glad/glad.h>

#include "brush.h"
#include "layer.h"
#include "program.h"
#include "user_state.h"
#include "vec.h"

// `Canvas` the canvas pixel data in both the CPU and GPU. It is
// responsible for updating both textures whenever something is
// drawn to the canvas.
class Canvas {
public:
	Canvas(size_t _width, size_t _height);
	~Canvas();

	Layer::Id insert_new_layer_above_selected(std::optional<Layer::Id> selected_layer);
	std::optional<Layer::Id> delete_selected_layer(std::optional<Layer::Id> selected_layer);

	void move_layer_up(std::optional<Layer::Id> layer_id);
	void move_layer_down(std::optional<Layer::Id> layer_id);

	bool get_layer_visibility(Layer::Id layer_id);
	void set_layer_visibility(Layer::Id layer_id, bool is_visible);
	bool get_layer_alpha_lock(Layer::Id layer_id);
	void set_layer_alpha_lock(Layer::Id layer_id, bool is_alpha_locked);

	std::optional<std::reference_wrapper<Layer>> lookup_layer(Layer::Id layer_id);


	void draw_circle_at_pos(Layer& layer, Brush& brush, CursorState cursor, Vec3 color);
	void draw_circles_on_segment(
		Layer& layer, Brush& brush, 
		CursorState start, CursorState end, 
		Vec3 color, 
		bool include_start = false, 
		unsigned int num_segments = 8
	);

	void render(BrushManager& brush_manager, Vec2 mouse_pos);

	size_t width() const;
	size_t height() const;
	const std::vector<Layer>& get_layers() const;
	GLuint output_texture() const;

private:
	size_t m_width, m_height;

	Vec3 m_base_color;
	std::vector<Layer> m_layers;
	GLuint m_output_fbo;
	GLuint m_output_texture;

	Program m_cursor_program;

	GLuint get_dummy_vao() const;

	void move_layer(std::optional<Layer::Id> layer_id, int delta);

	void render_cursor(BrushManager& brush_manager, Vec2 mouse_pos);
};


