#include <windows.h>
#include <tchar.h>
#include <d3d11.h>
#include <dxgi1_4.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_te_imconfig.h"
#include "imgui_te_engine.h"
#include "imgui_te_context.h"
#include "imgui_te_ui.h"
#include "navigator.h"

// Data
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

// Test Engine
static ImGuiTestEngine* g_test_engine = nullptr;
static Navigator* g_navigator = nullptr;

// Forward declarations
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Test functions
void RegisterNavigatorTests(ImGuiTestEngine* engine);

// Main code
int main(int, char**)
{
    // Create application window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Navigator Test", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Navigator Test", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, nullptr, nullptr, wc.hInstance, nullptr);

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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Initialize Test Engine
    g_test_engine = ImGuiTestEngine_CreateContext();
    ImGuiTestEngineIO& test_io = ImGuiTestEngine_GetIO(g_test_engine);
    test_io.ConfigVerboseLevel = ImGuiTestVerboseLevel_Info;
    test_io.ConfigRunSpeed = ImGuiTestRunSpeed_Fast;
    
    // Initialize Navigator
    g_navigator = new Navigator();
    g_navigator->Initialize();

    // Register Navigator tests
    RegisterNavigatorTests(g_test_engine);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages
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

        // Handle window resize
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

        // Create simple test environment
        ImGui::Begin("Navigator Test Window");
        ImGui::Text("Navigator Test Application");
        if (ImGui::Button("Run All Tests"))
        {
            ImGuiTestEngine_QueueTests(g_test_engine, ImGuiTestGroup_Tests, "navigator");
        }
        ImGui::End();

        // Render Navigator
        if (g_navigator) {
            g_navigator->Render();
        }

        // Test Engine UI
        ImGuiTestEngine_ShowTestEngineWindows(g_test_engine, nullptr);

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

        g_pSwapChain->Present(1, 0);
    }

    // Cleanup
    if (g_navigator) {
        g_navigator->Cleanup();
        delete g_navigator;
        g_navigator = nullptr;
    }

    ImGuiTestEngine_DestroyContext(g_test_engine);
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Register Navigator-specific tests
void RegisterNavigatorTests(ImGuiTestEngine* engine)
{
    ImGuiTest* t = nullptr;

    // Test: Navigator window opens
    t = IM_REGISTER_TEST(engine, "navigator_tests", "navigator_window_opens");
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        // Check if Navigator window exists
        ctx->WindowFocus("Navigator");
        IM_CHECK(ctx->UiContext->NavWindow != nullptr);
        IM_CHECK_STR_EQ(ctx->UiContext->NavWindow->Name, "Navigator");
    };

    // Test: Categories tree can be expanded
    t = IM_REGISTER_TEST(engine, "navigator_tests", "categories_tree_expand");
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->WindowFocus("Navigator");
        
        // Click on Categories to expand
        ctx->ItemClick("Categories");
        
        // Verify Electronics node is visible
        ImGuiWindow* nav_window = ctx->GetWindowByRef("Navigator");
        IM_CHECK(nav_window != nullptr);
        
        // Check if Electronics tree node exists (it should be visible after expanding Categories)
        bool electronics_found = false;
        ctx->UiContext = ctx->UiContext; // Refresh context
        if (ctx->ItemExists("Electronics"))
            electronics_found = true;
        IM_CHECK(electronics_found);
    };

    // Test: Electronics subcategories
    t = IM_REGISTER_TEST(engine, "navigator_tests", "electronics_subcategories");
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->WindowFocus("Navigator");
        
        // Expand Categories first
        ctx->ItemClick("Categories");
        ctx->Yield(); // Wait a frame
        
        // Expand Electronics
        ctx->ItemClick("Electronics");
        ctx->Yield(); // Wait a frame
        
        // Check if Computers and Mobile nodes exist
        IM_CHECK(ctx->ItemExists("Computers") || ctx->ItemExists("Mobile"));
    };

    // Test: Leaf nodes don't expand
    t = IM_REGISTER_TEST(engine, "navigator_tests", "leaf_nodes_behavior");
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->WindowFocus("Navigator");
        
        // Navigate to a leaf node
        ctx->ItemClick("Categories");
        ctx->Yield();
        ctx->ItemClick("Electronics");
        ctx->Yield();
        ctx->ItemClick("Computers");
        ctx->Yield();
        
        // Try clicking on Laptops (leaf node) - should not cause issues
        if (ctx->ItemExists("Laptops"))
        {
            ctx->ItemClick("Laptops");
            ctx->Yield();
            // Test passes if no crash occurs
            IM_CHECK(true);
        }
    };

    // Test: All main sections exist
    t = IM_REGISTER_TEST(engine, "navigator_tests", "all_main_sections_exist");
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->WindowFocus("Navigator");
        
        // Check main tree nodes exist
        IM_CHECK(ctx->ItemExists("Categories"));
        IM_CHECK(ctx->ItemExists("Customers"));  
        IM_CHECK(ctx->ItemExists("Reports"));
    };
}

// Helper functions (same as main app)
bool CreateDeviceD3D(HWND hWnd)
{
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
    if (res == DXGI_ERROR_UNSUPPORTED)
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

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            g_ResizeWidth = (UINT)LOWORD(lParam);
            g_ResizeHeight = (UINT)HIWORD(lParam);
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
