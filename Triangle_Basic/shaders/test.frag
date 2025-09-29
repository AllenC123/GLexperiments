#version 460

in vec3 color;
out vec4 fragment;

void main()
{
    //fragment = vec4(color, 1.0f);
    fragment = vec4(color.r, 0.0f, color.b, 1.0f);
}
