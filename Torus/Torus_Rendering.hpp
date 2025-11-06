#ifndef TORUS_RENDERING_HPP_INCLUDED
#define TORUS_RENDERING_HPP_INCLUDED
#include <string> //RenderMethodName

// initializes torus-rendering window
void GlutStuff(int argc, char** argv);

constexpr float PI = 3.14159265358979323846; // should use built-in constant instead
constexpr int numStrips = 32;
constexpr int numRings = 96;
constexpr int numVertecies = 1;
constexpr float tubeRadius = 0.5f;
constexpr float outerRadius = 1.0f;

// unfortunately, ImGui Sliders only use floats
constexpr float default_rotation_delta{0.125f};
extern float rotation_angle;
extern float rotation_delta;

typedef unsigned int GLenum; // gl.h
extern  GLenum renderMethod;
std::string RenderMethodName(GLenum);
extern bool shouldCycleRenderMethod;
extern bool shouldApplyRotation;
extern bool shouldStopRendering;
// you should make use of the 'glutCloseFunc' callback.
// unless it interferes with the 'CONTINUE_EXECUTION' option.

#endif
