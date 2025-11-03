#include "Torus_Rendering.hpp"

#include <GL/glut.h> //should just specify freeglut instead
#include <GL/freeglut_ext.h> // required for glutSetOption (handling window-closing event properly)

#include <cmath>             // for sin and cos
#include <array>
#include <iostream>
#include <vector>
#include <cassert> // only used for a single assert (isFirstCall in GenerateTorus)
#include <thread>
#include <cstdlib> // for std::getenv


// 'glutenum' here should be one of the GLUT API macro definitions from freeglut_std.h
// starting from 'GLUT_WINDOW_X' and ending with 'GLUT_WINDOW_FORMAT_ID'
#define PRINTGLUT(glutenum) std::cout << #glutenum <<" = " << glutGet(glutenum) << '\n';


float rotation_angle {0};
float rotation_delta {rotation_delta_default};
bool ShouldStopRendering {false};

int index_total{0};

constexpr std::array<GLenum, 9> rendermethods { GL_POINTS, GL_LINES, GL_LINE_LOOP, GL_LINE_STRIP, GL_QUADS, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_QUAD_STRIP, GL_POLYGON};
//TRIANGLE_FAN and GL_POLYGON are both slow, but POLYGON seems a bit faster.
GLenum cyclemethod {GL_LINE_STRIP};
bool shouldCycleRendermethods {true};

#define PRECALCULATE_TORUS_GEOMETRY
#ifdef PRECALCULATE_TORUS_GEOMETRY

struct point_t
{
    GLfloat x, y, z;
    int UID; // used to lookup the colorcycle

    int strip_n, ring_n, k_n; // storing info about which loop of the geometry it belongs to
};

using m_points = std::array<point_t, numVertecies>;
using m_rings = std::array<m_points, numRings>;
using Torus_t = std::array<m_rings, numStrips>;


// we could probably just use 4d OpenGL matrices instead
struct colorentry_t 
{
    GLfloat r, g, b;
    colorentry_t() = default;
    colorentry_t(point_t& p, int degree) : r(p.x), g(p.y), b(p.z)
    {
        r += 0.1 + 0.35 * abs(sin(double(double((degree+1)/360.0) * PI)));
        g += 0.1 + 0.35 * abs(cos(double(double((degree+1)/360.0) * PI)));
        b += 0.1 + 0.35 * abs(tan(double(double((degree+1)/360.0) * PI)));

        //r += abs(sin(double(double((degree+1)/360.0) * double(p.UID % (p.strip_n+1)) * PI)));
        //g += abs(cos(double(double((degree+1)/360.0) * double(p.UID % (p.ring_n+1)) * PI)));
        //b += abs(tan(double(double((degree+1)/360.0) * double(p.UID % (p.k_n+1)) * PI)));

        //# define RGB_CYCLE_RENDERMETHOD
        //# ifdef RGB_CYCLE_RENDERMETHOD
        //r += 0.2 * double((storedpoint.UID+1) % (storedpoint.strip_n + 1)) * abs(sin(double(double(((degree+storedpoint.ring_n+1) * rotation_angle)/360.0) * 2.0 * PI)));
        //g += 0.2 * double((degree+1) % (storedpoint.k_n + 1)) * abs(cos(double(double(((degree+1) * rotation_angle)/360.0) * 2.0 * PI)));
        //b += 0.2 * double((storedpoint.strip_n+1) % (storedpoint.ring_n + 1)) * abs(tan(double(double(((degree+storedpoint.UID+1) * p.x)/360.0) * 2.0 * PI)));
        //# endif
    }
};

struct precalc_colorcycle_t
{
    // requires that the temp object in GenerateTorus is static
    point_t& storedpoint; // so that we can just iterate through colortable to draw
    std::array<colorentry_t, 360> colorcycle;

    precalc_colorcycle_t(point_t& p) : storedpoint(p)
    {
        for (int deg = 0; deg < 360; ++deg)
        {
            colorcycle[deg] = colorentry_t(p,deg);
        }
    }
};

std::vector<precalc_colorcycle_t> colortable{};

Torus_t& GenerateTorus()
{
    static Torus_t result{}; // storedpoint in precalc_colorcycle_t cannot be a reference unless this is static
    colortable.clear(); // not necessary

    for (int strip = 0; strip < numStrips; ++strip)
    {
        for (int ring = 0; ring < numRings; ++ring)
        {
            for (int k = 0; k < numVertecies; ++k)
            {
                const float s = (strip + k + 1) % numStrips + tubeRadius * (k / numVertecies);
                //const float s = (strip + k) % numStrips + 0.5 + float(k/tubeRadius);
                point_t& P = result[strip][ring][k];
                P.strip_n = strip;
                P.ring_n = ring;
                P.k_n = k;

                P.x = (outerRadius + tubeRadius * cos(s * 2.0 * PI / numStrips)) * cos(ring * 2.0 * PI / numRings);
                P.y = (outerRadius + tubeRadius * cos(s * 2.0 * PI / numStrips)) * sin(ring * 2.0 * PI / numRings);
                P.z = tubeRadius * sin(s * 2.0 * PI / numStrips);

                P.UID = index_total;
                colortable.emplace_back(precalc_colorcycle_t(P));
                ++index_total;
            }
        }
    }

    std::cout << "Torus Generated with: " << index_total << " verticies." << std::endl;
    static bool isFirstCall{true};
    assert(isFirstCall);
    isFirstCall = false;

    return result;
}

const Torus_t& Torus = GenerateTorus();

void DrawPregenTorus()
{
    // modulus is undefined for negative values (segfaults)
    const int current_degree = int((rotation_angle > 0)? rotation_angle:-rotation_angle) % 360;
    glBegin(cyclemethod);

    // iterates through Torus arrays (old method)
/*     for (const m_rings& R : Torus)
    {
        for (const m_points& P : R)
        {
            for (const point_t& K : P)
            {
                // base colors (no RGB cycling)
                glVertex3f(K.x, K.y, K.z);
                //glColor3f(K.x, K.y, K.z);

                // RGB cycle effect
                const colorentry_t& C = colortable[K.UID].colorcycle[current_degree];
                //const point_t& otherpoint = Torus[K.strip_n][(K.ring_n+2)%numRings][K.k_n];
                const point_t& otherpoint = Torus[K.strip_n][(K.ring_n+2)%numRings][(K.k_n+1)%numVertecies];
                
                glVertex3f(otherpoint.x, otherpoint.y, otherpoint.z);
                glColor3f(C.r, C.g, C.b);
            }
        }
    } */

    // iterates through colortable (new method)
    for (const precalc_colorcycle_t& C : colortable)
    {
        const colorentry_t& E = C.colorcycle[current_degree];
        const point_t& P = C.storedpoint;

        glVertex3f(P.x, P.y, P.z);
        //glColor3f(P.x, P.y, P.z); // color without any RGB cycling
        glColor3f(E.r, E.g, E.b);

        //const point_t& otherpoint = Torus[P.strip_n][(P.ring_n+2)%numRings][P.k_n];
        const point_t& otherpoint = Torus[(P.strip_n+2)%numStrips][(P.ring_n+3)%numRings][(P.k_n+1)%numVertecies];
        //const point_t& otherpoint = Torus[(P.strip_n+((P.k_n + P.strip_n)/numRings)+1)%numStrips][(P.ring_n+((P.k_n + P.strip_n + P.ring_n)/numStrips)+1)%numRings][P.k_n];
        glVertex3f(otherpoint.x + 0.025, otherpoint.y + 0.025, otherpoint.z + 0.025);
        glColor3f(E.g, E.b, E.r); // intentionally shuffling the order
        
/*      const point_t& otherpointtwo = Torus[(P.strip_n+2)%numStrips][(P.ring_n+3)%numRings][(P.k_n+1)%numVertecies];
        glColor3f(E.r, E.g, E.b);
        glVertex3f(otherpointtwo.x, otherpointtwo.y, otherpointtwo.z); */

        //const point_t& backwardspoint = colortable[((P.UID == 0)? index_total-1 : (P.UID-1))].storedpoint; // going backwards
        //glVertex3f(backwardspoint.x, backwardspoint.y, backwardspoint.z);
        //glColor3f(E.r, E.g, E.b);
        //glColor3f(P.x, P.y, P.z); // color without any RGB cycling

    }

    glEnd();
}
#endif

#ifndef PRECALCULATE_TORUS_GEOMETRY
void drawTorus()
{
    glBegin(cyclemethod);

    for (int strip = 0; strip < numStrips; ++strip)
    {
        for (int ring = 0; ring < numRings; ++ring)
        {
            for (int k = 0; k < numVertecies; ++k)
            {
                float s = (strip + k) % numStrips + float(k / numVertecies);

                float x = (outerRadius + tubeRadius * cos(s * 2.0 * PI / numStrips)) * cos(ring * 2.0 * PI / numRings);
                float y = (outerRadius + tubeRadius * cos(s * 2.0 * PI / numStrips)) * sin(ring * 2.0 * PI / numRings);
                float z = tubeRadius * sin(s * 2.0 * PI / numStrips);

                glColor3f(x, y, z);
                glVertex3f(x, y, z);
            }
        }
    }

    glEnd();
}
#endif

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity(); // replaces current matrix with identity (default) matrix

    if (shouldCycleRendermethods)
    {
        // modulus is undefined for negative values (segfaults)
        const int currentmethod_index = int(((rotation_angle > 0)? rotation_angle:-rotation_angle) / 360) % 9;
        if (cyclemethod != rendermethods[currentmethod_index])
        {
            cyclemethod = rendermethods[currentmethod_index];
            std::cout << "rendermethod: " << currentmethod_index << std::endl;

            // speed up rotation to compensate FPS-drops on certain rendermethods
            //rotation_delta = (((currentmethod_index == 6) || (currentmethod_index == 8))? rotation_delta_default * 3.0 : rotation_delta_default);
        }
    }

    rotation_angle += rotation_delta;

    glTranslatef(0.0f, 0.0f, -5.0f); // z-coord must be negative to be in front of camera
    glRotatef(rotation_angle, 1.0f, 1.2f, 1.0f);
    glRotatef(rotation_angle, 0.0f, 0.0f, 1.0f);
    

    //glRotatef(rotation_angle, 0.0f, 1.0f, 0.0f);
    //glRotatef(rotation_angle, 1.0f, 0.0f, 0.0f);
    // glRotatef(rotation_angle, 0.0f, 0.0f, 0.0f); // very strange; it flips and shrinks back and forth
    // glRotatef(0, 1.0f, 1.0f, 1.0f); // equivalent to doing nothing
    /* glRotatef applies the parameters through a matrix-multiplication operation, which is why passing zeroes eventually inverts the shape.*/

#ifdef PRECALCULATE_TORUS_GEOMETRY
    DrawPregenTorus();
#else
    drawTorus();
#endif

// nested torus
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -4.4f); // must reapply this
    glRotatef(rotation_angle, 1.0f, 1.2f, 1.0f);
    glRotatef(rotation_angle * -1.5, 0.0f, 0.0f, 1.0f);
    DrawPregenTorus();


    glFlush();         // not necessary, but seems to slightly improve performance
    glutSwapBuffers(); // no effect unless double-buffered?

    glutPostRedisplay(); // marks the current window as needing to be redisplayed
}


void idle()
{
    if (ShouldStopRendering)
    {
        glutLeaveMainLoop();
        return;
    }
    
    // # define TILT_INSTEAD // tilt back and forth instead of spinning
    #ifdef TILT_INSTEAD
    if (rotation_angle >= 360.0f)
    {
        rotation_delta = -0.01f;
    }
    
    else if (rotation_angle <= 0.0f)
    {
        rotation_delta = 0.015f;
    }
    #endif // TILT INSTEAD
    
    //PRINTGLUT(GLUT_ELAPSED_TIME);  // super spam
    return;
}

// the function signature requires parameters to be integers,
// otherwise we'd pass them as doubles to avoid recasting in 'gluPerspective'
void reshape(int width, int height)
{
    glViewport(0, 0, width, height); // necessary for scaling after a window resize
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(36.0, static_cast<double>(width) / height, 1.0, 10.0);
    glMatrixMode(GL_MODELVIEW); // this can go in 'display' function instead
}

// moved from the main function so that it can be executed in another thread
void GlutStuff(int argc, char** argv)
{
    // "depth testing (GL_LESS) should be enabled for correct drawing of the nonconvex objects, i.e. the glutTorus..."
    // https://freeglut.sourceforge.net/docs/api.php#GeometricObject
    
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(960, 960);
    glutInit(&argc, argv); // all 'glutInit' functions must be called BEFORE this call?
    // some options to glutInit include: '-gldebug', '-sync' (neither of which seems to be working?)
    
    // must be called after glutInit
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION); // prevents glut from terminating the entire program when it's window is closed
    //glutSetOption(GLUT_GEOMETRY_VISUALIZE_NORMALS, true); //only affects glut's built-in object-rendering functions (glutTorus)
    glutCreateWindow("Torus");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    //int game_mode_status = glutEnterGameMode();
    //std::cout << "game_mode_status: " << game_mode_status << '\n';
    // freezes the entire desktop (until killed) and reports a status of '2'
    
    //from the nvidia-settings profiles, the flag name for vsync is: GLSyncToVblank;
    //in fg_init.c, it's the 'XSyncSwitch' member (GLboolean) of the 'fgState' struct
    
    PRINTGLUT(GLUT_INIT_DISPLAY_MODE);
    
    // mainloop must be called in the same thread / (function?) as the glutInit (and glutCreateWindow?) calls.
    glutMainLoop(); // by default, never returns - terminates the program when window closes.
    ShouldStopRendering = true; // indicates that the window has already been closed
    std::cout << "rejoined main thread \n";
    return;
}


// Uses code taken directly from freeglut (fg_init.c) to ensure cmdline args are getting matched
// because it seems to be ignoring them
#ifdef TESTCMDLINEPARSING
#include <string.h> //strcmp
bool TestCmdlineParsing(int argc, char** argv) 
{
    bool found_sync_arg = false;
    std::cout << "\n\n argv: \n";
    
    for (int i = 0; i <= argc; ++i)
    {
        if (argv[i] == NULL)
        {
            std::cout << "NULL\n";
            continue;
        }
        std::cout << argv[i] << '\n';
        
        if( strcmp( argv[ i ], "-gldebug" ) == 0 )
        {
            //fgState.GLDebugSwitch = GL_TRUE;
            std::cout << "gldebug arg found!\n";
        }
        else if( strcmp( argv[ i ], "-sync" ) == 0 )
        {
            found_sync_arg = true;
            std::cout << "glsync arg found!\n";
            //fgState.XSyncSwitch = GL_TRUE;
        }
    }
    return found_sync_arg;
}
#endif

// This version of main launches only the Torus Window in a seperate thread
//#define THIS_IS_MAIN_FILE // undefine (define in main.cpp) for ImGUI window
#ifdef THIS_IS_MAIN_FILE
int main(int argc, char** argv, char** envp) {
    #ifdef TESTCMDLINEPARSING
    bool argsfound = TestCmdlineParsing(argc, argv);
    if (!argsfound) {
        std::cout << "ragequitting because syncarg was not found!\n";
        return 1;
    }
    #endif
    
    #define ENVTEST // try to set the envvar 'GLUT_FPS' (makes glut print FPS averages on std::cerr)
    #ifdef ENVTEST
    std::cout << "argc = " << argc << '\n';
    std::cout << "&argv = " << argv << '\n';
    std::cout << "&envp = " << envp << '\n';
    
    bool secondloop = false;
    constexpr bool looptwice = true;
    SECONDLOOP:  // reprinting all the argv/envp vars to see the changes
    std::cout << "\n\n argv: \n";
    for (int i = 0; i <= argc; ++i)
    {
        if (argv[i] == NULL)
        {
            std::cout << "NULL\n";
            continue;
        }
        std::cout << argv[i] << '\n';
    }
    
    const std::string ProgramName = "TORUS";
    if (!std::string(argv[0]).ends_with(ProgramName)) {
        std::cerr << "warning: argv[0] does not match expected program name\n";
    }
    
    // setenv (and unsetenv, putenv) are NOT included in <cstdlib>
    // if you want setenv, you'll need to include the C <stdlib.h> header
    // std::setenv("__GL_SYNC_TO_VBLANK", "1", false); // name, value, overwrite_existing
    
    char epic[16] = "GLUT_FPS=1000\000";
    //envp[-1] = epic; // I'm a lunatic.
    // prepending envvar (which overwrites the NULL-terminator of argv) is actually unnecessary
    // (also negative array indecies are undefined behavior lmao)
    
    char vsync_envvar[23] = "__GL_SYNC_TO_VBLANK=1\000";  // might be Nvidia-specific?
    bool print_envline = false;
    
    std::cout << "\n\n envp: \n";
    // if (envp == NULL) {std::cout << "env is NULL\n";} // probably impossible
    if (!print_envline) { std::cout << "--- [global environment variables omitted] ---\n"; }
    for (int i = 0; envp[i] != NULL; ++i)
    {
        // printing the shell's entire (global) environment is spam, so we try to isolate the explicitly-passed variables
        // arguments passed on the command-line are always? listed before the system envvars.
        /* if (std::string(envp[i]) == "SHELL=/bin/bash") // assuming this is always the first system envvar
        // string conversion is necessary; for some reason direct comparison doesn't work (even with null-terminated string)
        {
            std::cout << "---[global environment variables omitted for simplicity]---\n";
            print_envline = false; // everything after this is a globally-set variable
        } // comment out this block if you want to see all of the envvars */
        
        if (std::string envvar = std::string(envp[i]); 
            envvar.starts_with("_=") && envvar.ends_with(ProgramName)) {
            //std::cout << "match for argv found!\n";
            //std::cout << "equal addresses: " <<std::boolalpha << (&envp[i] == argv) << '\n';
            print_envline = true; // turn it back on after skipping the globals
            // assumes that the program name is the final envvar (or close to it)
        }
        if (print_envline) std::cout << "[" << &envp[i] << "]: " << envp[i] << '\n';
        if (secondloop) continue;
        
        // appending envvar
        if (envp[i+1] == NULL) {
            //std::cout << "\nfound NULL; appending envvar\n";
            envp[i+1] = epic;
            envp[i+2] = vsync_envvar;
            envp[i+3] = NULL;
            break; // otherwise infinite loop
        }
    }
    
    if (!secondloop)
    {
        secondloop = true;
        const std::vector<const char*> envstrings {"GLUT_FPS", "__GL_SYNC_TO_VBLANK"};
        for (const char* envstring: envstrings)
        {
            if (const char* maybewin = std::getenv(envstring))
                 std::cout << "ENVTEST " << envstring << " success! value: " << maybewin << std::endl;
            else std::cout << "ENVTEST " << envstring << " failed: std::getenv did not find variable\n";
            
            if (looptwice)  {
                std::cout << "\n\n SECONDLOOP \n\n";
                goto SECONDLOOP;  // only on success; otherwise it would loop forever
            }
        }
    }
    std::cout << "\n\n";
    #endif  // ENVTEST
    // if the ENVTEST worked, you'll see periodic messages in the form of:
    // "freeglut: 4552 frames in 1.00 seconds = 4547.45 FPS"
    
    GlutStuff(argc, argv);
    std::jthread testThread{GlutStuff, argc, argv};
    
    std::cout << "glut thread launched!\n";
    
    std::cout << "main  thread_id:" << std::this_thread::get_id() << '\n';
    std::cout << "torus thread_id:" << testThread.get_id() << '\n';
    std::cout << std::boolalpha << "    joinable: " << testThread.joinable() << '\n';

    /* while (testThread.joinable() && !ShouldStopRendering) {
        std::cout << "sleep \n";
        sleep(1);
    } */

    testThread.join();

    std::cout << "main thread finished \n";

    return 0;
}
#endif
