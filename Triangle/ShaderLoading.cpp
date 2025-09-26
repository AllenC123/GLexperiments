#include <filesystem>
#include <fstream> // file i/o operations
#include <iostream>

std::string LoadShader(std::string shader_name)
{
    std::filesystem::path shader_directory{std::filesystem::current_path() / "shaders"};
    if (!(std::filesystem::exists(shader_directory) &&
    std::filesystem::is_directory(shader_directory))){
        std::cerr << "[ERROR] failed to locate shader directory: " << shader_directory << std::endl;
        exit(2);
    }
    
    std::filesystem::path shader_path{shader_directory / shader_name};
    std::cout << "________________________________________________\n";
    std::cout << "loading shader: " << shader_path.filename() << '\n';
    std::cout << "________________________________________________\n";
    if (!std::filesystem::exists(shader_path)) {
        std::cerr << "[ERROR] failed to locate shader: " << shader_path << std::endl;
        exit(3);
    }
    
    std::ifstream shader_file{shader_path}; std::stringstream shader_text{}; std::string line{};
    if (!shader_file) { std::cerr << "[ERROR] unable to open/read file: " << shader_path << std::endl; exit(4); }
    while (std::getline(shader_file, line)) { shader_text << line << '\n'; }
    std::cout << shader_text.str();
    std::cout << "________________________________________________\n\n";
    
    return std::string{shader_text.str()};
}

#include "glad/gl.h"
GLuint CompileShaders()
{
    static const std::string vert_shader_str{LoadShader("test.vert")};
    static const std::string frag_shader_str{LoadShader("test.frag")};
    static const GLchar* vert_shader_cstring{vert_shader_str.c_str()};
    static const GLchar* frag_shader_cstring{frag_shader_str.c_str()};
    
    const GLuint vertex_shader{glCreateShader(GL_VERTEX_SHADER)};
    glShaderSource(vertex_shader, 1, &vert_shader_cstring, NULL);
    glCompileShader(vertex_shader);
    
    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &frag_shader_cstring, nullptr);
    glCompileShader(fragment_shader);
    
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    return program;
}
