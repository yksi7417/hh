#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class ImGuiComponentList {
public:
  void Init(GLFWwindow* window, const char* glsl_version);
  void NewFrame();
  void Update();
  void Render();
  void Shutdown();
};