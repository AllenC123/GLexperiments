#ifndef TORUS_RENDERING_HPP_INCLUDED
#define TORUS_RENDERING_HPP_INCLUDED

constexpr float PI = 3.14159265358979323846; // should use built-in constant instead
constexpr int numStrips = 32;
constexpr int numRings = 96;
constexpr int numVertecies = 1;
constexpr float tubeRadius = 0.5f;
constexpr float outerRadius = 1.0f;

// unfortunately, ImGui doesn't have Sliders supporting doubles
extern float rotation_angle;
extern float rotation_delta;
constexpr float rotation_delta_default {0.005};

// typedef GLenum unsigned int
// extern GLenum cyclemethod;
extern unsigned int cyclemethod;
extern bool shouldCycleRendermethods;

// initializes torus-rendering window
void GlutStuff(int argc, char** argv);

extern bool ShouldStopRendering;
// you should make use of the 'glutCloseFunc' callback.
// unless it interferes with the 'CONTINUE_EXECUTION' option.


#endif
