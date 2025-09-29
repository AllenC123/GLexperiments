#version 460

in vec3 vertex_coord;
in vec3 vertex_color;
out vec3 color;

void main()
{
    gl_Position = vec4(vertex_coord, 1.0f);
    color = vertex_color;
}
