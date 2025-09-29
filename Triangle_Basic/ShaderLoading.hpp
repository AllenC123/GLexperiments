#ifndef TRIANGLE_SHADER_LOADER_INCLUDED
#define TRIANGLE_SHADER_LOADER_INCLUDED
#ifndef DISABLE_SHADERS
#include <string>

typedef unsigned int GLuint; // real definition in "gl.h"
std::string LoadShader(std::string shader_name);
GLuint CompileShaders();
bool ValidateShaderProgram(GLuint program);

#endif //DISABLE_SHADERS
#endif
