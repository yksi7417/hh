#include "navigator.h"
#include "imgui.h"

Navigator::Navigator() : m_initialized(false) {
}

Navigator::~Navigator() {
    if (m_initialized) {
        Cleanup();
    }
}

void Navigator::Initialize() {
    m_initialized = true;
}

void Navigator::Render() {
    if (!m_initialized) return;
    
    // Navigator Window (dockable)
    ImGui::Begin("Navigator");
    
    RenderCategoriesTree();
    RenderCustomersTree();
    RenderReportsTree();
    
    ImGui::End(); // Navigator
}

void Navigator::RenderCategoriesTree() {
    // Tree structure
    if (ImGui::TreeNode("Categories"))
    {
        if (ImGui::TreeNode("Electronics"))
        {
            if (ImGui::TreeNode("Computers"))
            {
                ImGui::TreeNodeEx("Laptops", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
                ImGui::TreeNodeEx("Desktops", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Mobile"))
            {
                ImGui::TreeNodeEx("Phones", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
                ImGui::TreeNodeEx("Tablets", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Clothing"))
        {
            ImGui::TreeNodeEx("Men's", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Women's", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Children's", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Home & Garden"))
        {
            ImGui::TreeNodeEx("Furniture", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Tools", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Decor", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}

void Navigator::RenderCustomersTree() {
    if (ImGui::TreeNode("Customers"))
    {
        ImGui::TreeNodeEx("Active", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("Inactive", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("VIP", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreePop();
    }
}

void Navigator::RenderReportsTree() {
    if (ImGui::TreeNode("Reports"))
    {
        ImGui::TreeNodeEx("Sales", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("Inventory", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("Financial", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreePop();
    }
}

void Navigator::Cleanup() {
    m_initialized = false;
}
