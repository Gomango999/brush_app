#version 430 core

uniform mat3 u_transform;

out vec2 v_tex_coord;

void main() {
    const vec2 positions[6] = vec2[](
        vec2(-1.0, -1.0), // bottom-left
        vec2( 1.0, -1.0), // bottom-right
        vec2( 1.0,  1.0), // top-right

        vec2(-1.0, -1.0), // bottom-left
        vec2( 1.0,  1.0), // top-right
        vec2(-1.0,  1.0)  // top-left
    );

    const vec2 tex_coords[6] = vec2[](
        vec2(0.0, 0.0),
        vec2(1.0, 0.0),
        vec2(1.0, 1.0),

        vec2(0.0, 0.0),
        vec2(1.0, 1.0),
        vec2(0.0, 1.0)
    );

    vec2 pos = positions[gl_VertexID];
    vec2 tex = tex_coords[gl_VertexID];

    vec3 transformed = u_transform * vec3(pos, 1.0);

    gl_Position = vec4(transformed.xy, 0.0, 1.0);
    v_tex_coord = tex;
}
