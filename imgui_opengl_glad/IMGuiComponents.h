#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cstdint>

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
};