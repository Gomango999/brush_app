#version 330 core

in vec2 tc;
out vec4 FragColor;

uniform sampler2D canvas_texture;

void main()
{
    FragColor = texture(canvas_texture, tc);
} 