#version 430 core

in vec2 TexCoord;
uniform sampler2D u_texture;
out vec4 FragColor;

void main() {
    // FragColor = texture(u_texture, TexCoord);
    FragColor = vec4(0.0, 1.0, 1.0, 1.0);
}
