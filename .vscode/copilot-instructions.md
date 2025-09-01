# GitHub Copilot Project Context

## Environment Information
- **Operating System**: Windows
- **Shell**: PowerShell (Windows PowerShell v5.1)
- **Build System**: CMake with Visual Studio toolchain
- **Architecture**: x64-windows

## Shell Commands Guidelines
- Use PowerShell syntax, not Bash
- Use semicolon (`;`) to join commands on a single line, NOT `&&`
- Use PowerShell-style path separators (`\` or PowerShell's path handling)
- Use PowerShell cmdlets when appropriate (e.g., `Get-ChildItem` instead of `ls`)

### Examples:
```powershell
# Correct PowerShell syntax
cd build; cmake --build . --config Debug

# NOT this (Bash syntax)
cd build && cmake --build . --config Debug
```

### Examples:
```powershell
# Correct PowerShell syntax
dir c:\dvlp\vcpkg_installed\x64-windows\share\gtest
Remove-Item -Recurse -Force build

# NOT this (Bash syntax)
ls c:\dvlp\vcpkg_installed\x64-windows\share\gtest
rm -rf build
```


## Build System
- Using CMake with Visual Studio 2022 toolchain
- Target platform: x64-windows
- Package manager: vcpkg (when available)
- Primary build configuration: Debug

## Project Structure
- Main application in `imgui_opengl_glad/` directory
- Uses Dear ImGui for GUI with OpenGL3 + GLFW
- Market data processing with lock-free data structures
- Plugin-based architecture for market data sources

## Coding Standards
- C++17 standard
- Prefer stack allocation over dynamic allocation where possible
- Lock-free programming patterns for high-performance data processing
- Clear separation of concerns with modular design

## Dependencies
- Dear ImGui (included as submodule/source)
- GLFW3 (via vcpkg when available)
- GLAD (via vcpkg when available)
- Custom market data API

## Common Build Commands (PowerShell)
```powershell
# Configure project
cmake -B build -S .

# Build project
cmake --build build --config Debug

# Change directory and build
cd imgui_opengl_glad; cmake -B build -S .

# Run executable
.\build\Debug\emsp.exe

# Run tests
.\build\Debug\test_data_updater.exe
```

## Notes for Code Generation
- Generate Windows-compatible file paths
- Use PowerShell-compatible command syntax
- Consider Visual Studio project structure
- Account for Windows-specific APIs when relevant
- Use appropriate Windows include paths and library linking
