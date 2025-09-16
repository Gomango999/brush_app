#version 430 core

in vec2 v_tex_coord;

uniform sampler2D u_canvas;

out vec4 frag_color;

void main() {
    frag_color = texture(u_canvas, vec2(v_tex_coord.x, v_tex_coord.y));
}

