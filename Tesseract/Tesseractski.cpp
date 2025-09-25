#include <GL/glut.h>

//#include <iostream>
#include <cmath>


GLfloat rotation_angle = 0.0f;
GLfloat rotation_delta = 0.2f;

GLfloat verticies[][4] = {
    {1,1,1,1}, // 1
    {1,1,-1,1}, // 4
    {1,-1,-1,1}, // 6
    {1,-1,1,1}, // 3

    {-1,-1,1,1}, // 8
    {-1,-1,-1,1}, // 5
    {-1,1,-1,1}, // 7
    {-1,1,1,1}, // 2
};

// interesting
/* {1,1,1,1}, {1,0,0,0},
{1,1,1,1}, {0,1,0,0},
{1,1,1,1}, {0,0,1,0},
{1,1,1,1}, {0,0,0,1},
{-1,0,0,0}, {-1,-1,1,1}, //mirrors the shape
{0,-1,0,0}, {-1,-1,1,1},
{0,0,-1,0}, {-1,-1,1,1},
{0,0,0,-1}, {-1,-1,1,1},*/

GLfloat verticies_inner[][4] = {
    {0.5,0.5,0.5,1}, // 1
    {0.5,0.5,-0.5,1}, // 4
    {0.5,-0.5,-0.5,1}, // 6
    {0.5,-0.5,0.5,1}, // 3

    {-0.5,-0.5,0.5,1}, // 8
    {-0.5,-0.5,-0.5,1}, // 5
    {-0.5,0.5,-0.5,1}, // 7
    {-0.5,0.5,0.5,1}, // 2
};


// Vertex colors
GLfloat colors[][3] = {
    {1,0,0},    // Red
    {0,1,0},    // Green
    {0,0,1},    // Blue
    {1,1,0},    // Yellow
    {1,0,1},    // Magenta
    {0,1,1},    // Cyan
    {0,0.5,0},  // Orange
    {0,1,0.5},  //Lime
};


void InitializeGL()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glLineWidth(1.0);
    glEnable(GL_BLEND);
    glBlendColor(0.5,0.5,0.5,0.5);
    glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_DST_COLOR);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(10, 1, 0, 20);
    gluLookAt(0, 0, 20, 0, 0, 0, 0, 1, 0);
    glMatrixMode(GL_MODELVIEW);
}


void Display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glClearColor(0, 0, 0, 0);
    //glClear(GL_DEPTH_BUFFER_BIT);
    //glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_TRUE);
    //glClearColor(0.0, 0.0, 0.0, 0.0);
    //glClear(GL_COLOR_BUFFER_BIT);
    //glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

    glLoadIdentity();
    glRotatef(rotation_angle, 1, 0.9, 0.5);
    
    #ifdef ACCUMULATION_BUFFER
    glAccum(GL_MULT, 0.9);
    glAccum(GL_RETURN, 0.2);
    #endif
    
    glBegin(GL_LINES);
    for (int i = 0; i < 8; ++i) {
        GLfloat& B = verticies[i][3];
        B = 1.5 + 0.5*sin(2.0 * M_PI * rotation_angle / 360.0);
        for (int j = 1; j < 4; ++j) {
            int x = (i+j) % 8;
            int y = ((i >= j)? (i-j) : ((8+i) - j)); // negative modulo doesn't work right

            glColor3fv(colors[i % 8]); glVertex4fv(verticies[i]);
            glColor3fv(colors[x % 8]); glVertex4fv(verticies[x]);
            glColor3fv(colors[y % 8]); glVertex4fv(verticies[y]);
        }
    }
    glEnd();

    //gluLookAt(0, 0, 1, 0, 0, 0, 1, 0, 0); // offsets the position of the inner cube.
    //glTranslated(0.1,0.1,0.1); //better method

    // there must be a more efficient way to traverse this
    glBegin(GL_LINES);
    for (int i = 0; i < 8; ++i) {
        GLfloat& A = verticies_inner[i][3];
        A = 1.0 - 0.5*sin(2.0 * M_PI * rotation_angle / 360.0); // trig functions have a range of -1 to 1; don't allow the total to drop too low or the geometry will blow up.
        glColor3fv(colors[i % 8]); glVertex4fv(verticies_inner[i]);
        glColor3fv(colors[i % 8]); glVertex4fv(verticies[i]);
        //A = 1.0;
        for (int j = 1; j < 4; ++j) {
            int x = (i+j) % 8;
            int y = ((i >= j)? (i-j) : ((8+i) - j));

            //verticies_inner[x][3] = 1.0 + (rotation_angle / 360.0);
            //verticies_inner[y][3] = 1.0 + (rotation_angle / 360.0);
            glColor3fv(colors[i % 8]); glVertex4fv(verticies_inner[i]);
            glColor3fv(colors[x % 8]); glVertex4fv(verticies_inner[x]);
            glColor3fv(colors[y % 8]); glVertex4fv(verticies_inner[y]);
        }
    }
    glEnd();

    #ifdef ACCUMULATION_BUFFER
    glAccum(GL_ACCUM, 0.5);
    #endif
    
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

int main (int argc, char** argv) {
    glutInit(&argc, argv);
    
    // newer GPUs do not have accumulation-buffer in hardware
    #ifdef ACCUMULATION_BUFFER
    glEnable(GL_ACCUM);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA | GLUT_ACCUM | GLUT_DEPTH);
    #else
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH);
    #endif
    
    glutInitWindowSize(960, 960);
    glutCreateWindow("Tesseractski");
    
    InitializeGL();
    glutDisplayFunc(Display); // required
    glutReshapeFunc(Reshape); // optional
    glutTimerFunc(3, Timer, 0); // optional
    
    Display();
    glutMainLoop(); // never returns

    return 0;
}
