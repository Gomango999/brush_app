#version 430 core

in vec2 tex_coord;
out vec4 frag_color;

uniform sampler2D u_texture;
uniform vec2 u_tex_dim;
uniform vec2 u_circle_pos;
uniform float u_radius;
uniform vec4 u_color;

void main() {
	vec4 base_color = texture(u_texture, tex_coord);

	vec2 pixel_pos = tex_coord * u_tex_dim;
	float dist = distance(pixel_pos, u_circle_pos);

	if (dist < u_radius) {
		frag_color = mix(base_color, u_color, u_color.a);
	} else {
		frag_color = base_color;	
	}
}