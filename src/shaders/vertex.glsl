#version 330 core
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_tc;

out vec2 tc;

void main()
{
    gl_Position = vec4(in_pos, 1.0);
    tc = in_tc;
}