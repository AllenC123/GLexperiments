#version 330

in vec3 color;
out vec4 FragColor;

// swaps green and blue
void main() { FragColor = vec4(color.r, color.b, color.g, 1.0f); }
