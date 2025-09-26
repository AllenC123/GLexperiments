#define GL_GLEXT_PROTOTYPES // must be defined to enable shader-related functions (glUseProgram)
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
    glfwSetErrorCallback(error_callback); // safe to call before glfwInit
    
    if(!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    int win_width{640}, win_height{480}; float ratio{640.f/480.f}; // updated inside frameloop
    GLFWwindow* window = glfwCreateWindow(win_width, win_height, "OpenGL Triangle", NULL, NULL);
    if (!window) { glfwTerminate(); return 2; }
    
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
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
        glClear(GL_COLOR_BUFFER_BIT);
        
        // scaling / recentering the triangle to fit window-size
        glfwGetFramebufferSize(window, &win_width, &win_height);
        const float ratio{float(win_width) / float(win_height)}; // preserves aspect-ratio during resize
        // when this whole block is disabled, resizing the window never scales or recenters the triangle
        // when only the viewport-size is updated, resizing the window scales and stretches the triangle
        glViewport(0, 0, win_width, win_height);
        
        mat4x4 m, p, mvp;
        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, glfwGetTime());
        mat4x4_ortho(p, ratio, -ratio, -1.f, 1.f, 1.f, -1.f); // flipping signs on the 'ratio' reverses spin-direction
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
    return 0;
}
