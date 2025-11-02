#include <iostream>

#define GL_GLEXT_PROTOTYPES // must be defined to enable shader-related functions (glUseProgram)
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h> // glut-shape functions
//#include <GL/gl2.h>

#include "glutCylinder.hpp"


#ifndef DISABLE_SHADERS
#include "VertexT.hpp"
#include "ShaderLoading.hpp"
void ActivateShaders()
{
  /*static const std::array<VertexT, 3> vertex_list {
      VERTEX({-0.75f, 0.67f}, {1.f, 0.f, 0.f}),
      VERTEX({ 0.75f, 0.67f}, {0.f, 1.f, 0.f}),
      VERTEX({ 0.0f, -0.75f}, {0.f, 0.f, 1.f}),
  };*/
  
  GLfloat* vertices; GLfloat* normals; GLushort* sliceIdx; GLushort* stackIdx;
  double radius{1.0}; double height{256.0}; GLint slices{64}; GLint stacks{64};
  glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
  
  int i,j,idx, nVert;
  fghGenerateCylinder(radius,-height,slices,stacks,&vertices,&normals,&nVert);
  stackIdx = (GLushort*)malloc(slices*(stacks+1)*sizeof(GLushort));
  sliceIdx = (GLushort*)malloc(slices*2         *sizeof(GLushort));
  for (i=0,idx=0; i<stacks+1; i++)
  {
      GLushort offset = 1+(i+1)*slices;
      for (j=0; j<slices; j++, idx++)
      {
          stackIdx[idx] = offset+j;
      }
  }
  for (i=0,idx=0; i<slices; i++)
  {
      GLushort offset = 1+i;
      sliceIdx[idx++] = offset+slices;
      sliceIdx[idx++] = offset+(stacks+1)*slices;
  }
  /*fghDrawGeometryWire(
      vertices, normals, nVert,
      sliceIdx, 1, slices*2, GL_LINES,
      stackIdx, stacks+1, slices
  );*/
  
  const GLuint shader_program = CompileShaders();
  if (!ValidateShaderProgram(shader_program)) {
      std::cerr << "shader validation failed\n";
      exit(3);
  }
  else glUseProgram(shader_program);
  bool attributeLookupFailed{false};
  const GLint vertex_coord_location{glGetAttribLocation(shader_program, "vertex_coord")};
  //const GLint vertex_color_location{glGetAttribLocation(shader_program, "vertex_color")};
  
  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetAttribLocation.xhtml
  //If the named attribute is not an active attribute in the specified program object,
  //  or if the name starts with the reserved prefix "gl_", a value of -1 is returned.
  if (vertex_coord_location == -1) { std::cerr << "shader attribute-lookup failed for: 'vertex_coord'\n"; attributeLookupFailed = true; }
  //if (vertex_color_location == -1) { std::cerr << "shader attribute-lookup failed for: 'vertex_color'\n"; attributeLookupFailed = true; }
  if (attributeLookupFailed) { exit(4); }
  
  GLuint gl_vertex_array; glGenVertexArrays(1, &gl_vertex_array); glBindVertexArray(gl_vertex_array);
  GLuint vertex_buffer; glGenBuffers(1,&vertex_buffer); glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  //glNamedBufferData(vertex_buffer, sizeof(vertex_list), vertex_list[0].coord.data(), GL_STATIC_DRAW);
  glNamedBufferData(vertex_buffer, nVert*3, vertices, GL_STATIC_DRAW);
  
  glEnableVertexArrayAttrib(gl_vertex_array, vertex_coord_location);
  //glEnableVertexArrayAttrib(gl_vertex_array, vertex_color_location);
  glVertexAttribPointer(vertex_coord_location, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*3, vertices);
  //glVertexAttribPointer(vertex_color_location, 3, GL_FLOAT, GL_TRUE, sizeof(GLfloat), normals);
  // these ^ function-calls describe the data-layout of the data in 'GL_ARRAY_BUFFER'
  
  free(vertices); free(normals); free(stackIdx); free(sliceIdx);
  return;
}
#endif


void DisplayGlutCylinder()
{
    static GLdouble rotation_angle{0.00};
    static GLdouble rotation_delta{0.02};
    
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (rotation_angle >= 360) rotation_angle -= 360.0; else
    if (rotation_angle < -360) rotation_angle += 360.0;
    rotation_angle += rotation_delta;
    
    glLoadIdentity();
    glTranslated(-1, -1, 0);
    glRotated(5.0, 2.5, -2.5, 1.0);
    glRotated(rotation_angle, 0.0, 0.0, 1.0);
    
    double radius{1.0}; double height{-256.0}; GLint slices{64}; GLint stacks{64};
    // drawing wireframes with red fill/undercolor. seems like alpha is completely ignored
    glColor4f(0.5f, 0.0f, 0.0f, 1.0f); glutSolidCylinder(radius, height, slices, stacks);
    glColor4f(0.0f, 1.0f, 1.0f, 1.0f); glutWireCylinder(radius, height, slices, stacks);
    
    glTranslated(0, 0, (height*0.5) - (height*0.5*sin(rotation_angle*0.1)));
    //glColor4f(0.5f, 0.0f, 0.0f, 1.0f); glutSolidSphere(radius*0.85, slices, stacks);
    glColor4f(0.0f, 1.0f, 1.0f, 1.0f); glutWireSphere(radius *0.85, slices, stacks);
    // careful with the stacking order ('solid' shapes completely occlude the area)
    
    glutSwapBuffers();
    glutPostRedisplay(); // marks the current window as ready to be displayed
}


// has a name collision with some freeglut function
void Display_()
{
    static GLdouble rotation_angle{0.00};
    static GLdouble rotation_delta{0.02};
    
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (rotation_angle >= 360) rotation_angle -= 360.0; else
    if (rotation_angle < -360) rotation_angle += 360.0;
    rotation_angle += rotation_delta;
    
    glLoadIdentity();
    glTranslated(-1, -1, 0);
    glRotated(5.0, 2.5, -2.5, 1.0);
    glRotated(rotation_angle, 0.0, 0.0, 1.0);
    
    glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
    GLfloat* vertices; GLfloat* normals; GLushort* sliceIdx; GLushort* stackIdx;
    double radius{1.0}; double height{-256.0}; GLint slices{64}; GLint stacks{64};
    // why doesn't this work?????????
    /*int nVert = fghCylinder(
        radius, -height, slices, stacks, GL_TRUE,
        vertices, normals, sliceIdx, stackIdx
    );*/
    
    int i,j,idx{0}; // idx into vertex/normal buffer
    GLfloat radf = (GLfloat)radius; GLfloat z;
    const GLfloat zStep = (GLfloat)height / ( ( stacks > 0 ) ? stacks : 1 ); // Step in z as stacks are drawn.
    GLfloat *sint,*cost; // Pre-computed circle
    fghCircleTable(&sint,&cost,-slices,GL_FALSE);
    
    int nVert{slices*(stacks+3)+2}; // need two extra stacks for closing off top and bottom with correct normals
    // Note, (stacks+1)*slices vertices for side of object, 2*slices+2 for top and bottom closures
    //fghGenerateCylinder(radius,-height,slices,stacks,&vertices,&normals,&nVert);
    vertices = (GLfloat*)malloc((nVert)*3*sizeof(GLfloat));
    normals  = (GLfloat*)malloc((nVert)*3*sizeof(GLfloat));
    
    z=0; // top on Z-axis
    vertices[0] =  0.f;
    vertices[1] =  0.f;
    vertices[2] =  0.f;
    normals[0] =  0.f;
    normals[1] =  0.f;
    normals[2] = -1.f;
    idx = 3;
    // other on top (get normals right)
    for (j=0; j<slices; j++, idx+=3)
    {
        vertices[idx  ] = cost[j]*radf;
        vertices[idx+1] = sint[j]*radf;
        vertices[idx+2] = z;
        normals[idx  ] = 0.f;
        normals[idx+1] = 0.f;
        normals[idx+2] = -1.f;
    }
    
    // each stack
    for (i=0; i<stacks+1; i++ )
    {
        for (j=0; j<slices; j++, idx+=3)
        {
            vertices[idx  ] = cost[j]*radf;
            vertices[idx+1] = sint[j]*radf;
            vertices[idx+2] = z;
            normals[idx  ] = cost[j];
            normals[idx+1] = sint[j];
            normals[idx+2] = 0.f;
        }

        z += zStep;
    }
    
    z -= zStep;
    // other on bottom (get normals right)
    for (j=0; j<slices; j++, idx+=3)
    {
        vertices[idx  ] = cost[j]*radf;
        vertices[idx+1] = sint[j]*radf;
        vertices[idx+2] = z;
        normals[idx  ] = 0.f;
        normals[idx+1] = 0.f;
        normals[idx+2] = 1.f;
    }
    
    // bottom
    vertices[idx  ] =  0.f;
    vertices[idx+1] =  0.f;
    vertices[idx+2] =  height;
    normals[idx  ] =  0.f;
    normals[idx+1] =  0.f;
    normals[idx+2] =  1.f;
    
    // Release sin and cos tables
    free(sint);
    free(cost);
    
    // _____________________________________________________________//
    // fghCylinder
    // _____________________________________________________________//
    stackIdx = (GLushort*)malloc(slices*(stacks+1)*sizeof(GLushort));
    sliceIdx = (GLushort*)malloc(slices*2         *sizeof(GLushort));
    for (i=0,idx=0; i<stacks+1; i++) // generate for each stack
    {
        GLushort offset = 1+(i+1)*slices; // start at 1 (0 is top vertex), and we advance one stack down as we go along
        for (j=0; j<slices; j++, idx++)
        {
            stackIdx[idx] = offset+j;
        }
    }
    for (i=0,idx=0; i<slices; i++) // generate for each slice
    {
        GLushort offset = 1+i; // start at 1 (0 is top vertex), and we advance one slice as we go along
        sliceIdx[idx++] = offset+slices;
        sliceIdx[idx++] = offset+(stacks+1)*slices;
    }
    /*fghDrawGeometryWire(
      vertices, normals, nVert,
      sliceIdx, 1, slices*2, GL_LINES,
      stackIdx, stacks+1, slices
    );*/
    
    // manually drawing
    GLsizei numParts = 1;
    GLsizei numParts2 = stacks+1;
    GLsizei numVertPerPart = slices*2;
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glNormalPointer(GL_FLOAT, 0, normals);
    
    if (sliceIdx) for (i=0; i<numParts; i++) // drawing lines along the cylinder
    glDrawElements(GL_LINES,numVertPerPart,GL_UNSIGNED_SHORT,sliceIdx+i*numVertPerPart);
    
    if (stackIdx) for (i=0; i<numParts2; i++) // drawing rings
    glDrawElements(GL_LINE_LOOP,slices,GL_UNSIGNED_SHORT,stackIdx+i*slices);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    // end of manual draw
    
    // these almost work (missing the lengthwise lines)
    //glDrawArrays(GL_LINES,i*numVertPerPart,numVertPerPart); // drawing lines along the cylinder
    //glDrawArrays(GL_LINE_LOOP,i*slices,numParts2); // drawing rings
    
    // attempting to find equivalent vertex/normal calls
    //glDrawElements(GL_LINES,numVertPerPart,GL_UNSIGNED_SHORT,sliceIdx+i*numVertPerPart);
    /*glBegin(GL_LINES);
    for (i=0; i<numParts; i++) // drawing lines along the cylinder
    {
        glVertex3fv((GLfloat*)sliceIdx+i*numVertPerPart);
        glNormal3fv((GLfloat*)sliceIdx+i*numVertPerPart);
    }
    glEnd();
    
    //glDrawElements(GL_LINE_LOOP,slices,GL_UNSIGNED_SHORT,stackIdx+i*slices);
    glBegin(GL_LINE_LOOP);
    for (i=0; i<numParts2; i++) // drawing rings
    {
        glVertex3fv((GLfloat*)stackIdx+i*slices);
        glNormal3fv((GLfloat*)stackIdx+i*slices);
    }
    glEnd();*/
    
    // shaders apply even to previous draws?
    // why does the shader only ever draw a circle???
    /*const GLuint shader_program = CompileShaders();
    if (!ValidateShaderProgram(shader_program)) {
        std::cerr << "shader validation failed\n";
        exit(3);
    }
    else glUseProgram(shader_program);
    bool attributeLookupFailed{false};
    const GLint vertex_coord_location{glGetAttribLocation(shader_program, "vertex_coord")};
    
    if (vertex_coord_location == -1) { std::cerr << "shader attribute-lookup failed for: 'vertex_coord'\n"; attributeLookupFailed = true; }
    //if (vertex_color_location == -1) { std::cerr << "shader attribute-lookup failed for: 'vertex_color'\n"; attributeLookupFailed = true; }
    if (attributeLookupFailed) { exit(4); }
    
    GLuint gl_vertex_array; glGenVertexArrays(1, &gl_vertex_array); glBindVertexArray(gl_vertex_array);
    GLuint vertex_buffer; glGenBuffers(1,&vertex_buffer); glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    //glVertexArrayElementBuffer()
    //glVertexArrayVertexBuffer()
    
    glNamedBufferData(vertex_buffer, nVert*3, vertices, GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, nVert*3, vertices, GL_STATIC_DRAW);
    glEnableVertexArrayAttrib(gl_vertex_array, vertex_coord_location);
    glVertexAttribPointer(vertex_coord_location, 3, GL_UNSIGNED_SHORT, GL_FALSE, 0, vertices);
    
    // fg_gl2.h / fg_gl2.c
    //glutSetVertexAttribCoord3(vertex_coord_location);
    //fghEnableVertexAttribArray(vertex_coord_location);
    //fghVertexAttribPointer(vertex_coord_location, 3, GL_UNSIGNED_SHORT, GL_FALSE, 0, vertices);
    
    glDrawArrays(GL_LINES, 0, nVert*3);
    //glBindBuffer(GL_ARRAY_BUFFER, 0); // messes up inner lines???
    glBindVertexArray(0);*/
    
    glutSwapBuffers();
    glutPostRedisplay(); // marks the current window as ready to be displayed
    free(vertices); free(normals); free(stackIdx); free(sliceIdx);
}

// window-resize callback. the function signature requires integer parameters,
// otherwise they should be doubles to avoid the recasting in 'gluPerspective'
void Reshape(int width, int height) {
    glViewport(0, 0, width, height); // necessary for scaling after a window resize
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(16.0, static_cast<double>(width)/height, 0.0, 256.0);
    gluLookAt(0.0, 0.0, 12.0, -1.0, -1.0, -16.0, 0.0, 1.0, 0.0);
    glMatrixMode(GL_MODELVIEW); // this can go in 'display' function instead
}

void InitializeGL()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glLineWidth(1.0f);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(16.0, 1.0, 0.0, 256.0);
    gluLookAt(0.0, 0.0, 12.0, -1.0, -1.0, -16.0, 0.0, 1.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(960, 960);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH);
    //glutInitContextVersion(3,1); // version 4.0+ unsupported? (glut shape functions don't render)
    // 3.1 renders the shapes, but all higher versions do not. v3.3 is highest minor version for v3
    
    //'GLUT_CORE_PROFILE' is default? (doesn't render)
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE); // fixes higher versions
    glutInitContextVersion(4, 6); // rendering works with compatibility-profile!
    
    // ironically, 'GLUT_FORWARD_COMPATIBLE' breaks rendering for all context-versions above 2.1!!
    /*glutInitContextFlags(GLUT_DEBUG | GLUT_FORWARD_COMPATIBLE); // context-flags can be combined
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE); // never use!
    glutInitContextFlags(GLUT_DEBUG); // does nothing? */
    
    // must be called after glutInit
    //glutSetOption(GLUT_GEOMETRY_VISUALIZE_NORMALS, true); // only affects glut's built-in object-rendering functions (glutTorus)
    // 'GLUT_GEOMETRY_VISUALIZE_NORMALS' has no effect on the cylinder/sphere functions?
    glutCreateWindow("Cylinder Depth Test");
    
    InitializeGL();
    glutDisplayFunc(DisplayGlutCylinder);
    //glutDisplayFunc(Display_);
    glutReshapeFunc(Reshape);
    //ActivateShaders();
    //PrintCylinder();
    //glutExit();
    //return 0;
    
    // 'glutenum' here should be one of the GLUT API macro definitions from freeglut_std.h
    // starting from 'GLUT_WINDOW_X' and ending with 'GLUT_WINDOW_FORMAT_ID'
    #define PRINTGLUT(glutenum) std::cout << #glutenum << ": " << glutGet(glutenum) << '\n';
    PRINTGLUT(GLUT_VERSION);
    std::cout << '\n';
    
    // major/minor versions are reported as '1/0' if 'InitContextVersion' was not called
    PRINTGLUT(GLUT_INIT_MAJOR_VERSION);
    PRINTGLUT(GLUT_INIT_MINOR_VERSION);
    PRINTGLUT(GLUT_INIT_PROFILE);
    PRINTGLUT(GLUT_INIT_FLAGS);
    std::cout << '\n';
    
    // window must be created before these calls
    PRINTGLUT(GLUT_WINDOW_RGBA); // always true?
    PRINTGLUT(GLUT_WINDOW_DOUBLEBUFFER); // controlled by 'GLUT_DOUBLE' DisplayMode-flag
    PRINTGLUT(GLUT_INIT_DISPLAY_MODE);
    PRINTGLUT(GLUT_WINDOW_FORMAT_ID);
    PRINTGLUT(GLUT_ELAPSED_TIME);
    std::cout << '\n';
    
    glutMainLoop(); // never returns
    return 0;
}
