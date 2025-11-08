// This version of main creates an ImGui window and launches the TORUS window in another thread
#define THIS_IS_MAIN_FILE // define here (and undefine in Torus_Rendering.cpp) for ImGUI window
#ifdef THIS_IS_MAIN_FILE

#include <iostream>
#include <thread>

#include <GLFW/glfw3.h> // Will drag in system OpenGL headers

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Torus_Rendering.hpp"
#ifdef ENABLE_TORUS_FILES
#include "Torus.hpp" // unused
#endif


static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// apparently numpad keys have different 'key' values
void KeypressCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
  // 'action' 1: keypress, 0: keyrelease
  if(!action) return;
  switch(key) {
    case 'Q': case 'q':
    glfwSetWindowShouldClose(window, 1);
      shouldStopRendering = true; break;
    case '0': SetRenderMethod(0); break;
    case '1': SetRenderMethod(1); break;
    case '2': SetRenderMethod(2); break;
    case '3': SetRenderMethod(3); break;
    case '4': SetRenderMethod(4); break;
    case '5': SetRenderMethod(5); break;
    case '6': SetRenderMethod(6); break;
    case '7': SetRenderMethod(7); break;
    case '8': SetRenderMethod(8); break;
    case '9': SetRenderMethod(9); break;
    case '-': NextRenderMethod(false); break;
    case '=': NextRenderMethod(true ); break;
    default : break;
  }
}
// TODO: apply keybinds to TORUS window. also handle numpad

int main(int argc, char** argv, char** envp [[maybe_unused]])
{
    bool isVsync{true}; // This only controls the vsync of the imgui window - not the TORUS window
    assert(IMGUI_CHECKVERSION() && "ImGui version-check failed!");
    std::cout << "using imgui v" << IMGUI_VERSION << '\n';
    
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;
    
    // Decide GL+GLSL versions
    #if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    #else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
    #endif
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    //from 'Torus.hpp'
    //Torus_T Torus{};
    //Torus.Create();
    
    std::jthread glutThread{GlutStuff, argc, argv};
    std::cout << "glut thread launched!\n";
    
    // create this window AFTER launching GLUT thread so the GUI is stacked on top
    GLFWwindow* window = glfwCreateWindow(450, 300, "Torus_GUI", nullptr, nullptr);
    if (window == nullptr) { shouldStopRendering=true; glutThread.join(); return 1; }
    glfwSetKeyCallback(window, KeypressCallback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    std::cout <<
    "\nKeybinds:\n" <<
    "    Q: exit\n" <<
    "  -/+: prev/next render-method\n" <<
    "[0-9]: switch to render-method\n" <<
    "\n";
    
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    ImGuiIO& imguiIO = ImGui::GetIO();
    imguiIO.IniFilename = NULL; // disable autosaving of the 'imgui.ini' config file (just stores window states)
    imguiIO.ConfigWindowsMoveFromTitleBarOnly = true;
    imguiIO.ConfigWindowsResizeFromEdges = false;
    //imguiIO.WantCaptureMouse = false;
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    
    bool imguiTransparentWindow {false}; // ImGuiWindowFlags_NoBackground
    const ImGuiWindowFlags window_flags {
        ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_NoScrollWithMouse
    };
    
    ImGui::StyleColorsDark();
    ImVec4 clear_color{0.25f, 0.25f, 0.25f, 1.0f};
    const ImVec4 currentMethodColor {0.25f, 0.5f, 1.f, 1.f}; // renderMethod button color (highlighted)
    
    while (!glfwWindowShouldClose(window))
    {
        // close this window when the TORUS window has been closed
        if (shouldStopRendering) { glfwSetWindowShouldClose(window, true); break; }
        
        glfwPollEvents(); // necessary for handling window-events (like resize and window-close)
        glClearColor((clear_color.x*clear_color.w), (clear_color.y*clear_color.w), (clear_color.z*clear_color.w), clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        ImGui::Begin("Main_GUI", nullptr, (imguiTransparentWindow? (window_flags|ImGuiWindowFlags_NoBackground) : window_flags));
        ImGui::SetWindowPos({0,0}); ImGui::SetWindowSize({-1,-1}); // these are actually necessary (WTF imgui?)
        
        ImGui::Text("%8.1f FPS [%.3f ms/frame]", imguiIO.Framerate, 1000.f/imguiIO.Framerate);
        if (ImGui::Button("Vsync Toggle")) { isVsync = !isVsync; glfwSwapInterval(isVsync); }
        ImGui::SameLine(); ImGui::Text("Vsync: %s", (isVsync? "enabled" : "disabled"));
        ImGui::Separator();
        
        ImGui::ColorEdit3("background color", (float*)(&clear_color));
        ImGui::Checkbox("menu transparency", &imguiTransparentWindow);
        ImGui::Separator();
        
        ImGui::Separator();
        ImGui::Text("RenderMethod: %s (%lu)", RenderMethodName().c_str(), renderMethodIndex);
        ImGui::Text("Auto-Cycling:"); ImGui::SameLine();
        if (ImGui::Button((shouldCycleRenderMethod? "enabled##cycling": "disabled##cycling"))) shouldCycleRenderMethod = !shouldCycleRenderMethod;
        if (ImGui::Button("prev")){ NextRenderMethod(false); shouldCycleRenderMethod = true; } ImGui::SameLine();
        if (ImGui::Button("next")){ NextRenderMethod(true ); shouldCycleRenderMethod = true; } ImGui::SameLine();
        for (std::size_t RM{0}; RM < 10;) { bool isActive{(RM == renderMethodIndex)};
            if (isActive) ImGui::PushStyleColor(ImGuiCol_Button, currentMethodColor);
            if (ImGui::Button(std::string{char(48+RM)}.c_str())) SetRenderMethod(RM);
            if (isActive){ImGui::PopStyleColor();} if(++RM < 10) ImGui::SameLine();
            //if (ImGui::Button(RenderMethodName(GetRenderMethod(RM)).substr(3).c_str())) SetRenderMethod(RM);
            //if ((++RM % 5) != 0) ImGui::SameLine();
        }
        ImGui::Separator();
        
        if (ImGui::SliderFloat("rotation angle", &rotation_angle, -360.f, 360.f,
            NULL, ImGuiSliderFlags_AlwaysClamp|ImGuiSliderFlags_NoRoundToFormat)) {
            shouldApplyRotation = false; rotation_delta = 0.0f;
        }
        
        if (ImGui::SliderFloat("rotation speed", &rotation_delta, -1.0f, 1.0f,
            NULL, ImGuiSliderFlags_Logarithmic)) {
            shouldApplyRotation = true;
        }
        
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    
    //if ((glutThread.joinable()) && (!shouldStopRendering))
    if (glutThread.joinable()) { // shouldn't need to check 'shouldStopRendering'
        shouldStopRendering = true; // tell the rendering window to close itself
        // cannot call 'glutLeaveMainLoop()' directly because glut wasn't initialized from this thread
        glutThread.join(); std::cout << "rejoined main thread\n";
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}

#endif //THIS_IS_MAIN_FILE
