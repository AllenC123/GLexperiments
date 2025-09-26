#ifndef TRIANGLE_SHADER_LOADER_INCLUDED
#define TRIANGLE_SHADER_LOADER_INCLUDED
#include <string>

typedef unsigned int GLuint; // real definition in "gl.h"
std::string LoadShader(std::string shader_name);
GLuint CompileShaders();

#endif
