#version 330 core

in vec2 tc;
out vec4 FragColor;

uniform sampler2D g_canvas;

void main()
{
    FragColor = texture(g_canvas, tc);
} 