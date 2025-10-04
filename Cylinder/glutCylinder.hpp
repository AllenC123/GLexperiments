#ifndef GLEXPERIMENTS_GLUT_CYLINDER_INCLUDED
#define GLEXPERIMENTS_GLUT_CYLINDER_INCLUDED
#include "freeglut/fg_gl2.h"

int fghCylinder(
    GLfloat radius, GLfloat height, GLint slices, GLint stacks, GLboolean useWireMode,
    GLfloat* vertices, GLfloat* normals, GLushort* sliceIdx, GLushort* stackIdx
);
void fghGenerateCylinder(
    GLfloat radius, GLfloat height, GLint slices, GLint stacks, /*  input */
    GLfloat **vertices, GLfloat **normals, int* nVert           /* output */
);
void fghDrawGeometryWire(GLfloat *vertices, GLfloat *normals, GLsizei numVertices,
  GLushort *vertIdxs, GLsizei numParts, GLsizei numVertPerPart, GLenum vertexMode,
  GLushort *vertIdxs2, GLsizei numParts2, GLsizei numVertPerPart2
);

void fghCircleTable(GLfloat **sint, GLfloat **cost, const int n, const GLboolean halfCircle);
void PrintCylinder();

#endif
