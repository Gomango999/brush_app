#pragma once

#include <glm/glm.hpp>

class CanvasView {
	float m_scale;
	float m_rotation; // stored in radians
	glm::vec2 m_translation;
	
	size_t m_width, m_height;

public:
	glm::vec2 translation() const { return m_translation; }
	float scale() const { return m_scale; }
	float rotation() const { return m_rotation; }
	glm::mat3 get_transform() const;

	void zoom_into_point(glm::vec2 point);
	void zoom_out_of_point(glm::vec2 point);

	void rotate(float delta_radians);

	void move(glm::vec2 translation); // move in screen space

	void flip();

	size_t width() const { return m_width; };
	size_t height() const { return m_height; };
	void set_width(size_t width) { m_width = width; };
	void set_height(size_t height) { m_height = height; };

private:
	glm::mat3 translation_mat() const;
	glm::mat3 scale_mat() const;
	glm::mat3 rotation_mat() const;
};