#pragma once

#include <glm/glm.hpp>

#include "frame_buffer.h"
#include "program.h"
#include "texture.h"

class CanvasView {
	float m_scale;
	float m_rotation; // stored in radians
	glm::vec2 m_translation;

	size_t m_canvas_width, m_canvas_height;

	FrameBuffer m_frame_buffer;
	Program m_program;

public:
	CanvasView(size_t canvas_width, size_t canvas_height);

	glm::vec2 translation() const { return m_translation; }
	float scale() const { return m_scale; }
	float rotation() const { return m_rotation; }
	glm::mat3 get_transform() const;

	glm::vec2 screen_space_to_world_space(glm::vec2 point) const;

	void zoom_into_point(glm::vec2 point);
	void zoom_out_of_point(glm::vec2 point);
	void rotate(float delta_radians);
	void move(glm::vec2 translation); // move in screen space
	void flip();

	void render(glm::vec2 screen_size, const Texture2D& canvas);



	size_t width() const { return m_frame_buffer.width(); }
	size_t height() const { return m_frame_buffer.height(); }
	const Texture2D& get_view_texture() const { return m_frame_buffer.texture(); }

private:
	glm::mat3 aspect_ratio_mat() const;
};