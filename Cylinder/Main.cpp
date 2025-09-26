//#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/glut.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h> // glut-Cylinder functions

//#include <iostream>
//#include <cmath>


GLdouble rotation_angle = 0.0;
GLdouble rotation_delta = 0.2;

void Display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glLoadIdentity();
    glTranslated(-1, -1, 0);
    glRotated(5.0, 2.5, -2.5, 1.0);
    glRotated(rotation_angle, 0.0, 0.0, 1.0);
    
    //gluLookAt(0, 0, 1, 0, 0, 0, 1, 0, 0);
    //glTranslated(0.9, 0.9, 0.9); //better method
    
    glColor3f(0.0f, 1.0f, 1.0f);
    double radius{1.0}; double height{256.0}; GLint slices{64}; GLint stacks{64};
    //glutWireSphere(radius, slices, stacks);
    glutWireCylinder(radius, -height, slices, stacks);
    //glutSolidCylinder(radius, height, slices, stacks);
    
    glutSwapBuffers();
}

// window-resize callback. Does this even do anything?
void Reshape(int width, int height) {
    glViewport(0, 0, width, height);
}

void Timer(int) {
    if (rotation_angle > 360.0) rotation_angle = 0;
    rotation_angle += rotation_delta;
    glutTimerFunc(3, Timer, 0); // queue next timer activation
    glutPostRedisplay();
}

void InitializeGL()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glLineWidth(1.0f);
    glEnable(GL_BLEND);
    glBlendColor(0.5, 0.5, 0.5, 0.5);
    glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_DST_COLOR);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(10.0, 1.0, 0.0, 256.0);
    gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH);
    glutInitWindowSize(960, 960);
    glutCreateWindow("Cylinder Depth Test");
    
    InitializeGL();
    glutDisplayFunc(Display); // required
    glutReshapeFunc(Reshape); // optional
    glutTimerFunc(3, Timer, 0); // optional
    
    Display();
    glutMainLoop(); // never returns

    return 0;
}
