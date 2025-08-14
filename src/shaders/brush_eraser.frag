#version 430 core

in vec2 tex_coord;
out vec4 frag_color;

uniform sampler2D u_texture;
uniform vec2 u_tex_dim;
uniform vec2 u_circle_pos;
uniform float u_radius;
uniform float u_opacity;

void main() {
	vec4 base_color = texture(u_texture, tex_coord);

	vec2 pixel_pos = tex_coord * u_tex_dim;
	float dist = distance(pixel_pos, u_circle_pos);

	if (dist < u_radius) {
		float new_alpha = base_color.a * (1.0 - u_opacity);
		frag_color = vec4(base_color.rgb, new_alpha);
	} else {
		frag_color = base_color;	
	}
}