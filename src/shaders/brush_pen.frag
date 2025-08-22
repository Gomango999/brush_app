#version 430 core

in vec2 tex_coord;
out vec4 frag_color;

uniform vec2 u_tex_dim;
uniform vec2 u_circle_pos;
uniform float u_radius;
uniform vec3 u_color;
uniform float u_opacity;

void main() {
	vec2 pixel_pos = tex_coord * u_tex_dim;
	float dist = distance(pixel_pos, u_circle_pos);

	if (dist < u_radius) {
		frag_color = vec4(u_color, u_opacity);
	} else {
		frag_color = vec4(0., 0., 0., 0.);	
	}
}