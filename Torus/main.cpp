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


int main(int argc, char** argv, char** envp [[maybe_unused]])
{
    bool isVsync{true}; // This only controls the vsync of the imgui window - not the TORUS window
    
    //from 'Torus.hpp'
    //Torus_T Torus{};
    //Torus.Create();
    
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
    
    GLFWwindow* window = glfwCreateWindow(400, 240, "Torus_GUI", nullptr, nullptr);
    if (window == nullptr) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    ImGuiIO& imguiIO = ImGui::GetIO();
    imguiIO.IniFilename = NULL; // disable autosaving of the 'imgui.ini' config file (just stores window states)
    imguiIO.ConfigWindowsMoveFromTitleBarOnly = true;
    imguiIO.ConfigWindowsResizeFromEdges = false;
    imguiIO.WantCaptureMouse = false;
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
    ImVec4 clear_color = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    
    std::jthread glutThread{GlutStuff, argc, argv};
    std::cout << "glut thread launched!\n";
    
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
        ImGui::Text("RenderMethod: %s", (RenderMethodName(renderMethod).c_str()));
        ImGui::Text("Auto-Cycling: %sabled",(shouldCycleRenderMethod?"en":"dis"));
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
