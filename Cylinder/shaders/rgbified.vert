#version 330

uniform uint clock; // milliseconds
in  vec3 vertex_coord;
out vec3 vertex_color;

void main()
{
    float time = (clock*0.0125f);
    gl_Position = vec4(vertex_coord, 1.0f);
    vec3 T = vec3(time, (time + ((2.0f*3.14f)/3.0f)), (time + ((4.0f*3.14f)/3.0f)));
    vec3 V = vec3(abs(vertex_coord[0]), abs(vertex_coord[1]), abs(vertex_coord[2]));
    vertex_color = vec3(
        (V.x*sin(T[0])+cos(T[0])) + (V.y*sin(T[1])+cos(T[1])) + (V.z*sin(T[2])+cos(T[2])),
        (V.y*sin(T[0])+cos(T[0])) + (V.z*sin(T[1])+cos(T[1])) + (V.x*sin(T[2])+cos(T[2])),
        (V.z*sin(T[0])+cos(T[0])) + (V.x*sin(T[1])+cos(T[1])) + (V.y*sin(T[2])+cos(T[2]))
    );
}
