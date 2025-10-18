#pragma once

#include <glad/glad.h>  // MUST be included before any OpenGL headers (including GLFW)
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "GLFW/glfw3.h"

#include <cstdint>
#include <memory>

#include "MarketDataTable.h"
#include "Navigator.h"

// Forward declarations
struct HostContext;
struct HostMDSlot;

class ImGuiComponents {
  public:
    void Init(GLFWwindow* window, const char* glsl_version);
    void NewFrame();
    void Update(HostContext& ctx, const HostMDSlot& slot, uint64_t& next_paint_ms);
    void Render();
    void Shutdown();

  private:
    std::unique_ptr<MarketDataTable> market_data_table_;
    std::unique_ptr<Navigator> navigator_;
};