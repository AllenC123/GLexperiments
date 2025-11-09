//____________________________________________________________________________//
// PROGRAM FLAGS
//____________________________________________________________________________//

// use the 'main' function in this file as program entry-point - single window
// disable this define (and define in main.cpp instead) to get an ImGUI window
//#define THIS_IS_MAIN_FILE

// set the envvar 'GLUT_FPS' (which makes glut print FPS averages on std::cerr)
// and also attempt to force vSync: '__GL_SYNC_TO_VBLANK=1' (nvidia GPUs only?)
//#define ENVTEST
// if the ENVTEST worked, you'll see periodic messages in the form of:
//      "freeglut: 4552 frames in 1.00 seconds = 4547.45 FPS"

// run the TORUS window in a seperate thread - only useful for testing
//#define THREAD_TEST

//____________________________________________________________________________//

#include "Torus_Rendering.hpp"

#include <cmath> // 'sin' and 'cos'
#include <array>
#include <vector>
#include <cassert>
#include <iostream>

#include <GL/glut.h> //should just specify freeglut instead
#include <GL/freeglut_ext.h> // required for glutSetOption (handling window-closing event properly)

int TORUS_WINDOW_ID{0};
bool TORUS_VSYNC{true};
bool vsync_initial{TORUS_VSYNC};

bool shouldStopRendering{false};
bool shouldApplyRotation {true};
float rotation_angle{0.000000f};
float rotation_delta{default_rotation_delta};

constexpr std::array<GLenum,10> RenderMethods {
    GL_POINTS, GL_LINES, GL_LINE_LOOP, GL_LINE_STRIP,
    GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN,
    GL_QUADS, GL_QUAD_STRIP, GL_POLYGON,
}; // 'GL_TRIANGLE_FAN' and 'GL_POLYGON' are both slow (and jank), but 'GL_POLYGON' seems a bit faster.

bool shouldCycleRenderMethod{true};
std::size_t renderMethodIndex{ 3 }; // GL_LINE_STRIP
GLenum renderMethod{RenderMethods[renderMethodIndex]};

// [GLenum] GL_LINES --> "GL_LINES" [str]
std::string RenderMethodName(GLenum RM) {
  #define CASE(_E) case _E: {return #_E;}
  switch (RM) {
    CASE(GL_POINTS)
    CASE(GL_LINES)
    CASE(GL_LINE_LOOP)
    CASE(GL_LINE_STRIP)
    CASE(GL_TRIANGLES)
    CASE(GL_TRIANGLE_STRIP)
    CASE(GL_TRIANGLE_FAN)
    CASE(GL_QUADS)
    CASE(GL_QUAD_STRIP)
    CASE(GL_POLYGON)
    default: assert(false && "unreachable");
    return "unreachable";
    #undef CASE
  }
}

std::string RenderMethodName(){ return RenderMethodName(renderMethod); }
GLenum GetRenderMethod(std::size_t I) { return RenderMethods[I]; }
GLenum SetRenderMethod(std::size_t I) {
    shouldCycleRenderMethod = false;
    renderMethod = RenderMethods[I];
    renderMethodIndex=I;
    return renderMethod;
}

void NextRenderMethod(bool positive) {
    if ((!positive) && (renderMethodIndex == 0))
    renderMethodIndex = (RenderMethods.size()-1);
    else renderMethodIndex += (positive? 1 : -1);
    
    if (renderMethodIndex >= RenderMethods.size())
        renderMethodIndex = 0;
    
    renderMethod = RenderMethods[renderMethodIndex];
}

void UpdateRenderMethod()
{
    bool positive{(rotation_angle >= 0)};
    if ((rotation_angle <= 360.f) && (rotation_angle >= -360.f)) return;
    rotation_angle = (positive? -360.f : 360.0f); // resetting rotation - NOT to zero - that's a half-rotation off (same position as 360 but flipped)
    assert(positive == (rotation_delta >= 0.0f)); // the logic above is only correct provided that rotation_delta determines which bound is exceeded (+-360); otherwise an infinite loop may occur
    if (!shouldCycleRenderMethod) return;
    
    NextRenderMethod(positive);
    #ifdef THIS_IS_MAIN_FILE // ImGui window displays renderMethod name/info
    std::cout << "renderMethod: " << RenderMethodName(renderMethod) << '\n';
    #endif
}


#define PRECALCULATE_TORUS_GEOMETRY
#ifdef PRECALCULATE_TORUS_GEOMETRY

int index_total{0};

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
    static bool isFirstCall{true}; assert(isFirstCall);
    if (!isFirstCall) return result;
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
    isFirstCall = false;

    return result;
}

const Torus_t& Torus = GenerateTorus();

void DrawPregenTorus()
{
    // modulus is undefined for negative values (segfaults)
    const int current_degree = int((rotation_angle >= 0)? rotation_angle:-rotation_angle) % 360;
    glBegin(renderMethod);

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
    glBegin(renderMethod);

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


// 'glutenum' here should be one of the GLUT API macro definitions from freeglut_std.h
// starting from 'GLUT_WINDOW_X' and ending with 'GLUT_WINDOW_FORMAT_ID'
#define PRINTGLUT(glutenum) std::cout << #glutenum <<" = " << glutGet(glutenum) << '\n';

void idle()
{
    if (shouldStopRendering || (vsync_initial != TORUS_VSYNC)) {
        glutLeaveMainLoop();
        return;
    }
    
    if (shouldApplyRotation) {
        rotation_angle += rotation_delta;
        UpdateRenderMethod();
    }
    
    //PRINTGLUT(GLUT_ELAPSED_TIME);  // super spam
    return;
}


void PrintArgs(const int argc, char** argv) {
    std::cout << "argc: " << argc << '\n';
    for (int I{0}; I<argc; ++I) {
        std::cout << "arg[#" << I << "] = ";
        if (!argv[I]) std::cout << "[NULL]";
        else std::cout << '"'<<argv[I]<<'"';
        std::cout << '\n';
    } std::cout << '\n';
}

// GLUT doesn't actually provide any mechanism for toggling double-buffering of an existing window
// so it's necessary to destroy the current window and then recreate it with a new glutDisplayMode
void GlutToggleVsync(int argc, char** argv)
{
    const bool vSync{ TORUS_VSYNC };
  //unsigned int newMode{((vSync? GLUT_DOUBLE:0U) | GLUT_RGBA | GLUT_DEPTH)};
    std::cout << "\nGlutToggleVsync: " << (vSync? "enabled\n": "disabled\n");
    glutInitWindowPosition(2560, 0);
    glutInitWindowSize(960, 960);
    PrintArgs(argc, argv);
    glutInit(&argc, argv);
    
    // no need to destroy old window - already destroyed by 'glutLeaveMainLoop'?
    // destroying it either terminates the program, or gives an error:
    // "Function <glutDestroyWindow> called without first calling 'glutInit'"
    //int OLD_WINDOW_ID = TORUS_WINDOW_ID;
    //glutDestroyWindow(OLD_WINDOW_ID);
    
  //glutSetOption(GLUT_INIT_DISPLAY_MODE, newMode);
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
    glutInitDisplayMode((TORUS_VSYNC?GLUT_DOUBLE:0U) | GLUT_RGBA | GLUT_DEPTH);
    TORUS_WINDOW_ID = glutCreateWindow((TORUS_VSYNC? "TORUS [vSync]":"TORUS"));
    //glutPushWindow(); // lowers window in stacking order (NEVER USE HERE!!!)
    //glutSetWindow(TORUS_WINDOW_ID);
    
    glEnable(GL_DEPTH_TEST); // disabling this creates an interesting effect
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // these all need to be set again for the new window
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    return;
}

// moved from the main function so that it can be executed in another thread
void GlutStuff(int argc, char** argv)
{
    // window parameters from commandline:
    //"./TORUS -geometry 1080x1080+1920+0"
    std::cout << "\nGlutStuff\n";
    PrintArgs(argc, argv);
    
    // must create copies to preserve the original commandline, to ensure that any
    // specified window/display parameters are applied on each call to 'glutInit()'
    // otherwise cmdline parameters only work once, because 'glutInit' erases them
    static int saved_argc = argc;
    char* saved_argv[saved_argc]; // "Warning: ISO C++ forbids variable length array ‘saved_argv’"
    for (int I{0}; I<argc; ++I) {
        saved_argv[I] = argv[I];
    } //saved_argv[I] = *(new char*{argv[I]}); // still crashes!?
    // still randomly crashes sometimes with 'BadRequest' X-Error; always Major/Minor opcode 138/5
    // I'm guessing the random crashes happen because 'argv' has pointers belonging to main thread
    
    // "depth testing (GL_LESS) should be enabled for correct drawing of the nonconvex objects, i.e. the glutTorus..."
    // https://freeglut.sourceforge.net/docs/api.php#GeometricObject
    glutInitDisplayMode((TORUS_VSYNC? GLUT_DOUBLE:0U) | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(2560, 0);
    glutInitWindowSize(960, 960);
    glutInit(&argc, argv); // all 'glutInit' functions must be called BEFORE this call?
    // some options to glutInit include: '-gldebug', '-sync' (neither of which seems to be working?)
    
    // must be called after glutInit
    //glutSetOption(GLUT_GEOMETRY_VISUALIZE_NORMALS, true); //only affects glut's built-in object-rendering functions (glutTorus)
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION); // prevents glut from terminating the entire program when it's window is closed
    TORUS_WINDOW_ID = glutCreateWindow((TORUS_VSYNC? "TORUS [vSync]":"TORUS"));
    
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
    
    PRINTGLUT(GLUT_VERSION);
    PRINTGLUT(GLUT_INIT_MAJOR_VERSION);
    PRINTGLUT(GLUT_INIT_MINOR_VERSION);
    PRINTGLUT(GLUT_INIT_FLAGS);
    PRINTGLUT(GLUT_INIT_PROFILE);
    PRINTGLUT(GLUT_INIT_DISPLAY_MODE);
    
    //glutPushWindow(); // lowers window in stacking order
    
    // mainloop must be called in the same thread / (function?) as the glutInit (and glutCreateWindow?) calls.
    glutMainLoop(); // by default, never returns - terminates the program when window closes.
    while ((!shouldStopRendering) && (vsync_initial != TORUS_VSYNC)) {
        argc = saved_argc; // restoring initial args before 'glutInit'
        for (int I{0}; I<argc; ++I) { argv[I] = saved_argv[I]; }
        GlutToggleVsync(argc, argv);
        vsync_initial = TORUS_VSYNC;
        glutMainLoop();
    }
    shouldStopRendering = true; // indicates that the window has already been closed
    return;
}


//#define TESTCMDLINEPARSING // Simulates the parsing behavior of 'glutInit()' (without any actual effect)
#ifdef TESTCMDLINEPARSING
#include <string.h> //strcmp
#include <cstdlib> // std::getenv

// checking the cmdline args for vsync and debug flags ('-sync' and '-gldebug'), which eventually get passed into 'glutInit()' (theoretically)
// Uses code taken directly from freeglut (fg_init.c) to ensure cmdline args are getting matched
// because it seems to be ignoring them
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


#ifdef THIS_IS_MAIN_FILE
#ifdef THREAD_TEST
#include <thread>
#endif
// This version of main only runs the Torus window
int main(int argc, char** argv, char** envp [[maybe_unused]])
{
    #ifdef TESTCMDLINEPARSING
    bool argsfound = TestCmdlineParsing(argc, argv);
    if (!argsfound) {
        std::cout << "ragequitting because 'sync' arg was not found!\n";
        return 1;
    }
    #endif
    
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
    
    #ifdef THREAD_TEST
    std::jthread testThread{GlutStuff, argc, argv};
    std::cout << "glut thread launched!\n";
    std::cout << "main  thread_id:" << std::this_thread::get_id() << '\n';
    std::cout << "torus thread_id:" << testThread.get_id() << '\n';
    std::cout << std::boolalpha << "    joinable: " << testThread.joinable() << '\n';
    
    /* while (testThread.joinable() && !shouldStopRendering) {
        std::cout << "sleep \n";
        sleep(1);
    } */
    
    testThread.join();
    std::cout << "rejoined main thread \n";
    std::cout << "main thread finished \n";
    #else
    GlutStuff(argc, argv);
    #endif
    return 0;
}
#endif
