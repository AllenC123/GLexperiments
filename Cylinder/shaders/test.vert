#version 460

in vec3 vertex_coord;
//in vec3 vertex_color;
out vec3 color;

void main()
{
    gl_Position = vec4(vertex_coord, 1.0f);
    //color = vec3(0.0f, 1.0f, 1.0f);
    color = vec3(1.0f, 0.0f, 1.0f);
    //color = vertex_color;
    //color = vec3(vertex_color.r, 0.0f, vertex_color.b);
    
    //color = vec3(vertex_color.r, 0.0f, 1.0f);
    // vertex-colors are always zero when manually rendering the vertex-data
    // so nothing will appear unless you hardcode at least one postive value
}
