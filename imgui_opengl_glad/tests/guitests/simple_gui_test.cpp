// Simple GUI Test using ImGui Test Engine
// This demonstrates the most basic test structure

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_test_engine/imgui_te_context.h"

// Register the simplest possible GUI test
void RegisterSimpleGuiTests(ImGuiTestEngine* e)
{
    ImGuiTest* t = nullptr;

    //-----------------------------------------------------------------
    // ## Test 1: Most Basic Test - Just open a window with text
    //-----------------------------------------------------------------
    t = IM_REGISTER_TEST(e, "simple_gui", "basic_window");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        IM_UNUSED(ctx);
        ImGui::Begin("Simple Test Window", nullptr, ImGuiWindowFlags_NoSavedSettings);
        ImGui::Text("Hello, Test Engine!");
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        // Just verify the window exists
        ctx->SetRef("Simple Test Window");
        IM_CHECK(ctx->GetWindowByRef("") != nullptr);
    };

    //-----------------------------------------------------------------
    // ## Test 2: Basic Button Click
    //-----------------------------------------------------------------
    t = IM_REGISTER_TEST(e, "simple_gui", "button_click");
    struct ButtonTestVars { int ClickCount = 0; };
    t->SetVarsDataType<ButtonTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ButtonTestVars& vars = ctx->GetVars<ButtonTestVars>();
        ImGui::Begin("Button Test", nullptr, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::Button("Click Me"))
            vars.ClickCount++;
        ImGui::Text("Clicked %d times", vars.ClickCount);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ButtonTestVars& vars = ctx->GetVars<ButtonTestVars>();
        ctx->SetRef("Button Test");
        
        // Verify initial state
        IM_CHECK_EQ(vars.ClickCount, 0);
        
        // Click the button
        ctx->ItemClick("Click Me");
        IM_CHECK_EQ(vars.ClickCount, 1);
        
        // Click again
        ctx->ItemClick("Click Me");
        IM_CHECK_EQ(vars.ClickCount, 2);
    };

    //-----------------------------------------------------------------
    // ## Test 3: Simple Checkbox Test
    //-----------------------------------------------------------------
    t = IM_REGISTER_TEST(e, "simple_gui", "checkbox");
    struct CheckboxVars { bool IsChecked = false; };
    t->SetVarsDataType<CheckboxVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        CheckboxVars& vars = ctx->GetVars<CheckboxVars>();
        ImGui::Begin("Checkbox Test", nullptr, ImGuiWindowFlags_NoSavedSettings);
        ImGui::Checkbox("Test Checkbox", &vars.IsChecked);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        CheckboxVars& vars = ctx->GetVars<CheckboxVars>();
        ctx->SetRef("Checkbox Test");
        
        // Check initial state
        IM_CHECK_EQ(vars.IsChecked, false);
        
        // Check the checkbox
        ctx->ItemCheck("Test Checkbox");
        IM_CHECK_EQ(vars.IsChecked, true);
        
        // Uncheck it
        ctx->ItemUncheck("Test Checkbox");
        IM_CHECK_EQ(vars.IsChecked, false);
    };
}
