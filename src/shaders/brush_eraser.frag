#version 430 core

in vec2 tex_coord;
out vec4 frag_color;

uniform vec2 u_tex_dim;
uniform vec2 u_circle_pos;
uniform float u_radius;
uniform float u_opacity;

void main() {
	vec2 pixel_pos = tex_coord * u_tex_dim;
	float dist = distance(pixel_pos, u_circle_pos);

	// The blend mode should ignore the color and only modify
	// the opacity
	vec3 ignored_color = vec3(0., 0., 0.);
	if (dist < u_radius) {
		frag_color = vec4(ignored_color, u_opacity);
	} else {
		frag_color = vec4(ignored_color, 0.);	
	}
}