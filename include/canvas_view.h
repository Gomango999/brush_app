#pragma once

#include <glm/glm.hpp>

#include "frame_buffer.h"
#include "program.h"
#include "texture.h"

class CanvasView {
	float m_scale;
	float m_rotation; // stored in radians
	glm::vec2 m_translation;

	Texture2D m_texture;
	FrameBuffer m_frame_buffer;
	Program m_program;

public:
	CanvasView(size_t window_width, size_t window_height);

	glm::vec2 translation() const { return m_translation; }
	float scale() const { return m_scale; }
	float rotation() const { return m_rotation; }
	glm::mat3 get_transform() const;

	void zoom_into_point(glm::vec2 point);
	void zoom_out_of_point(glm::vec2 point);

	void rotate(float delta_radians);

	void move(glm::vec2 translation); // move in screen space

	void flip();

	void render(const Texture2D& canvas);

	size_t width() const { return m_texture.width(); }
	size_t height() const { return m_texture.height(); }
	void resize(size_t width, size_t height);
	const Texture2D& get_view_texture() const { return m_texture; }

private:
	glm::mat3 translation_mat() const;
	glm::mat3 scale_mat() const;
	glm::mat3 rotation_mat() const;
};