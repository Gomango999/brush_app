#pragma once
#include <optional>

#include <glm/glm.hpp>

#include "frame_buffer.h"
#include "program.h"
#include "texture.h"

class CanvasView {
	bool m_flipped;
	glm::vec2 m_scale;
	float m_rotation; // stored in radians
	glm::vec2 m_translation; // stored in NDC space 

	size_t m_canvas_width, m_canvas_height;

	FrameBuffer m_frame_buffer;
	Program m_program;

public:
	CanvasView(size_t canvas_width, size_t canvas_height);

	glm::vec2 translation() const { return m_translation; }
	bool is_flipped() { return m_flipped; }
	glm::vec2 scale() const { return m_scale; }
	float rotation() const { return m_rotation; }
	glm::mat3 get_transform() const;

	glm::vec2 screen_space_to_canvas_space(glm::vec2 point) const;

	// All arguments are given in screen space
	void zoom_into_point(glm::vec2 point, float zoom_factor);
	void zoom_into_center(float zoom_factor) { zoom_into_point(size() * 0.5f, zoom_factor); };
	void rotate(float delta_radians);
	void set_rotation(float radians);
	void move(glm::vec2 translation); 
	void flip();

	void render(glm::vec2 screen_size, const Texture2D& canvas);

	size_t width() const { return m_frame_buffer.width(); }
	size_t height() const { return m_frame_buffer.height(); }
	glm::vec2 canvas_size() const { return glm::vec2(m_canvas_width, m_canvas_height); }
	glm::vec2 size() const { return m_frame_buffer.size(); }
	const Texture2D& get_view_texture() const { return m_frame_buffer.texture(); }
};