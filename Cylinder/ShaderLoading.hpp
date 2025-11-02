#ifndef CYLINDER_SHADER_LOADER_INCLUDED
#define CYLINDER_SHADER_LOADER_INCLUDED
#ifndef DISABLE_SHADERS
#define SHADERS_COMPILED true
#include <string>

// real definition in "gl.h"
typedef unsigned int GLuint;

std::string LoadShader(std::string shader_name);
GLuint CompileShaders();
bool ValidateShaderProgram(GLuint program);

#else // ifdef DISABLE_SHADERS
#define SHADERS_COMPILED false
#endif // ifndef DISABLE_SHADERS
#endif // header guard
