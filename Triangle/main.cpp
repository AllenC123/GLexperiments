// glfw3 header includes the OpenGL header from your development environment by default,
// but on some platforms this only supports older versions of OpenGL.
// Most programs use an extension loader library instead (glad)
// In which case it needs to be included first

#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"
#define GLFW_INCLUDE_NONE // redundant; explicitly disabling inclusion of the development environment's header.
#include <GLFW/glfw3.h>

#include "linmath.h"


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

#ifndef HARDCODED_SHADERS_ONLY
#include <filesystem>
#include <fstream> // file i/o operations
#include <iostream>

std::string LoadShader(std::string shader_name)
{
    std::filesystem::path shader_directory{std::filesystem::current_path() / "shaders"};
    if (!(std::filesystem::exists(shader_directory) &&
    std::filesystem::is_directory(shader_directory))){
        std::cerr << "[ERROR] failed to locate shader directory: " << shader_directory << std::endl;
        glfwTerminate(); exit(2);
    }
    
    std::filesystem::path shader_path{shader_directory / shader_name};
    std::cout << "________________________________________________\n";
    std::cout << "loading shader: " << shader_path.filename() << '\n';
    std::cout << "________________________________________________\n";
    if (!std::filesystem::exists(shader_path)) {
        std::cerr << "[ERROR] failed to locate shader: " << shader_path << std::endl;
        glfwTerminate(); exit(3);
    }
    
    std::ifstream shader_file{shader_path}; std::stringstream shader_text{}; std::string line{};
    if (!shader_file) { std::cerr << "[ERROR] unable to open/read file: " << shader_path << std::endl; glfwTerminate(); exit(4); }
    while (std::getline(shader_file, line)) { shader_text << line << '\n'; }
    std::cout << shader_text.str();
    std::cout << "________________________________________________\n\n";
    
    return std::string{shader_text.str()};
}
#endif


GLuint CompileShaders()
{
    #ifdef HARDCODED_SHADERS_ONLY
    static const GLchar* vertex_shader_text =
    "#version 330\n"
    "uniform mat4 MVP;\n"
    "in vec3 vCol;\n"
    "in vec2 vPos;\n"
    "out vec3 color;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
    "    color = vCol;\n"
    "}\n";
    static const GLchar* fragment_shader_text =
    "#version 330\n"
    "in vec3 color;\n"
    "out vec4 fragment;\n"
    "void main()\n"
    "{\n"
    "    fragment = vec4(color, 1.0);\n"
    "}\n";
    #else
    static const std::string vert_shader_str {LoadShader("test.vert")};
    static const std::string frag_shader_str {LoadShader("test.frag")};
    static const GLchar* vertex_shader_text  {vert_shader_str.c_str()};
    static const GLchar* fragment_shader_text{frag_shader_str.c_str()};
    #endif
    
    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);
    
    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);
    
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    return program;
}


int main(void)
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Triangle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    // NOTE: OpenGL error checks have been omitted for brevity

    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    const GLuint program = CompileShaders();
    const GLint mvp_location = glGetUniformLocation(program, "MVP");
    const GLint vpos_location = glGetAttribLocation(program, "vPos");
    const GLint vcol_location = glGetAttribLocation(program, "vCol");

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

        glUseProgram(program);
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
