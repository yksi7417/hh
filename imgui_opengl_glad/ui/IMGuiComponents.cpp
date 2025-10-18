#include "IMGuiComponents.h"

#include <cstring>

#include "../core/main_context.h"
#include "MarketDataTable.h"
#include "imgui.h"

void ImGuiComponents::Init(GLFWwindow* window, const char* glsl_version) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look
    // identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Initialize the enhanced market data table
    market_data_table_ = std::make_unique<MarketDataTable>();
    // We'll initialize it with max rows when we have context
    
    // Initialize the navigator
    navigator_ = std::make_unique<Navigator>();
    // We'll initialize it with max rows when we have context
}

void ImGuiComponents::NewFrame() {
    // feed inputs to dear imgui, start new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiComponents::Update(HostContext& ctx, const HostMDSlot& slot, uint64_t& next_paint_ms) {
    // Initialize components if not done yet
    static bool components_initialized = false;
    if (!components_initialized) {
        if (market_data_table_) {
            market_data_table_->Initialize(ctx.num_rows);
        }
        if (navigator_) {
            navigator_->Initialize(ctx.num_rows);
        }
        components_initialized = true;
    }

    // Create main window with dockspace (similar to imgui_basic)
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("MainWindow", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    // Create dockspace
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    ImGui::End();  // End MainWindow

    // Update and render components - both share the same data source
    uint64_t t = now_ms();
    bool should_refresh = (t >= next_paint_ms);

    // Render Navigator (left side, dockable)
    if (navigator_) {
        navigator_->Render(ctx, slot);
    }

    // Render Market Data Table (right side, dockable)
    if (market_data_table_) {
        market_data_table_->UpdateFromContext(ctx, slot, should_refresh);
        market_data_table_->Render(ctx, slot);
    }

    if (should_refresh) {
        next_paint_ms = t + 250;
    }
}

void ImGuiComponents::Render() {
    // Render dear imgui into screen
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void ImGuiComponents::Shutdown() {
    // Cleanup components
    market_data_table_.reset();
    navigator_.reset();

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
