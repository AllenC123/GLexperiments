#include <iostream>
#include <format> // ParseCmdlineArgs
#include <cmath>  // inlined 'fghCircleTable' requires: sin, cos, and M_PI

#define GL_GLEXT_PROTOTYPES // must be defined to enable shader-related functions (glUseProgram)
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h> // glut-shape functions


// alternative rendering selection
static bool useGlutFunction{false};
static bool glutSphereMovement{true};
static GLdouble rotation_angle{0.00};
static GLdouble rotation_delta{0.02};

// Cylinder variables and geometric parameters
GLfloat* vertices; GLfloat* normals; GLushort* sliceIdx; GLushort* stackIdx;
double radius{1.0}; double height{256.0}; GLint slices{64}; GLint stacks{64};
int nVert{slices*(stacks+3)+2}; // need two extra stacks for closing off top and bottom with correct normals
// Note, (stacks+1)*slices vertices for side of object, 2*slices+2 for top and bottom closures


//#define DISABLE_SHADERS
#ifndef DISABLE_SHADERS
#include "ShaderLoading.hpp"

// intentionally sets incorrect 'stride' value in 'glVertexAttribPointer'
// which causes the shader to draw a figure-8 spline (using glDrawArrays)
#define RENDER_SPLINE_HACK

GLuint shader_program{0};
GLuint gl_vertex_array{};
GLuint vertex_buffer{};

void ActivateShaders()
{
  shader_program = CompileShaders();
  if (!ValidateShaderProgram(shader_program)) {
      std::cerr << "shader validation failed\n";
      exit(3);
  }
  else glUseProgram(shader_program);
  bool attributeLookupFailed{false};
  const GLint vertex_coord_location{glGetAttribLocation(shader_program, "vertex_coord")};
  
  if (vertex_coord_location == -1) { std::cerr << "shader attribute-lookup failed for: 'vertex_coord'\n"; attributeLookupFailed = true; }
  if (attributeLookupFailed) { exit(4); }
  
  glGenVertexArrays(1, &gl_vertex_array); glBindVertexArray(gl_vertex_array);
  glGenBuffers(1,&vertex_buffer); glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  
  glEnableVertexArrayAttrib(gl_vertex_array, vertex_coord_location);
  glEnableVertexArrayAttrib(vertex_buffer, vertex_coord_location);
  #ifndef RENDER_SPLINE_HACK
    // stride parameter should be calculated using a multiple of the 'size' parameter (array length)
    glVertexAttribPointer(vertex_coord_location, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*3, vertices);
  #else //RENDER_SPLINE_HACK
    // intentionally incorrect stride (no multiplier); renders a figure-8 spline with 'glDrawArray'
    glVertexAttribPointer(vertex_coord_location, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat), vertices);
  #endif
  return;
}

#endif //#ifndef DISABLE_SHADERS


// Alternative implementation using GLUT shape-drawing functions
// displays wireframe cylinder and sphere - both with a red fill
void GlutRenderCylinder()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (rotation_angle >= 360) rotation_angle -= 360.0; else
    if (rotation_angle < -360) rotation_angle += 360.0;
    rotation_angle += rotation_delta;
    
    glLoadIdentity();
    glTranslated(-1, -1, 0);
    glRotated(5.0, 2.5, -2.5, 1.0);
    glRotated(rotation_angle, 0.0, 0.0, 1.0);
    
    // drawing wireframes with red fill/undercolor. seems like alpha is completely ignored
    glColor4f(0.5f, 0.0f, 0.0f, 1.0f); glutSolidCylinder(radius, -height, slices, stacks);
    glColor4f(0.0f, 1.0f, 1.0f, 1.0f); glutWireCylinder(radius, -height, slices, stacks);
    
    if (glutSphereMovement)
    { glTranslated(0, 0,-(height*0.5)+(height*0.5*sin(rotation_angle*0.1))); } else {
    glColor4f(0.5f, 0.0f, 0.0f, 1.0f); glutSolidSphere(radius*0.85, slices, stacks);}
    glColor4f(0.0f, 1.0f, 1.0f, 1.0f); glutWireSphere(radius *0.85, slices, stacks);
    // careful with the stacking order ('solid' shapes completely occlude the area)
    
    glutSwapBuffers();
    glutPostRedisplay(); // marks the current window as ready to be displayed
    return;
}


// Unfortunately, GLUT functions allocate (and delete!) all of their generated vertex-array data internally.
// Consequently, integrating shaders becomes impossible because you cannot create a pointer for glBufferData
// This function effectively inlines the entire codepath executed by glutWireCylinder (expanded recursively),
//  such that all vertex-data remains accessible during rendering.
//  also heap-allocated variables have been made static or global.
// Reimplementation of 'glutWireCylinder' with shader integration!
void RenderCylinder()
{
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
    int i,j,idx{0}; // idx into vertex/normal buffer
    GLfloat radf = (GLfloat)radius; GLfloat z;
    const GLfloat zStep = (GLfloat)-height/((stacks > 0)? stacks : 1); // Step in z as stacks are drawn.
    
    // Pre-computed circle
    static GLfloat *sint{};
    static GLfloat *cost{};
    if ((!sint) || (!cost))
    { // fghCircleTable(&sint, &cost, -slices, GL_FALSE);
      const int size = abs(slices); // Table size, the sign of slices flips the circle direction
      // Allocate memory for n samples, plus duplicate of first entry at the end
      sint = (GLfloat*)malloc(sizeof(GLfloat) * (size+1));
      cost = (GLfloat*)malloc(sizeof(GLfloat) * (size+1));
      sint[0] = 0.0f;
      cost[0] = 1.0f;
      
      // Determine the angle between samples
      const GLfloat angle = 2*(GLfloat)M_PI/(GLfloat)((slices == 0)? 1 : slices);
      
      // Compute cos and sin around the circle
      for (int i{1}; i < size; ++i) {
          sint[i] = (GLfloat)sin(angle*i);
          cost[i] = (GLfloat)cos(angle*i);
      }
      // Last sample is duplicate of the first (sin or cos of 2 PI)
      sint[size] = sint[0];
      cost[size] = cost[0];
    }
    
    //fghGenerateCylinder(radius, -height, slices, stacks, &vertices, &normals, &nVert);
    if (!vertices) vertices = (GLfloat*)malloc((nVert)*3*sizeof(GLfloat));
    if (!normals)  normals  = (GLfloat*)malloc((nVert)*3*sizeof(GLfloat));
    
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
    //free(sint); free(cost);
    
    // _____________________________________________________________//
    // fghCylinder
    // _____________________________________________________________//
    if (!stackIdx) stackIdx = (GLushort*)malloc(slices*(stacks+1)*sizeof(GLushort));
    if (!sliceIdx) sliceIdx = (GLushort*)malloc(slices*2         *sizeof(GLushort));
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
    
    // __________________________________________________________________________________ //
    // rendering
    // __________________________________________________________________________________ //
    
    #ifndef DISABLE_SHADERS
        if (!shader_program) ActivateShaders();
        glUseProgram(shader_program);
        glBindVertexArray(gl_vertex_array);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        
        // which one of these is correct? why does this only SOMETIMES work??
        glNamedBufferData(gl_vertex_array, nVert, vertices, GL_DYNAMIC_DRAW);
        glNamedBufferData(vertex_buffer, nVert*3, vertices, GL_DYNAMIC_DRAW);
        //glNamedBufferData(gl_vertex_array, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        //glNamedBufferData(vertex_buffer, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    #endif
    
    #ifdef RENDER_SPLINE_HACK
    glDrawArrays(GL_LINE_LOOP, 0, nVert);
    // spline rendering (no multiplier on 'stride' in 'glVertexAttribPointer')
    // should draw vertex-data with 'glDrawArrays' instead of 'glDrawElements'
    #else // manually drawing
        glVertexPointer(3, GL_FLOAT, 0, vertices);
        glNormalPointer(GL_FLOAT, 0, normals);
        
        GLsizei numParts = 1;
        GLsizei numParts2 = stacks+1;
        GLsizei numVertPerPart = slices*2;
        
        for (i=0; i<numParts; i++) // drawing lines along the cylinder
        glDrawElements(GL_LINES, numVertPerPart, GL_UNSIGNED_SHORT, sliceIdx+(i*numVertPerPart));
        
        for (i=0; i<numParts2; i++) // drawing rings
        glDrawElements(GL_LINE_LOOP, slices, GL_UNSIGNED_SHORT, stackIdx+(i*slices));
    #endif
    
    glutSwapBuffers();
    glutPostRedisplay(); // marks the current window as ready to be displayed
    //free(vertices); free(normals); free(stackIdx); free(sliceIdx);
    return;
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

// no idea what the integer parameters are
void KeypressCallback(unsigned char C, int, int) {
    switch(C) {
        case 'q': case 'Q': glutExit(); exit(0);
        default: break;
    }
}

void InitializeGL()
{
    #ifdef DISABLE_SHADERS
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    #endif
    
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glLineWidth(1.0f);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(16.0, 1.0, 0.0, 256.0);
    gluLookAt(0.0, 0.0, 12.0, -1.0, -1.0, -16.0, 0.0, 1.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
}

int ParseCmdlineArgs(int argc, char** argv)
{
    for (int C{1}; C<argc; C++)
    { // parsing commandline arguments
      std::string arg{argv[C]}; bool known{false};
      std::cout << std::format("arg[#{}]: '{}'\n", C, arg);
      if (arg.starts_with("--rotationspeed")) { known=true; rotation_delta=std::stod(argv[++C]); } // TODO: bounds-check
      if (arg.starts_with("--rotationdelta")) { known=true; rotation_delta=std::stod(argv[++C]); } // TODO: bounds-check
      if (arg.starts_with("--freeze-sphere")) { known=true; glutSphereMovement=false; }
      if (arg.starts_with("--altrender") ||
      (arg.starts_with("--glut-cylinder")))
      { known=true; useGlutFunction=true; }
      if ((!known) || (arg == "--help")) {
      if (arg != "--help") std::cerr << std::format("[ERROR] unrecognized argument: '{}'\n", arg);
        typedef std::array<std::string, 2> UsageString; std::array<UsageString, 5> OptionStrings {
          UsageString{"--glut-cylinder", "render with glut shape-drawing functions (no shaders)"},
          UsageString{"--freeze-sphere", "disables sphere movement inside glut drawing function"},
          UsageString{"--rotationspeed", "set the rotation speed of the cylinder (default 0.02)"},
          UsageString{"--rotationdelta", "alias of '--rotationspeed'"},
          UsageString{"--altrender", "alias of '--glut-cylinder'"},
        }; std::cerr << "available options:\n";
        for (const auto& [option, description]: OptionStrings) {
          std::cerr << std::format(" {}: {}\n", option, description);
        } std::cerr << '\n';
        if (arg == "--help") {
          std::cerr << "'Q' terminates the program\n";
          std::cerr << "when shaders are enabled, you may get a completely black window\n"
            << "in that case, just re-run the program until it renders something (lmao)\n"
            << "  The GLUT implementation of the rendering-function should always work.\n";
        }
        return C;
      }
    }
    // TODO: iterate over OptionStrings to match and parse args.
    // implement aliases and nargs/arg-counts for options expecting parameters (like '--rotationspeed')
    return 0;
}

int main(int argc, char** argv)
{
    if (int status{0}; (status = ParseCmdlineArgs(argc, argv)))
        return status; // returns index of first invalid arg
    
    if (!useGlutFunction)
    {
      std::cout << "OpenGL Cylinder Rendering";
      #ifdef DISABLE_SHADERS
        std::cout << "\n_________________\n";
        std::cout << "\nSHADERS DISABLED!\n";
        std::cout << "_________________\n\n";
      #else
        std::cout << " [using shaders]";
        #ifndef RENDER_SPLINE_HACK
            std::cout << "\n\n";
        #endif
      #endif
      #ifdef RENDER_SPLINE_HACK
        std::cout << "\n____________________________\n";
        std::cout << "\nSPLINE RENDERING HACK ACTIVE\n";
        std::cout << "____________________________\n\n";
      #endif
    } else { // if (useGlutFunction)
      std::cout << "\nCylinder Rendering using GLUT [shaders off]\n";
      std::cout << std::format("    glut-sphere movement type: {}\n",
          ((glutSphereMovement)? "sliding" : "frozen (stationary)"));
      std::cout << '\n';
    }
    // TODO: pretty sure 'RENDER_SPLINE_HACK' can be refactored to boolean
    // TODO: print more cylinder vars (rotation-speed, radius, length, etc)
    // TODO: keybinds for: rotation_delta, sphere-movement, camera position
    // TODO: cmdline args for shader names and other params (slices/stacks)
    // is it possible to reassign 'glutDisplayFunc' in mainloop? test this.
    
    glutInit(&argc, argv);
    glutInitWindowSize(960, 960);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH);
    
    // seems unnecessary to specify the context-version - defaults to 1.0
    //glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE); // fixes higher versions
    //glutInitContextVersion(4, 6); // rendering works with compatibility-profile!
    
    // ironically, 'GLUT_FORWARD_COMPATIBLE' breaks rendering for all context-versions above 2.1!!
    /*glutInitContextFlags(GLUT_DEBUG | GLUT_FORWARD_COMPATIBLE); // context-flags can be combined
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE); // never use!
    glutInitContextFlags(GLUT_DEBUG); // does nothing? */
    
    // must be called after glutInit
    //glutSetOption(GLUT_GEOMETRY_VISUALIZE_NORMALS, true); // only affects glut's built-in object-rendering functions (glutTorus)
    // 'GLUT_GEOMETRY_VISUALIZE_NORMALS' has no effect on the cylinder/sphere functions?
    
    glutCreateWindow("CylinderRender");
    glutKeyboardFunc(KeypressCallback);
    glutDisplayFunc((!useGlutFunction)?
    RenderCylinder:GlutRenderCylinder);
    glutReshapeFunc(Reshape);
    
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
    
    InitializeGL();
    glutMainLoop(); // never returns
    
    return 0;
}
