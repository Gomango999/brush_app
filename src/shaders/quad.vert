#version 430 core

out vec2 tex_coord;

const vec2 verts[4] = vec2[](
    vec2(-1, -1),
    vec2(1, -1),
    vec2(-1, 1),
    vec2(1, 1)
);

void main() {
    gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);
    tex_coord = (verts[gl_VertexID] + 1.0) / 2.0;
}
