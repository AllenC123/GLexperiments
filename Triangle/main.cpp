// glfw3 header includes the OpenGL header from your development environment by default,
// but on some platforms this only supports older versions of OpenGL.
// Most programs use an extension loader library instead (glad)
// In which case it needs to be included first

#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"
#define GLFW_INCLUDE_NONE // redundant; explicitly disabling inclusion of the development environment's header.
#include <GLFW/glfw3.h>

#include "ShaderLoading.hpp"
#include "linmath.h" // mat4x4

struct Vertex {
    vec2 pos;
    vec3 col;
};

static const Vertex vertices[3] {
  { { -0.6f, -0.4f }, { 1.f, 0.f, 0.f } },
  { {  0.6f, -0.4f }, { 0.f, 1.f, 0.f } },
  { {   0.f,  0.6f }, { 0.f, 0.f, 1.f } },
};

static void error_callback(int error, const char* description) { fprintf(stderr, "Error: %s\n", description); }
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS)) glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main()
{
    glfwSetErrorCallback(error_callback);
    
    if (!glfwInit()) exit(EXIT_FAILURE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Triangle", NULL, NULL);
    if (!window) { glfwTerminate(); exit(EXIT_FAILURE); }
    
    glfwSetKeyCallback(window, key_callback);
    
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);
    
    // NOTE: OpenGL error checks have been omitted for brevity
    
    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    const GLuint shader_program = CompileShaders();
    const GLint mvp_location = glGetUniformLocation(shader_program, "MVP");
    const GLint vpos_location = glGetAttribLocation(shader_program, "vPos");
    const GLint vcol_location = glGetAttribLocation(shader_program, "vCol");
    
    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);
    glEnableVertexAttribArray(vpos_location);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, pos));
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, col));
    
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float) height;
        
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        
        mat4x4 m, p, mvp;
        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, (float) glfwGetTime());
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);
        
        glUseProgram(shader_program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp);
        glBindVertexArray(vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
