#!/bin/bash
# Test script for clang-format configuration
# This mimics the GitHub Actions formatting check

echo "Testing clang-format configuration..."

# Find files to format (same as GitHub Actions)
FILES=$(find imgui_opengl_glad -name "*.cpp" -o -name "*.h" | grep -E "(core|ui)" 2>/dev/null || true)

if [ -z "$FILES" ]; then
    echo "No core/ui files found to check"
    exit 0
fi

echo "Found files to check:"
echo "$FILES"
echo ""

# Check if clang-format is available
if ! command -v clang-format >/dev/null 2>&1; then
    echo "clang-format not found, skipping local test"
    echo "This test will run in GitHub Actions CI"
    exit 0
fi

# Test configuration parsing
echo "Testing .clang-format configuration..."
echo "int main(){return 0;}" | clang-format --style=file --assume-filename=test.cpp >/dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ .clang-format configuration is valid"
else
    echo "✗ .clang-format configuration has errors"
    exit 1
fi

echo ""
echo "Testing dry-run formatting on actual files..."

# Test dry-run formatting (same as GitHub Actions)
echo "$FILES" | xargs clang-format --dry-run --Werror --style=file

if [ $? -eq 0 ]; then
    echo "✓ All files pass formatting check"
else
    echo "✗ Some files have formatting issues"
    echo ""
    echo "To fix formatting issues, run:"
    echo "  find imgui_opengl_glad -name \"*.cpp\" -o -name \"*.h\" | grep -E \"(core|ui)\" | xargs clang-format -i"
    exit 1
fi
