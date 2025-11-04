#ifndef CYLINDER_SHADER_LOADER_INCLUDED
#define CYLINDER_SHADER_LOADER_INCLUDED
#ifndef DISABLE_SHADERS
#define SHADERS_COMPILED true
#include <string>

// real definition in "gl.h"
typedef unsigned int GLuint;

// shader-files are expected to have the file-extension '.frag' or '.vert'
std::string LoadShader(std::string name);
GLuint CompileShaders(
  std::string vert_shader_name="default",
  std::string frag_shader_name="default");
bool ValidateShaderProgram(GLuint shader);

#else // ifdef DISABLE_SHADERS
#define SHADERS_COMPILED false
#endif // ifndef DISABLE_SHADERS
#endif // header guard
