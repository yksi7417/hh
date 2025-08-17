#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <dxgi1_4.h>
#include <tchar.h>
#include <windows.h>
#include <vector>
#include <string>
#include <random>
#include <sstream>

// Data
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

// Forward declarations
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Sample data for the Orders table
struct Order {
    int id;
    std::string customer;
    std::string product;
    int quantity;
    float price;
    std::string status;
};

// Sample data arrays for random generation
static const char* first_names[] = {
    "John", "Jane", "Mike", "Sarah", "Tom", "Lisa", "Bob", "Amy", "David", "Emma",
    "Chris", "Anna", "Mark", "Kate", "Paul", "Laura", "Steve", "Maria", "Dan", "Sophie",
    "Alex", "Grace", "Matt", "Eva", "Nick", "Lily", "Sam", "Rose", "Ben", "Zoe",
    "Jake", "Mia", "Luke", "Ella", "Ryan", "Chloe", "Adam", "Olivia", "Josh", "Maya"
};

static const char* last_names[] = {
    "Smith", "Johnson", "Brown", "Davis", "Wilson", "Chen", "Martinez", "Anderson",
    "Garcia", "Miller", "Taylor", "Moore", "Jackson", "Martin", "Lee", "Thompson",
    "White", "Harris", "Clark", "Lewis", "Walker", "Hall", "Allen", "Young",
    "King", "Wright", "Lopez", "Hill", "Scott", "Green", "Adams", "Baker",
    "Gonzalez", "Nelson", "Carter", "Mitchell", "Perez", "Roberts", "Turner", "Phillips"
};

static const char* products[] = {
    "Widget A", "Widget B", "Widget C", "Widget D", "Widget E",
    "Gadget Pro", "Gadget Lite", "Gadget Max", "Device X1", "Device X2",
    "Tool Basic", "Tool Premium", "Kit Standard", "Kit Deluxe", "Module Alpha",
    "Module Beta", "Component Z", "Assembly Unit", "Part 2000", "Part 3000"
};

static const char* statuses[] = {
    "Pending", "Processing", "Shipped", "Delivered", "Cancelled"
};

static std::vector<Order> orders;
static bool orders_generated = false;

// Function to generate random orders
void GenerateOrders(int count) {
    if (orders_generated) return;
    
    orders.reserve(count);
    std::random_device rd;
    std::mt19937 gen(rd());
    
    std::uniform_int_distribution<> first_name_dist(0, sizeof(first_names)/sizeof(first_names[0]) - 1);
    std::uniform_int_distribution<> last_name_dist(0, sizeof(last_names)/sizeof(last_names[0]) - 1);
    std::uniform_int_distribution<> product_dist(0, sizeof(products)/sizeof(products[0]) - 1);
    std::uniform_int_distribution<> status_dist(0, sizeof(statuses)/sizeof(statuses[0]) - 1);
    std::uniform_int_distribution<> quantity_dist(1, 20);
    std::uniform_real_distribution<float> price_dist(5.99f, 299.99f);
    
    for (int i = 0; i < count; ++i) {
        Order order;
        order.id = 1000 + i;
        order.customer = std::string(first_names[first_name_dist(gen)]) + " " + std::string(last_names[last_name_dist(gen)]);
        order.product = products[product_dist(gen)];
        order.quantity = quantity_dist(gen);
        order.price = price_dist(gen);
        order.status = statuses[status_dist(gen)];
        
        orders.push_back(order);
    }
    
    orders_generated = true;
}

// Main code
int main(int, char**)
{
    // Create application window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Navigator & Orders", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Generate 100k sample orders
    GenerateOrders(100000);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Create main window with two sections
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::Begin("MainWindow", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        // Create dockspace
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        // Navigator Window (dockable)
        ImGui::Begin("Navigator");
        
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
        
        if (ImGui::TreeNode("Customers"))
        {
            ImGui::TreeNodeEx("Active", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Inactive", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("VIP", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreePop();
        }
        
        if (ImGui::TreeNode("Reports"))
        {
            ImGui::TreeNodeEx("Sales", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Inventory", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Financial", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreePop();
        }
        
        ImGui::End(); // Navigator

        // Orders Window (dockable)
        ImGui::Begin("Orders");
        
        ImGui::Text("Total Orders: %d", (int)orders.size());
        ImGui::Separator();
        
        // Orders table with virtualization for large datasets
        if (ImGui::BeginTable("OrdersTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupColumn("Order ID", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 80.0f, 0);
            ImGui::TableSetupColumn("Customer", ImGuiTableColumnFlags_WidthFixed, 120.0f, 1);
            ImGui::TableSetupColumn("Product", ImGuiTableColumnFlags_WidthFixed, 100.0f, 2);
            ImGui::TableSetupColumn("Quantity", ImGuiTableColumnFlags_WidthFixed, 80.0f, 3);
            ImGui::TableSetupColumn("Price", ImGuiTableColumnFlags_WidthFixed, 80.0f, 4);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthStretch, 0.0f, 5);
            ImGui::TableHeadersRow();
            
            // Use clipper for efficient rendering of large lists
            ImGuiListClipper clipper;
            clipper.Begin((int)orders.size());
            
            while (clipper.Step())
            {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
                {
                    ImGui::TableNextRow();
                    
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%d", orders[row].id);
                    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", orders[row].customer.c_str());
                    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%s", orders[row].product.c_str());
                    
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%d", orders[row].quantity);
                    
                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("$%.2f", orders[row].price);
                    
                    ImGui::TableSetColumnIndex(5);
                    // Color code status
                    if (orders[row].status == "Delivered")
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", orders[row].status.c_str());
                    else if (orders[row].status == "Shipped")
                        ImGui::TextColored(ImVec4(0.0f, 0.7f, 1.0f, 1.0f), "%s", orders[row].status.c_str());
                    else if (orders[row].status == "Processing")
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", orders[row].status.c_str());
                    else if (orders[row].status == "Cancelled")
                        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", orders[row].status.c_str());
                    else
                        ImGui::Text("%s", orders[row].status.c_str());
                }
            }
            
            ImGui::EndTable();
        }
        
        ImGui::End(); // Orders

        ImGui::End();

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        g_pSwapChain->Present(1, 0); // Present with vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions to setup D3D11
bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
            g_ResizeHeight = (UINT)HIWORD(lParam);
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
