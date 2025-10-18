// Simple GUI Test using ImGui Test Engine
// This demonstrates the most basic test structure

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_test_engine/imgui_te_context.h"

// Include application headers for Navigator tests
#include "../../ui/Navigator.h"
#include "../../core/main_context.h"

// Test variables to hold Navigator state
struct NavigatorTestVars {
    // Mock data for testing
    HostContext ctx;
    HostMDSlot slot;
    std::vector<int64_t> ts_ns;
    std::vector<int64_t> px_n;
    std::vector<int64_t> qty;
    std::vector<uint8_t> side;
    
    std::unique_ptr<Navigator> navigator;
    bool initialized = false;
    
    void Initialize(uint32_t num_rows = 100) {
        if (initialized) return;
        
        // Setup mock data
        ts_ns.resize(num_rows, 0);
        px_n.resize(num_rows, 0);
        qty.resize(num_rows, 0);
        side.resize(num_rows, 0);
        
        // Initialize context
        ctx.num_rows = num_rows;
        ctx.seq = std::make_unique<std::atomic<uint32_t>[]>(num_rows);
        for (uint32_t i = 0; i < num_rows; ++i) {
            ctx.seq[i].store(0, std::memory_order_relaxed);
        }
        ctx.dirty.assign(num_rows, 0);
        ctx.last.resize(num_rows);
        ctx.q.init(1u << 18);
        
        // Initialize slot
        slot.num_rows = num_rows;
        slot.ts_ns = ts_ns.data();
        slot.px_n = px_n.data();
        slot.qty = qty.data();
        slot.side = side.data();
        slot.user = &ctx;
        
        initialized = true;
        
        // Initialize Navigator directly (Test Engine provides ImGui context)
        navigator = std::make_unique<Navigator>();
        navigator->Initialize(num_rows);
    }
    
    void Cleanup() {
        if (initialized) {
            navigator.reset();
            initialized = false;
        }
    }
};

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

    //-----------------------------------------------------------------
    // ## Navigator Tests
    //-----------------------------------------------------------------
    
    // Test 4: Navigator Window Exists and Shows Content
    t = IM_REGISTER_TEST(e, "simple_gui", "navigator_window");
    t->SetVarsDataType<NavigatorTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        NavigatorTestVars& vars = ctx->GetVars<NavigatorTestVars>();
        vars.Initialize();
        
        // Render the Navigator
        if (vars.navigator) {
            vars.navigator->Render(vars.ctx, vars.slot);
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        // Check if the Navigator window exists
        ctx->SetRef("Navigator");
        ImGuiWindow* window = ctx->GetWindowByRef("");
        IM_CHECK(window != nullptr);
        
        // Verify window contains some content
        IM_CHECK(window->DrawList->CmdBuffer.Size > 0);
    };
    
    // Test 5: Navigator Shows Data Categories Tree
    t = IM_REGISTER_TEST(e, "simple_gui", "navigator_categories");
    t->SetVarsDataType<NavigatorTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        NavigatorTestVars& vars = ctx->GetVars<NavigatorTestVars>();
        vars.Initialize(50);
        
        // Add some test data with varied sides
        for (int i = 0; i < 50; i++) {
            vars.ts_ns[i] = 1000000 * (i + 1);
            vars.px_n[i] = 10000 + i * 100;
            vars.qty[i] = (i + 1) * 10;
            vars.side[i] = (i % 3 == 0) ? 1 : 2;
            vars.ctx.seq[i].store(1, std::memory_order_relaxed);
        }
        
        if (vars.navigator) {
            vars.navigator->Render(vars.ctx, vars.slot);
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Navigator");
        IM_CHECK(ctx->GetWindowByRef("") != nullptr);
        
        // Open the Data Categories tree node, then the nested By Side node
        // Use path notation to access nested tree nodes
        ctx->ItemOpen("Data Categories/By Side");
    };
    
    // Test 6: Navigator Shows Statistics
    t = IM_REGISTER_TEST(e, "simple_gui", "navigator_statistics");
    t->SetVarsDataType<NavigatorTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        NavigatorTestVars& vars = ctx->GetVars<NavigatorTestVars>();
        vars.Initialize(20);
        
        // Add test data with specific values
        for (int i = 0; i < 20; i++) {
            vars.ts_ns[i] = 1000000 * (i + 1);
            vars.px_n[i] = 15000;
            vars.qty[i] = 100;
            vars.side[i] = (i < 10) ? 1 : 2;
            vars.ctx.seq[i].store(1, std::memory_order_relaxed);
            vars.ctx.dirty[i] = (i < 5) ? 1 : 0;
        }
        
        if (vars.navigator) {
            vars.navigator->Render(vars.ctx, vars.slot);
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Navigator");
        IM_CHECK(ctx->GetWindowByRef("") != nullptr);
        
        // Try to open the Statistics tree node
        ctx->ItemOpen("Statistics");
        IM_CHECK(ctx->GetWindowByRef("") != nullptr);
    };
    
    // Test 7: Navigator Shows Quick Filters
    t = IM_REGISTER_TEST(e, "simple_gui", "navigator_filters");
    t->SetVarsDataType<NavigatorTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        NavigatorTestVars& vars = ctx->GetVars<NavigatorTestVars>();
        vars.Initialize(10);
        
        if (vars.navigator) {
            vars.navigator->Render(vars.ctx, vars.slot);
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Navigator");
        IM_CHECK(ctx->GetWindowByRef("") != nullptr);
        
        // Try to open the Quick Filters tree node
        ctx->ItemOpen("Quick Filters");
        IM_CHECK(ctx->GetWindowByRef("") != nullptr);
    };
    
    // Test 8: Navigator Handles Empty Data
    t = IM_REGISTER_TEST(e, "simple_gui", "navigator_empty");
    t->SetVarsDataType<NavigatorTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        NavigatorTestVars& vars = ctx->GetVars<NavigatorTestVars>();
        vars.Initialize(0);
        
        if (vars.navigator) {
            vars.navigator->Render(vars.ctx, vars.slot);
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Navigator");
        
        // Navigator should still render even with no data
        IM_CHECK(ctx->GetWindowByRef("") != nullptr);
        
        // Should be able to open tree nodes even with no data
        ctx->ItemOpen("Data Categories");
        ctx->ItemOpen("Statistics");
        
        IM_CHECK(ctx->GetWindowByRef("") != nullptr);
    };
}
