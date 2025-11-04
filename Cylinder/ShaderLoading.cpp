#ifndef DISABLE_SHADERS
#include <filesystem>
#include <fstream> // file i/o operations
#include <iostream>
#include <vector>

// returns the contents of the file as a string
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
        std::cerr << "[ERROR] shader does not exist: " << shader_path << std::endl;
        exit(3);
    }
    
    std::ifstream shader_file{shader_path}; std::stringstream shader_text{}; std::string line{};
    if (!shader_file) { std::cerr << "[ERROR] unable to open/read file: " << shader_path << std::endl; exit(4); }
    while (std::getline(shader_file, line)) { shader_text << line << '\n'; }
    std::cout << shader_text.str();
    std::cout << "________________________________________________\n\n";
    
    return std::string{shader_text.str()};
}


#define GL_GLEXT_PROTOTYPES // must be defined for shader-related functions (glCreateShader, glCompileShader, etc)
#include <GL/gl.h>

GLuint CompileShaders(std::string vert_shader_name, std::string frag_shader_name)
{
    if (!vert_shader_name.ends_with(".vert")) vert_shader_name.append(".vert");
    if (!frag_shader_name.ends_with(".frag")) frag_shader_name.append(".frag");
    static const std::string vert_shader_code{LoadShader(vert_shader_name)};
    static const std::string frag_shader_code{LoadShader(frag_shader_name)};
    static const GLchar* vert_shader_cstring{vert_shader_code.c_str()};
    static const GLchar* frag_shader_cstring{frag_shader_code.c_str()};
    
    const GLuint vertex_shader{glCreateShader(GL_VERTEX_SHADER)};
    glShaderSource(vertex_shader, 1, &vert_shader_cstring, NULL);
    glCompileShader(vertex_shader);
    
    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &frag_shader_cstring, nullptr);
    glCompileShader(fragment_shader);
    
    const GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, fragment_shader);
    glAttachShader(shader_program, vertex_shader);
    glLinkProgram(shader_program);
    
    // after linking these can be deleted
    glDeleteShader(vertex_shader); glDeleteShader(fragment_shader);
    // the compiled shader-objects are left in GPU memory otherwise
    
    return shader_program;
}

bool ValidateShaderProgram(GLuint program)
{
    GLint link_status{GL_TRUE}; GLint isValid{GL_TRUE};
    glLinkProgram(program); glValidateProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_status);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &isValid);
    
    std::vector<GLchar> infolog(512, '\0'); GLsizei loglength{0};
    glGetProgramInfoLog(program,512, &loglength, infolog.data());
    
    if (loglength > 0) {
        const std::string underline{"\n________________________________________________________\n"};
        std::cout << underline << "OpenGL infolog" << " [length: " << loglength << "]" << underline;
        std::cout << infolog.data() << underline << "\n";
    }
    
    if (link_status == GL_FALSE) std::cerr << "linker failed\n";
    if (isValid == GL_FALSE) std::cerr << "validation failed\n";
    return ((link_status != GL_FALSE) && (isValid != GL_FALSE));
}

#endif // DISABLE_SHADERS
