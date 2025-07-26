#pragma once

#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <glad/glad.h>
#include <imgui.h>

#include "bounding_box.h"

struct Layer {
	typedef unsigned int Id;

	Id id;
	std::string name;
	bool is_visible;
	size_t height;

	Layer(size_t _height) {
		static Id next_id = 1;
		id = next_id;
		name = std::format("Layer {}", next_id);
		next_id++;
		is_visible = true;
		height = _height;
	}
};

struct UserState {
	ImVec4 color;
	float radius;
	float opacity;

	std::optional<Layer::Id> selected_layer;

	UserState() {
		color = ImVec4(0.0, 0.0, 0.0, 1.0);
		radius = 200.0;
		opacity = 1.0;

		selected_layer = std::nullopt;
	};
};

typedef std::pair<ImVec4, Layer::Id> PixelInLayer;
typedef std::vector<PixelInLayer> PixelStack;
typedef std::vector<std::vector<PixelStack>> LayerStack;

// `Canvas` the canvas pixel data in both the CPU and GPU. It is
// responsible for updating both textures whenever something is
// drawn to the canvas.
class Canvas {
private:
	size_t m_width, m_height;

	// TODO: Encapsulate this into their own object, so that we can 
	// better guarantee that these two variables are synced.
	std::vector<Layer> m_layers;
	std::unordered_map<Layer::Id, std::vector<Layer>::iterator> m_lookup_layer_by_id;

	ImVec4 m_base_color;
	LayerStack m_layer_stack;
	std::vector<uint8_t> m_output_image;
	GLuint m_gpu_texture;

	UserState m_user_state;

public:
	Canvas(size_t _width, size_t _height);

	Layer::Id insert_new_layer_above_selected();
	void delete_selected_layer();
	bool get_layer_visibility(Layer::Id layer_id);
	void set_layer_visibility(Layer::Id layer_id, bool is_visible);

	void draw_circle_at_pos(ImVec2 pos);

	size_t width() const;
	size_t height() const;
	UserState& user_state();
	const std::vector<Layer>& get_layers();
	GLuint get_gpu_texture() const;

private:
	BoundingBox fill_circle_in_layer(
		int center_x,
		int center_y,
		unsigned int radius,
		Layer::Id layer,
		ImVec4 color
	);
	void set_pixel_in_layer(size_t x, size_t y, Layer::Id layer_id, ImVec4 color);
	ImVec4 calculate_output_pixel_color(size_t x, size_t y);
	void set_pixel_in_output_image(size_t x, size_t y, ImVec4 color);
	void update_full_output_image();
	void update_output_image_within_bbox(BoundingBox bbox);
	void upload_full_pixel_data_to_gpu();
	void upload_pixel_data_within_bbox_to_gpu(BoundingBox bbox);
};


