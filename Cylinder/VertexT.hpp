#ifndef TRIANGLE_VERTEX_HPP_INCLUDED
#define TRIANGLE_VERTEX_HPP_INCLUDED

#include <array>
#include <string>

// definition in gl.h
typedef float GLfloat;

struct VertexT
{
    std::array<GLfloat,3> coord;
    std::array<GLfloat,3> color;
    std::string Representation() const; // "[XYZ](#RRGGBB)"
    VertexT(float pos[2], float col[3])
    : coord{pos[0], pos[1], 1.000f}, // somehow the Z-coord changes the glDrawArray color?!!?? 0 gives an ugly green/yellow
      color{col[0], col[1], col[2]}
    { ; }
};

#define VERTEX(X, Y, ...) VertexT{(new float[2] X, Y), (new float[3] __VA_ARGS__)}
// this macro ^ enables literal constructors (arrays):
/* const std::array<VertexT, 3> macro_syntax {
    VERTEX({-0.75f, 0.67f}, {1.f, 0.f, 0.f}),
    VERTEX({ 0.75f, 0.67f}, {0.f, 1.f, 0.f}),
    VERTEX({ 0.0f, -0.75f}, {0.f, 0.f, 1.f}),
};

// normally, you would have to write it like this
const std::array<VertexT, 3> normal_syntax_example {
    VertexT{(new float[2]{-0.75f, 0.67f}), (new float[3]{1.f, 0.f, 0.f})},
    VertexT{(new float[2]{ 0.75f, 0.67f}), (new float[3]{0.f, 1.f, 0.f})},
    VertexT{(new float[2]{ 0.0f, -0.75f}), (new float[3]{0.f, 0.f, 1.f})},
};

// constructor can't deduce initializer lists or arrays (also they're temporary values)
static_assert(std::is_convertible<float[2], decltype((new float[2]{-0.75f, 0.67f}))>()); // pass
static_assert(std::is_convertible<decltype((new float[2]{-0.75f, 0.67f})), float[2]>()); // fail
*/

#endif
