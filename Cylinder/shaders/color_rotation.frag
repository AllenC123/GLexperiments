#version 330

in  vec3 vertex_color;
out vec3 FragColor;

void main()
{
    // color-channel rotations
    vec3 RGB = vec3(vertex_color.r, vertex_color.g, vertex_color.b);
    vec3 BRG = vec3(vertex_color.b, vertex_color.r, vertex_color.g);
    vec3 GBR = vec3(vertex_color.g, vertex_color.b, vertex_color.r);
    FragColor = RGB;
}
