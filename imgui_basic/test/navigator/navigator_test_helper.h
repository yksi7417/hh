#pragma once
#include "../imgui_test_engine/imgui_te_engine.h"
#include "../imgui_test_engine/imgui_te_context.h"

// Navigator-specific test utilities
class NavigatorTestHelper {
public:
    // Helper to expand tree path (e.g., "Categories/Electronics/Computers")
    static bool ExpandTreePath(ImGuiTestContext* ctx, const char* path);
    
    // Helper to verify tree structure exists
    static bool VerifyTreeStructure(ImGuiTestContext* ctx);
    
    // Helper to test all leaf nodes are accessible
    static bool TestAllLeafNodes(ImGuiTestContext* ctx);
    
    // Helper to verify no memory leaks in tree operations
    static bool TestTreeMemoryStability(ImGuiTestContext* ctx);
};

// Register all Navigator tests
void RegisterAllNavigatorTests(ImGuiTestEngine* engine);
