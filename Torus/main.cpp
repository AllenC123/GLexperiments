#define THIS_IS_MAIN_FILE // define here (and undefine in Torus_Rendering.cpp) for ImGUI window
#ifdef THIS_IS_MAIN_FILE
// This version of main creates an ImGui window and launches the TORUS window in another thread

//#include <stdio.h>
#include <thread>
#include <iostream>

#include <GLFW/glfw3.h> // Will drag in system OpenGL headers
//#include <GLES/gl.h>
//#include <GL/glext.h>
//#include <GL/glcorearb.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
//#define GL_SILENCE_DEPRECATION
/* #if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif */

//#include "Torus.hpp"
#include "Torus_Rendering.hpp"


static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


int main(int argc, char** argv, char** envp [[maybe_unused]])
{
    bool isVsync{true}; // This only controls the vsync of the imgui window
    // the easiest way to make the torus window sync is through nvidia-settings lool
    //GLSyncToVblank
    
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
        if (ShouldStopRendering) { glfwSetWindowShouldClose(window, true); break; }
        
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
        
        const int current_degree{int((rotation_angle >= 0)? rotation_angle : -rotation_angle) % 360};
        ImGui::Text(
            ((rotation_angle >= 360.0f) || (rotation_angle <= -360.0f))?
            "rotation angle: %.2f (%3d)" : "rotation angle: %.2f",
            rotation_angle, current_degree
        );
        ImGui::SliderFloat("rotation speed", &rotation_delta, -1.0f, 1.0f, NULL, ImGuiSliderFlags_Logarithmic);
        // TODO: figure something out for rotation-angle controls. probably refactor rendermethod-cycling code
        //ImGui::SliderFloat("rotation angle", &rotation_angle, 0, 3240.f, NULL, ImGuiSliderFlags_NoRoundToFormat); // 3240 == 9*360
        /*const float base_angle{rotation_angle - float((rotation_angle >= 0)? current_degree : -current_degree)};
        if (ImGui::SliderInt("rotation angle", &current_degree, 0, 358, NULL, ImGuiSliderFlags_NoRoundToFormat)) {
            rotation_angle = base_angle + current_degree;
        }*/
        
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    
    //if ((glutThread.joinable()) && (!ShouldStopRendering))
    if (glutThread.joinable()) // we shouldn't need to check 'ShouldStopRendering'
    {
        ShouldStopRendering = true; // tell the rendering window to close itself
        glutThread.join();
    } // we can't directly call 'glutLeaveMainLoop()' here because we didn't initialize glut from this thread
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}

#endif //THIS_IS_MAIN_FILE
