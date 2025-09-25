#ifndef TORUS_HPP_DEFINED
#define TORUS_HPP_DEFINED

#include <vector>
#include <cmath>

//#include <glm/glm.hpp> // vec3, mat4, radians
#include <glm/common.hpp>
#include <glm/matrix.hpp>
#include <glm/ext.hpp> // perspective, translate, rotate


class Torus_T {
// assumed to be centered at the origin, with it's axis aligned along +Z
public:
    float OuterRadius {1.0};
    float TubeRadius {0.5};
    float Resolution{0.01}; //the interval at which we circumnavigate / draw the ring.
    float du {2*M_PI}, dv {2*M_PI}; // tracing angle in radians
    int nR{16}, nr{16}; // ???? number of radians, I think?

    std::vector<float> vertices{};
    std::vector<float> normals{};
    std::vector<float> texCoords{};
    std::vector<float> tangents{};

    // A torus is given by the paramteric equations:
    // x = (R + r cos(v))cos(u)
    // y = (R + r cos(v))sin(u)
    // z = r sin(v)
    // u, v => [0, 2pi]

    void Clear()
    {
        vertices.clear();
        normals.clear();
        texCoords.clear();
        tangents.clear();
    }

    void Create();
    //void Render(const glm::mat4& vMat, const glm::mat4& pMat);
};










#endif