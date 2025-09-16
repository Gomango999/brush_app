#version 430 core

in vec2 tex_coord;
out vec4 frag_color;

uniform sampler2D u_texture;
uniform vec2 u_tex_dim;
uniform vec2 u_mouse_pos;
uniform float u_radius;

void main() {
	vec2 mouse_pos = vec2(u_mouse_pos.x, u_tex_dim.y - u_mouse_pos.y);

	vec4 base_color = texture(u_texture, tex_coord);

	vec2 pixel_pos = tex_coord * u_tex_dim;
	float dist = distance(pixel_pos, mouse_pos);

	float thickness = 1.0f;
	float alpha = smoothstep(u_radius + thickness * 0.5, u_radius + thickness * 0.5 - 1.0, dist) -
                  smoothstep(u_radius - thickness * 0.5, u_radius - thickness * 0.5 - 1.0, dist);
	alpha *= 0.5;

	vec4 negative = vec4(1.0 - base_color.rgb, 1.0);
	frag_color = mix(base_color, negative, alpha);
}