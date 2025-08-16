#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK
MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) 
{
    switch (Message) {
    case WM_CLOSE:  PostQuitMessage(0); return 0;
    case WM_DESTROY: return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(Window, &ps);
        TextOutA(dc, 10, 10, "Hello, Handmade!", 17);
        EndPaint(Window, &ps);
    } return 0;
    }
    return DefWindowProcA(Window, Message, WParam, LParam);
}

int WINAPI WinMain(
    HINSTANCE Instance,     // Reference of this instance, 
    HINSTANCE PrevInstance, // Legacy, never used 
    LPSTR CmdLine, 
    int ShowCmd)            // normal windows / etc - windows setting
{
    WNDCLASSA wc = {0};
    wc.style         = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    wc.lpfnWndProc   = MainWindowCallback;
    wc.hInstance     = Instance;
    wc.lpszClassName = "HHWindowClass";
    RegisterClassA(&wc);

    HWND window = CreateWindowExA(
        0, wc.lpszClassName, "HH Day 1",
        WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 960, 540,
        0, 0, Instance, 0);

    // Use ShowCmd parameter to control how window is displayed
    ShowWindow(window, ShowCmd);
    UpdateWindow(window);

    // Debug print like Casey discusses (note the trailing \n is not needed)
    char* Foo = "WinMain started\n";
    OutputDebugStringA(Foo);  // Shows in DebugView or VS Output

    // Use CmdLine parameter to display command-line arguments if any
    if (CmdLine && strlen(CmdLine) > 0) {
        char debugMsg[256];
        sprintf_s(debugMsg, sizeof(debugMsg), "Command line: %s\n", CmdLine);
        OutputDebugStringA(debugMsg);
    }

    // PrevInstance is always NULL in Win32 (legacy from 16-bit Windows)
    // We can acknowledge it exists to suppress the warning
    (void)PrevInstance;  // Explicitly mark as unused

    MSG msg;
    for (;;) {
        BOOL got = GetMessageA(&msg, 0, 0, 0);
        if (got <= 0) break;
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return 0;
}
