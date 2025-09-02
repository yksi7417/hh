#!/usr/bin/env pwsh

# Script to format code using clang-format
# This will fix formatting issues automatically

Write-Host "Auto-formatting code with clang-format..."

# Try to find clang-format in common locations
$clangFormatPaths = @(
    "clang-format",  # If it's in PATH
    "C:\Program Files\LLVM\bin\clang-format.exe",
    "C:\vcpkg\installed\x64-windows\tools\llvm\clang-format.exe",
    "$env:VCPKG_ROOT\installed\x64-windows\tools\llvm\clang-format.exe",
    "$env:ProgramFiles\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\bin\clang-format.exe",
    "$env:ProgramFiles\Microsoft Visual Studio\2022\Professional\VC\Tools\Llvm\bin\clang-format.exe",
    "$env:ProgramFiles\Microsoft Visual Studio\2022\Enterprise\VC\Tools\Llvm\bin\clang-format.exe"
)

$clangFormat = $null
foreach ($path in $clangFormatPaths) {
    try {
        if ($path -eq "clang-format") {
            # Test if it's in PATH
            $result = & $path --version 2>$null
            if ($LASTEXITCODE -eq 0) {
                $clangFormat = $path
                break
            }
        } elseif (Test-Path $path) {
            $clangFormat = $path
            break
        }
    } catch {
        continue
    }
}

if (-not $clangFormat) {
    Write-Host "clang-format not found in common locations." -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install clang-format using one of these methods:" -ForegroundColor Yellow
    Write-Host "1. Global vcpkg (recommended):" -ForegroundColor Cyan
    Write-Host "   vcpkg install --triplet x64-windows llvm[clang-tools-extra]" -ForegroundColor White
    Write-Host ""
    Write-Host "2. Chocolatey:" -ForegroundColor Cyan
    Write-Host "   choco install llvm" -ForegroundColor White
    Write-Host ""
    Write-Host "3. Visual Studio Installer (C++ Clang tools component)" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "4. LLVM official installer from https://releases.llvm.org/" -ForegroundColor Cyan
    exit 1
}

try {
    $version = & $clangFormat --version
    Write-Host "Using: $version" -ForegroundColor Green
    Write-Host "Path: $clangFormat" -ForegroundColor Gray
} catch {
    Write-Error "Failed to execute clang-format at: $clangFormat"
    exit 1
}

# Find and format our source files (exclude third-party libraries)
$coreFiles = Get-ChildItem -Path "imgui_opengl_glad\core" -Include "*.cpp","*.h" -Recurse
$uiFiles = Get-ChildItem -Path "imgui_opengl_glad\ui" -Include "*.cpp","*.h" -Recurse
$testFiles = Get-ChildItem -Path "imgui_opengl_glad\tests" -Include "*.cpp","*.h" -Recurse
$mainFile = Get-Item -Path "imgui_opengl_glad\main.cpp" -ErrorAction SilentlyContinue

$files = @()
$files += $coreFiles
$files += $uiFiles
$files += $testFiles
if ($mainFile) { $files += $mainFile }

if ($files.Count -eq 0) {
    Write-Host "No source files found to format"
    exit 0
}

Write-Host "Formatting $($files.Count) files:"
$files | ForEach-Object { Write-Host "  $_" }

$formattedCount = 0
$files | ForEach-Object { 
    Write-Host "Formatting: $($_.FullName)"
    & $clangFormat -i $_.FullName
    if ($LASTEXITCODE -eq 0) {
        $formattedCount++
    } else {
        Write-Warning "Failed to format: $($_.FullName)"
    }
}

Write-Host "Successfully formatted $formattedCount out of $($files.Count) files" -ForegroundColor Green

# Run a dry-run check to verify formatting
Write-Host "`nVerifying formatting..."
$formatErrors = 0
$files | ForEach-Object { 
    $result = & $clangFormat --dry-run --Werror $_.FullName 2>&1
    if ($LASTEXITCODE -ne 0) { 
        Write-Host "Still has formatting issues: $($_.FullName)" -ForegroundColor Yellow
        $formatErrors++
    }
}

if ($formatErrors -eq 0) {
    Write-Host "All files are properly formatted!" -ForegroundColor Green
} else {
    Write-Warning "$formatErrors files still have formatting issues"
}
