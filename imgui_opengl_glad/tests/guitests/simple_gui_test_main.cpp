// Simple GUI Test Runner using ImGui Test Engine
// Minimal application to run basic GUI tests

#ifdef _MSC_VER
#pragma warning (disable: 4996)     // 'This function or variable may be unsafe'
#endif

#include <cstring>  // for strcmp

#define IMGUI_DEFINE_MATH_OPERATORS
#include <glad/glad.h>  // MUST be included before any OpenGL headers
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Test Engine
#include "imgui_test_engine/imgui_te_engine.h"
#include "imgui_test_engine/imgui_te_ui.h"
#include "imgui_test_engine/imgui_te_utils.h"

// Forward declaration of our test registration function
extern void RegisterSimpleGuiTests(ImGuiTestEngine* engine);

// Simple error callback for GLFW
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void PrintUsage(const char* program_name)
{
    printf("Usage: %s [options]\n", program_name);
    printf("Options:\n");
    printf("  --headless      Run tests headlessly and exit (for CI)\n");
    printf("  --fast          Run tests in fast mode\n");
    printf("  --normal        Run tests in normal mode (default)\n");
    printf("  --cinematic     Run tests in cinematic mode (slow, for demos)\n");
    printf("  --help          Show this help message\n");
}

int main(int argc, char** argv)
{
    // Parse command line arguments
    bool headless_mode = false;
    ImGuiTestRunSpeed run_speed = ImGuiTestRunSpeed_Normal;
    
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--headless") == 0)
        {
            headless_mode = true;
            run_speed = ImGuiTestRunSpeed_Fast; // Headless implies fast
        }
        else if (strcmp(argv[i], "--fast") == 0)
        {
            run_speed = ImGuiTestRunSpeed_Fast;
        }
        else if (strcmp(argv[i], "--normal") == 0)
        {
            run_speed = ImGuiTestRunSpeed_Normal;
        }
        else if (strcmp(argv[i], "--cinematic") == 0)
        {
            run_speed = ImGuiTestRunSpeed_Cinematic;
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            PrintUsage(argv[0]);
            return 0;
        }
        else
        {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            PrintUsage(argv[0]);
            return 1;
        }
    }
    
    printf("Test Mode: %s\n", headless_mode ? "Headless" : "Interactive");
    printf("Run Speed: %s\n", 
           run_speed == ImGuiTestRunSpeed_Fast ? "Fast" :
           run_speed == ImGuiTestRunSpeed_Normal ? "Normal" : "Cinematic");

    // Setup GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }

    // GL 3.3 + GLSL 330
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Simple GUI Test Runner", nullptr, nullptr);
    if (window == nullptr)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return 1;
    }
    printf("Window created successfully\n");
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return 1;
    }
    printf("GLAD initialized successfully\n");

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    printf("ImGui context created\n");

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    printf("ImGui backends initialized\n");

    // Setup style
    ImGui::StyleColorsDark();

    // Setup Test Engine
    ImGuiTestEngine* engine = ImGuiTestEngine_CreateContext();
    ImGuiTestEngineIO& test_io = ImGuiTestEngine_GetIO(engine);
    test_io.ConfigVerboseLevel = ImGuiTestVerboseLevel_Info;
    test_io.ConfigVerboseLevelOnError = ImGuiTestVerboseLevel_Debug;
    test_io.ConfigRunSpeed = run_speed;
    
    // Headless mode configuration
    if (headless_mode)
    {
        printf("Configuring headless mode...\n");
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        test_io.ConfigVerboseLevel = ImGuiTestVerboseLevel_Info;
        test_io.ConfigNoThrottle = true; // Run as fast as possible
    }
    
    printf("Test Engine created\n");

    // Start test engine
    ImGuiTestEngine_Start(engine, ImGui::GetCurrentContext());
    ImGuiTestEngine_InstallDefaultCrashHandler();
    printf("Test Engine started\n");

    // Register our simple tests
    RegisterSimpleGuiTests(engine);
    printf("Tests registered\n");
    
    // In headless mode, queue all tests to run automatically
    if (headless_mode)
    {
        printf("Queueing all tests for headless run...\n");
        ImGuiTestEngine_QueueTests(engine, ImGuiTestGroup_Tests, nullptr);
    }

    printf("Entering main loop...\n");
    // Main loop
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool aborted = false;
    int frame_count = 0;
    
    while (!aborted)
    {
        frame_count++;
        if (frame_count % 60 == 0)
            printf("Frame %d\n", frame_count);
            
        // Check if window should close
        if (glfwWindowShouldClose(window))
            aborted = true;
        
        // In headless mode, check if tests are done
        if (headless_mode)
        {
            if (ImGuiTestEngine_IsTestQueueEmpty(engine))
            {
                printf("All tests completed in headless mode. Exiting...\n");
                aborted = true;
            }
        }
            
        glfwPollEvents();
        
        // Only check engine abort if we're already aborting
        if (aborted && ImGuiTestEngine_TryAbortEngine(engine))
        {
            printf("Engine aborted at frame %d\n", frame_count);
            break;
        }
        
        if (aborted)
            continue;

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Show Test Engine UI (skip in headless mode)
        if (!headless_mode)
        {
            ImGuiTestEngine_ShowTestEngineWindows(engine, nullptr);
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        
        // Post-swap is required for screen capture support
        ImGuiTestEngine_PostSwap(engine);
    }

    // Cleanup
    ImGuiTestEngine_Stop(engine);
    
    // Print test results summary
    if (headless_mode)
    {
        printf("\n=== Test Results Summary ===\n");
        int count_tested = 0, count_success = 0;
        ImGuiTestEngine_GetResult(engine, count_tested, count_success);
        printf("Total Tests: %d\n", count_tested);
        printf("Passed: %d\n", count_success);
        printf("Failed: %d\n", count_tested - count_success);
        printf("===========================\n");
        
        // Return non-zero exit code if any tests failed
        int exit_code = (count_tested == count_success) ? 0 : 1;
        
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        
        ImGuiTestEngine_DestroyContext(engine);

        glfwDestroyWindow(window);
        glfwTerminate();

        return exit_code;
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    ImGuiTestEngine_DestroyContext(engine);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
