#include <iostream>
#include <format>

#define GL_GLEXT_PROTOTYPES // must be defined to enable shader-related functions (glUseProgram)
// it is also required to import definitions for 'glGen'/'glBind' functions
#include <GLFW/glfw3.h>

#include "VertexT.hpp"


static void error_callback(int error, const char* description) { fprintf(stderr, "Error (%d): %s\n", error, description); }
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS)) glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main()
{
    glfwSetErrorCallback(error_callback); // safe to call before glfwInit
    
    if(!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    
    int win_width{640}, win_height{640}; // updated inside frameloop
    GLFWwindow* window = glfwCreateWindow(win_width, win_height, "OpenGL Triangle", NULL, NULL);
    if (!window) { glfwTerminate(); return 2; }
    
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    const std::array<VertexT, 3> vertex_list {
      VERTEX({-0.75f, 0.67f}, {1.f, 0.f, 0.f}),
      VERTEX({ 0.75f, 0.67f}, {0.f, 1.f, 0.f}),
      VERTEX({ 0.0f, -0.75f}, {0.f, 0.f, 1.f}),
    };
    
    std::cout << "vertex array:\n  ";
    for (int I{0}; const VertexT& vertex: vertex_list) {
        std::cout << std::format("vertex(#{}): ", I++);
        std::cout << vertex.Representation() << "\n  ";
    } std::cout << '\n';
    
    GLuint gl_vertex_array;
    glGenVertexArrays(1, &gl_vertex_array);
    glBindVertexArray(gl_vertex_array);
    
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // scaling without preserving aspect-ratio (stretching)
        glfwGetFramebufferSize(window, &win_width, &win_height);
        glViewport(0, 0, win_width, win_height);
        
        glBegin(GL_TRIANGLES);
        for (const VertexT& vertex: vertex_list) {
            glArrayElement(gl_vertex_array);
            glColor3fv(vertex.color.data());
            glVertex3fv(vertex.coord.data());
        }
        glEnd();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
