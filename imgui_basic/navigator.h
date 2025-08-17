#pragma once

// Forward declaration for ImGui
struct ImGuiContext;

class Navigator {
public:
    Navigator();
    ~Navigator();
    
    // Initialize the navigator (call once)
    void Initialize();
    
    // Render the navigator window (call each frame)
    void Render();
    
    // Cleanup (call before destruction)
    void Cleanup();

private:
    void RenderCategoriesTree();
    void RenderCustomersTree();
    void RenderReportsTree();
    
    bool m_initialized;
};
