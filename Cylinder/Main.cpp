#include <iostream>
//#include <cmath>

//#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/glut.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h> // glut-shape functions


void Display()
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
    
    double radius{1.0}; double height{256.0}; GLint slices{64}; GLint stacks{64};
    // drawing wireframes with red fill/undercolor. seems like alpha is completely ignored
    glColor4f(0.5f, 0.0f, 0.0f, 1.0f); glutSolidCylinder(radius, -height, slices, stacks);
    glColor4f(0.0f, 1.0f, 1.0f, 1.0f); glutWireCylinder(radius, -height, slices, stacks);
    glColor4f(0.5f, 0.0f, 0.0f, 1.0f); glutSolidSphere(radius*0.85, slices, stacks);
    glColor4f(0.0f, 1.0f, 1.0f, 1.0f); glutWireSphere(radius *0.85, slices, stacks);
    // careful with the stacking order ('solid' shapes completely occlude the area)
    
    // clearing background while preserving existing color-channels (G/B)
    /*glClearColor(0.5, 0.0, 0.0, 0.5);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
    glClear(GL_COLOR_BUFFER_BIT);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);*/
    
    // trying to use blend-color
    /*glBlendColor(0.0f, 1.0f, 1.0f, 0.75f);
    //glBlendColor(0.75f, 0.0f, 0.0f, 0.25f);
    glClearColor(0.5f, 0.0f, 0.0f, 0.25f);
    glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);*/
    
    glutSwapBuffers();
    glutPostRedisplay(); // marks the current window as ready to be displayed
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
    
    // has no effect?
    /*glEnable(GL_BLEND);
    glBlendColor(0.5, 0.5, 0.5, 0.5);
    glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_DST_COLOR);*/
    
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
    glutDisplayFunc(Display);
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
    
    glutMainLoop(); // never returns
    return 0;
}
