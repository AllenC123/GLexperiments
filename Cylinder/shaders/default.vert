#version 330

in  vec3 vertex_coord;
out vec3 vertex_color;

void main()
{
    gl_Position = vec4(vertex_coord, 1.0f);
    vertex_color = vec3(abs(vertex_coord[0]), abs(vertex_coord[1]), abs(vertex_coord[2]));
}
