#version 460

in vec3 color;
out vec4 FragColor; // GLSL versions 4.20+ reject 'gl_FragColor' as "undeclared"

void main()
{
    //FragColor = vec4(color, 1.0f);
    //FragColor = vec4(color.r, 0.0f, color.b, 1.0f);
    FragColor = vec4(color.b, color.g, color.r, 1.0f);
}
