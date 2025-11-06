#ifdef ENABLE_TORUS_FILES
#include "Torus.hpp"
//#include <iostream>


void Torus_T::Create()
{
    Clear();

    du = 2 * M_PI / nR;
    dv = 2 * M_PI / nr;

    for (size_t i=0; i < nR; i++)
    {
        float u = i * du;

        for (size_t j=0; j <= nr; j++)
        {
            float v = (j % nr) * dv; // we're just wrapping the last iteration around to 0, to cover rounding errors (triangle-mesh gaps)

            for (size_t k=0; k<2; k++)
            {
                float uu = u + k * du;
                // compute vertex
                vertices.emplace_back(OuterRadius + TubeRadius * cos(v)) * cos(uu); // x
                vertices.emplace_back(OuterRadius + TubeRadius * cos(v)) * sin(uu); // y
                vertices.emplace_back(TubeRadius * sin(v)); // z

                // compute normal
                normals.emplace_back(cos(v) * cos(uu)); // nx
                normals.emplace_back(cos(v) * sin(uu)); // ny
                normals.emplace_back(sin(v)); // nz

                // compute texture coords
                float tx = uu / (2 * M_PI);
                float ty = v / (2 * M_PI);

                // create a tangent-vector with GLM
                // T = d(S)/du
                // S(u) is the circle at constant v
                glm::vec3 tg
                (
                    -(OuterRadius + TubeRadius * cos(v)) * sin(uu),
                    (OuterRadius + TubeRadius * cos(v)) * cos(uu),
                    0.0f
                );

                tg = glm::normalize(tg);
                tangents.push_back(tg.x);
                tangents.push_back(tg.y);
                tangents.push_back(tg.z);

                //std::cout << tg.x << "," << tg.y << "," << tg.z << std::endl;
            }

            // incr angle
            v += dv;

        }
    }

    return;
}


glm::mat4 transform(glm::vec2 const& Orientation, glm::vec3 const& Translate, glm::vec3 const& Up)
{
    glm::mat4 Proj = glm::perspective(glm::radians(45.f), 1.33f, 0.1f, 10.f);
    glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.f), Translate);
    glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Orientation.y, Up);
    glm::mat4 View = glm::rotate(ViewRotateX, Orientation.x, Up);
    glm::mat4 Model = glm::mat4(1.0f);
    return Proj * View * Model;
}

#endif
