#include "navigator_test_helper.h"
#include "../imgui_test_engine/imgui_te_context.h"
#include <string>
#include <vector>

bool NavigatorTestHelper::ExpandTreePath(ImGuiTestContext* ctx, const char* path)
{
    ctx->WindowFocus("Navigator");
    
    // Split path by '/'
    std::string path_str(path);
    std::vector<std::string> nodes;
    size_t start = 0;
    size_t end = 0;
    
    while ((end = path_str.find('/', start)) != std::string::npos) {
        nodes.push_back(path_str.substr(start, end - start));
        start = end + 1;
    }
    nodes.push_back(path_str.substr(start));
    
    // Click each node in sequence
    for (const auto& node : nodes) {
        if (!ctx->ItemExists(node.c_str())) {
            return false;
        }
        ctx->ItemClick(node.c_str());
        ctx->Yield(); // Wait a frame for tree to expand
    }
    
    return true;
}

bool NavigatorTestHelper::VerifyTreeStructure(ImGuiTestContext* ctx)
{
    ctx->WindowFocus("Navigator");
    
    // Test main categories exist
    if (!ctx->ItemExists("Categories")) return false;
    if (!ctx->ItemExists("Customers")) return false;
    if (!ctx->ItemExists("Reports")) return false;
    
    // Test Categories subtree
    ctx->ItemClick("Categories");
    ctx->Yield();
    
    if (!ctx->ItemExists("Electronics")) return false;
    if (!ctx->ItemExists("Clothing")) return false;
    if (!ctx->ItemExists("Home & Garden")) return false;
    
    // Test Electronics subtree
    ctx->ItemClick("Electronics");
    ctx->Yield();
    
    if (!ctx->ItemExists("Computers")) return false;
    if (!ctx->ItemExists("Mobile")) return false;
    
    return true;
}

bool NavigatorTestHelper::TestAllLeafNodes(ImGuiTestContext* ctx)
{
    // List of all leaf node paths to test
    const char* leaf_paths[] = {
        "Categories/Electronics/Computers/Laptops",
        "Categories/Electronics/Computers/Desktops",
        "Categories/Electronics/Mobile/Phones",
        "Categories/Electronics/Mobile/Tablets",
        "Categories/Clothing/Men's",
        "Categories/Clothing/Women's",
        "Categories/Clothing/Children's",
        "Categories/Home & Garden/Furniture",
        "Categories/Home & Garden/Tools",
        "Categories/Home & Garden/Decor",
        "Customers/Active",
        "Customers/Inactive",
        "Customers/VIP",
        "Reports/Sales",
        "Reports/Inventory",
        "Reports/Financial"
    };
    
    for (const char* path : leaf_paths) {
        // Reset to collapsed state
        ctx->WindowFocus("Navigator");
        
        // Try to navigate to leaf node
        if (!ExpandTreePath(ctx, path)) {
            return false;
        }
        
        // Leaf nodes should be clickable without causing issues
        std::string leaf_name = std::string(path);
        size_t last_slash = leaf_name.find_last_of('/');
        if (last_slash != std::string::npos) {
            leaf_name = leaf_name.substr(last_slash + 1);
        }
        
        if (ctx->ItemExists(leaf_name.c_str())) {
            ctx->ItemClick(leaf_name.c_str());
            ctx->Yield();
        }
    }
    
    return true;
}

bool NavigatorTestHelper::TestTreeMemoryStability(ImGuiTestContext* ctx)
{
    // Expand and collapse trees multiple times to test memory stability
    for (int i = 0; i < 10; i++) {
        ctx->WindowFocus("Navigator");
        
        // Expand all
        ctx->ItemClick("Categories");
        ctx->Yield();
        ctx->ItemClick("Electronics");
        ctx->Yield();
        ctx->ItemClick("Computers");
        ctx->Yield();
        
        // Collapse all (click again to collapse)
        ctx->ItemClick("Computers");
        ctx->Yield();
        ctx->ItemClick("Electronics");
        ctx->Yield();
        ctx->ItemClick("Categories");
        ctx->Yield();
    }
    
    return true;
}

void RegisterAllNavigatorTests(ImGuiTestEngine* engine)
{
    ImGuiTest* t = nullptr;

    // Basic functionality tests
    t = IM_REGISTER_TEST(engine, "navigator", "window_exists");
    t->TestFunc = [](ImGuiTestContext* ctx) {
        ctx->WindowFocus("Navigator");
        IM_CHECK(ctx->UiContext->NavWindow != nullptr);
    };

    t = IM_REGISTER_TEST(engine, "navigator", "tree_structure");
    t->TestFunc = [](ImGuiTestContext* ctx) {
        IM_CHECK(NavigatorTestHelper::VerifyTreeStructure(ctx));
    };

    t = IM_REGISTER_TEST(engine, "navigator", "all_leaf_nodes");
    t->TestFunc = [](ImGuiTestContext* ctx) {
        IM_CHECK(NavigatorTestHelper::TestAllLeafNodes(ctx));
    };

    t = IM_REGISTER_TEST(engine, "navigator", "memory_stability");
    t->TestFunc = [](ImGuiTestContext* ctx) {
        IM_CHECK(NavigatorTestHelper::TestTreeMemoryStability(ctx));
    };

    // Performance tests
    t = IM_REGISTER_TEST(engine, "navigator", "rapid_clicking");
    t->TestFunc = [](ImGuiTestContext* ctx) {
        ctx->WindowFocus("Navigator");
        
        // Rapidly click on tree nodes
        for (int i = 0; i < 100; i++) {
            ctx->ItemClick("Categories");
            if (i % 10 == 0) ctx->Yield(); // Occasional yield
        }
        
        IM_CHECK(true); // Pass if no crash
    };

    // UI state tests
    t = IM_REGISTER_TEST(engine, "navigator", "tree_state_persistence");
    t->TestFunc = [](ImGuiTestContext* ctx) {
        ctx->WindowFocus("Navigator");
        
        // Expand some nodes
        ctx->ItemClick("Categories");
        ctx->Yield();
        ctx->ItemClick("Electronics");
        ctx->Yield();
        
        // Focus another window and come back
        ctx->WindowFocus("Navigator Test Window");
        ctx->Yield();
        ctx->WindowFocus("Navigator");
        ctx->Yield();
        
        // Tree should still be expanded
        IM_CHECK(ctx->ItemExists("Computers"));
    };
}
