#version 330

in vec3 color;
out vec4 FragColor;

// swaps red and blue
void main() { FragColor = vec4(color.b, color.g, color.r, 1.0f); }
