#include <iostream>
#include <format>

#define GL_GLEXT_PROTOTYPES // must be defined to enable shader-related functions (glUseProgram)
// it is also required to import definitions for 'glGen'/'glBind' functions
#include <GLFW/glfw3.h>

#include "VertexT.hpp"
#include "ShaderLoading.hpp"


bool shader_toggled{false}; // shader was just toggled on/off
bool enable_shaders{true}; bool render_with_glDrawArray{true};
static void error_callback(int error, const char* description) { fprintf(stderr, "Error (%d): %s\n", error, description); }
static void key_callback(GLFWwindow* window, int key, int scancode [[maybe_unused]], int action, int mods [[maybe_unused]])
{ if (action != GLFW_PRESS) return;
  switch(key)
  {
    case GLFW_KEY_Q:
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    return;
    
    case GLFW_KEY_SPACE:
    {
      #ifndef DISABLE_SHADERS
      if(mods& GLFW_MOD_SHIFT) {
        shader_toggled = true;
        enable_shaders = !enable_shaders;
      } else
      #endif
      render_with_glDrawArray = !render_with_glDrawArray;
      std::cout << std::format("rendering-method: {} [shaders {}]\n",
        (render_with_glDrawArray?"'glDrawArray'":"'OpenGL' (manual vertex drawing)"),
        (SHADERS_COMPILED? (enable_shaders? "enabled" : "disabled") : "unavailable")
      );
    } return;
    
    default: return;
  }
}


int main(int argc, char** argv)
{
    for (int C{1}; C<argc; C++)
    { // parsing commandline arguments
      std::string arg{argv[C]}; bool known{false};
      std::cout << std::format("arg[#{}]: '{}'\n", C, arg);
      if (arg.starts_with("--cpurender")) { render_with_glDrawArray = enable_shaders = false; known = true; } // TODO: actually parse "--rendering=[...]"
      if (arg.starts_with("--drawarray")) { enable_shaders = false; render_with_glDrawArray = known = true; } // drawing with shaders is already default
      if (arg.starts_with("--noshaders") // TODO: actually parse "--shaders=[...]" (allowing selection of alternate shaders, or 'none')
      || (arg == ("--disable-shaders"))) { enable_shaders = false; known = true; }
      if ((!known) || (arg == "--help")) {
      if (arg != "--help") std::cerr << std::format("[ERROR] unrecognized argument: '{}'\n", arg);
        typedef std::array<std::string, 2> UsageString; std::array<UsageString, 4> OptionStrings {
          UsageString{"--cpurender", "switch to manual vertex-rendering (shaders cannot be used)"},
          UsageString{"--drawarray", "sets rendering-method 'glDrawArray' (and disables shaders)"},
          UsageString{"--noshaders", "disables shaders (loading skipped; not compiled or linked)"},
          UsageString{"--disable-shaders", "alias of '--noshaders'"},
        }; std::cerr << "available options:\n";
        for (const auto& [option, description]: OptionStrings) {
          std::cerr << std::format(" {}: {}\n", option, description);
        } std::cerr << '\n';
        if (arg == "--help") {
          std::cerr << "Escape/Q terminates the program\n";
          std::cerr << "Spacebar toggles rendering-method [cpu-render / glDrawArray]\n";
          std::cerr << "Shift + Spacebar toggles shaders on/off\n\n";
        }
        return C;
      }
    }
    
    glfwSetErrorCallback(error_callback); // safe to call before glfwInit
    
    if(!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    
    int win_width{1080}, win_height{1080}; // updated inside frameloop
    GLFWwindow* window = glfwCreateWindow(win_width, win_height, "OpenGL Triangle", NULL, NULL);
    if (!window) { glfwTerminate(); return 2; }
    
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    const std::array<VertexT, 3> vertex_list {
      VERTEX({-0.75f, 0.67f}, {1.f, 0.f, 0.f}),
      VERTEX({ 0.75f, 0.67f}, {0.f, 1.f, 0.f}),
      VERTEX({ 0.0f, -0.75f}, {0.f, 0.f, 1.f}),
    };
    
    std::cout << "vertex array:\n  ";
    for (int I{0}; const VertexT& vertex: vertex_list) {
        std::cout << std::format("vertex(#{}): ", I++);
        std::cout << vertex.Representation() << "\n  ";
    } std::cout << '\n';
    
    
    // manual vertex-data rendering only requires 'glEnableClientState' calls.
    glEnableClientState(GL_VERTEX_ARRAY); glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3,GL_FLOAT, sizeof(VertexT), vertex_list[0].coord.data());
    glColorPointer (3,GL_FLOAT, sizeof(VertexT), vertex_list[0].color.data());
    // pointers are required to call 'glDrawArrays' - not for manual-rendering
    
    GLuint shader_program{};
    if (enable_shaders)
    {
      #ifndef DISABLE_SHADERS
      render_with_glDrawArray = true; std::cout << "rendering-method:'glDrawArrays' [shaders enabled]\n";
      GLuint gl_vertex_array; glGenVertexArrays(1, &gl_vertex_array); glBindVertexArray(gl_vertex_array);
      GLuint vertex_buffer; glGenBuffers(1,&vertex_buffer); glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
      glNamedBufferData(vertex_buffer, sizeof(vertex_list), vertex_list[0].coord.data(), GL_STATIC_DRAW);
      //glBufferData(vertex_buffer, sizeof(vertex_list), vertex_list.data(), GL_STATIC_DRAW);
      
      int vertex_attributes_max; glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &vertex_attributes_max);
      std::cout << "[OpenGL] maximum vertex-attributes supported: " << vertex_attributes_max << "\n\n";
      
      shader_program = CompileShaders();
      //const GLuint shader_program = CompileShaders();
      if (!ValidateShaderProgram(shader_program)) {
          std::cerr << "shader validation failed\n";
          glfwDestroyWindow(window);
          glfwTerminate(); return 3;
      }
      else glUseProgram(shader_program);
      bool attributeLookupFailed{false};
      const GLint vertex_coord_location{glGetAttribLocation(shader_program, "vertex_coord")};
      const GLint vertex_color_location{glGetAttribLocation(shader_program, "vertex_color")};
      
      // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetAttribLocation.xhtml
      //If the named attribute is not an active attribute in the specified program object,
      //  or if the name starts with the reserved prefix "gl_", a value of -1 is returned.
      if (vertex_coord_location == -1) { std::cerr << "shader attribute-lookup failed for: 'vertex_coord'\n"; attributeLookupFailed = true; }
      if (vertex_color_location == -1) { std::cerr << "shader attribute-lookup failed for: 'vertex_color'\n"; attributeLookupFailed = true; }
      if (attributeLookupFailed) { glfwDestroyWindow(window); glfwTerminate(); return 4; }
      
      glEnableVertexArrayAttrib(gl_vertex_array, vertex_coord_location);
      glEnableVertexArrayAttrib(gl_vertex_array, vertex_color_location);
      glVertexAttribPointer(vertex_coord_location, 3, GL_FLOAT, GL_TRUE, sizeof(VertexT), (void*) offsetof(VertexT, coord));
      glVertexAttribPointer(vertex_color_location, 3, GL_FLOAT, GL_TRUE, sizeof(VertexT), (void*) offsetof(VertexT, color));
      // these ^ function-calls associate the currently-active 'GL_ARRAY_BUFFER' with the current 'GL_VERTEX_ARRAY'
    } else if (render_with_glDrawArray) { std::cout << "rendering-method: 'glDrawArrays' [shaders are disabled]\n";
    } else { std::cout << "rendering-method: OpenGL (manually drawing vertex-data) [shaders are disabled]\n"; }
     #else
    std::cerr << "[WARN] shaders cannot be enabled (program was compiled with 'DISABLE_SHADERS' flag set)\n";
    } if (render_with_glDrawArray) { std::cout << "rendering-method: 'glDrawArrays' [shaders unavailable]\n"; }
    else { std::cout << "rendering-method: OpenGL (manually rendering vertex-array) [shaders unavailable]\n"; }
    #endif
    
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // scaling without preserving aspect-ratio (stretching)
        glfwGetFramebufferSize(window, &win_width, &win_height);
        glViewport(0, 0, win_width, win_height);
        
        // ~50% chance 'glDrawArray' renders nothing when shaders are disabled (determined at program-startup)
        if (render_with_glDrawArray) { glDrawArrays(GL_TRIANGLES, 0, vertex_list.size()); }
        else { glBegin(GL_LINE_LOOP); // rendering vertices manually (cannot use shaders)
          for (const VertexT& vertex: vertex_list) {
            glColor3fv (vertex.color.data());
            glVertex3fv(vertex.coord.data());
          } glEnd();
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        if (SHADERS_COMPILED && shader_toggled)
        { if (enable_shaders) { glUseProgram(shader_program); }
          else { glUseProgram(0); // reset
            if (render_with_glDrawArray) {
              glVertexPointer(3, GL_FLOAT, sizeof(VertexT), vertex_list[0].coord.data());
              glColorPointer (3, GL_FLOAT, sizeof(VertexT), vertex_list[0].color.data());
            } glEnableClientState(GL_VERTEX_ARRAY); glEnableClientState(GL_COLOR_ARRAY);
          }
          shader_toggled = false;
        }
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
