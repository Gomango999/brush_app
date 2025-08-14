#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include <glad/glad.h>
#include "imgui.h"

#include "brush.h"
#include "layer.h"
#include "program.h"


// TODO: Should this really be stored as part of the canvas?
// I suspect it's an app thing that should get passed around.
struct UserState {

	BrushManager brush_manager;
	std::optional<Layer::Id> selected_layer;
	ImVec4 color;

	ImVec2 mouse_pos;

	UserState() : brush_manager() {
		color = ImVec4(0.0, 0.0, 0.0, 1.0);
		selected_layer = std::nullopt;
		mouse_pos = ImVec2(0.0, 0.0);
	};
};

// `Canvas` the canvas pixel data in both the CPU and GPU. It is
// responsible for updating both textures whenever something is
// drawn to the canvas.
class Canvas {
private:
	size_t m_width, m_height;

	ImVec4 m_base_color;
	std::vector<Layer> m_layers;
	GLuint m_output_fbo;
	GLuint m_output_texture;

	UserState m_user_state;

	Program m_cursor_program;

	GLuint get_dummy_vao() const;
	void render_cursor();

public:
	Canvas(size_t _width, size_t _height);
	~Canvas();

	Layer::Id insert_new_layer_above_selected();
	void delete_selected_layer();
	bool get_layer_visibility(Layer::Id layer_id);
	void set_layer_visibility(Layer::Id layer_id, bool is_visible);

	void draw_circle_at_pos(ImVec2 pos);
	void draw_circles_on_segment(ImVec2 start, ImVec2 end, bool draw_start, unsigned int num_segments = 8);

	void render_output_image();

	size_t width() const;
	size_t height() const;
	UserState& user_state();
	const std::vector<Layer>& get_layers() const;
	GLuint output_texture() const;

private:
	std::optional<std::reference_wrapper<Layer>> lookup_layer(Layer::Id layer_id);
};


