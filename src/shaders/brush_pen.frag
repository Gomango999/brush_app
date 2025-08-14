#version 430 core

in vec2 tex_coord;
out vec4 frag_color;

uniform sampler2D u_texture;
uniform vec2 u_tex_dim;
uniform vec2 u_circle_pos;
uniform float u_radius;
uniform vec3 u_color;
uniform float u_opacity;

void main() {
	vec4 base_color = texture(u_texture, tex_coord);

	vec2 pixel_pos = tex_coord * u_tex_dim;
	float dist = distance(pixel_pos, u_circle_pos);

	if (dist < u_radius) {
		vec3 new_color = mix(base_color.rgb, u_color, u_opacity);
		float new_alpha = base_color.a + (1.0 - base_color.a) * u_opacity;
		frag_color = vec4(new_color, new_alpha);
	} else {
		frag_color = base_color;	
	}
}