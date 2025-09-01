#!/usr/bin/env python3
"""
Custom linter for EMSP project design principles.

Design Rules:
1. No raw new/delete without documentation justification
2. Prefer RAII and smart pointers
3. Document performance-critical sections
4. No malloc/free in C++ code
"""

import re
import sys
import os
from pathlib import Path
from typing import List, Tuple, Optional

class ESMPLinter:
    def __init__(self):
        self.violations = []
        
    def check_file(self, filepath: Path) -> List[Tuple[int, str, str]]:
        """Check a single file for violations."""
        violations = []
        
        if filepath.suffix not in ['.cpp', '.h', '.hpp']:
            return violations
            
        try:
            with open(filepath, 'r', encoding='utf-8') as f:
                lines = f.readlines()
        except Exception as e:
            print(f"Error reading {filepath}: {e}")
            return violations
            
        # Check each line
        for i, line in enumerate(lines, 1):
            violations.extend(self._check_line(i, line, lines, filepath))
            
        return violations
    
    def _check_line(self, line_num: int, line: str, all_lines: List[str], filepath: Path) -> List[Tuple[int, str, str]]:
        """Check a single line for violations."""
        violations = []
        stripped = line.strip()
        
        # Skip comments and empty lines
        if not stripped or stripped.startswith('//') or stripped.startswith('/*'):
            return violations
            
        # Rule 1: Check for raw new/delete without documentation
        if self._has_raw_new_delete(stripped):
            if not self._has_justification_nearby(line_num, all_lines):
                violations.append((
                    line_num,
                    "raw-memory-management",
                    f"Raw new/delete found without documentation justification: {stripped[:50]}..."
                ))
        
        # Rule 2: Check for malloc/free usage
        if re.search(r'\b(malloc|calloc|realloc|free)\s*\(', stripped):
            violations.append((
                line_num,
                "c-style-memory",
                f"C-style memory management found, use RAII: {stripped[:50]}..."
            ))
            
        # Rule 3: Check for performance-critical sections
        if self._is_performance_critical(stripped):
            if not self._has_performance_doc_nearby(line_num, all_lines):
                violations.append((
                    line_num,
                    "undocumented-performance",
                    f"Performance-critical code should be documented: {stripped[:50]}..."
                ))
        
        return violations
    
    def _has_raw_new_delete(self, line: str) -> bool:
        """Check if line contains raw new/delete (not make_unique/make_shared)."""
        # Match raw new/delete but not make_unique, make_shared, placement new
        new_pattern = r'\bnew\s+(?!std::nothrow)(?!\()'
        delete_pattern = r'\bdelete\s+(?!\[\])'
        
        if re.search(new_pattern, line) or re.search(delete_pattern, line):
            # Exclude smart pointer usage
            if 'make_unique' in line or 'make_shared' in line:
                return False
            if 'std::unique_ptr' in line or 'std::shared_ptr' in line:
                return False
            return True
        return False
    
    def _has_justification_nearby(self, line_num: int, all_lines: List[str], window: int = 3) -> bool:
        """Check if there's documentation justifying raw memory management nearby."""
        start = max(0, line_num - window - 1)
        end = min(len(all_lines), line_num + window)
        
        context_lines = all_lines[start:end]
        
        justification_keywords = [
            'performance', 'optimization', 'legacy', 'api', 'interface',
            'required', 'necessary', 'critical', 'justif', 'reason',
            'TODO', 'FIXME', 'NOTE', '@brief', '/**', '///'
        ]
        
        for line in context_lines:
            line_lower = line.lower()
            if any(keyword in line_lower for keyword in justification_keywords):
                return True
                
        return False
    
    def _is_performance_critical(self, line: str) -> bool:
        """Check if line contains performance-critical code."""
        critical_patterns = [
            r'\bwhile\s*\(',  # Tight loops
            r'\bfor\s*\([^)]*;\s*[^)]*;\s*[^)]*\)',  # For loops  
            r'\.push\s*\(',  # Container operations
            r'\.emplace',
            r'std::move',
            r'std::forward',
            r'inline\s+',
            r'__forceinline',
        ]
        
        return any(re.search(pattern, line) for pattern in critical_patterns)
    
    def _has_performance_doc_nearby(self, line_num: int, all_lines: List[str], window: int = 2) -> bool:
        """Check if performance-critical code has nearby documentation."""
        start = max(0, line_num - window - 1)
        end = min(len(all_lines), line_num + window)
        
        context_lines = all_lines[start:end]
        
        perf_keywords = [
            'performance', 'optimization', 'fast', 'efficient', 'critical',
            'hot path', 'bottleneck', '@brief', '/**', '///', 'NOTE:'
        ]
        
        for line in context_lines:
            line_lower = line.lower()
            if any(keyword in line_lower for keyword in perf_keywords):
                return True
                
        return False

def main():
    linter = ESMPLinter()
    
    # Get files to check
    if len(sys.argv) > 1:
        files = [Path(f) for f in sys.argv[1:]]
    else:
        # Default: check imgui_opengl_glad/core/ and imgui_opengl_glad/ui/ directories
        base_dir = Path(__file__).parent / "imgui_opengl_glad"
        files = []
        for pattern in ['core/*.cpp', 'core/*.h', 'ui/*.cpp', 'ui/*.h']:
            files.extend(base_dir.glob(pattern))
    
    total_violations = 0
    
    for filepath in files:
        if not filepath.exists():
            continue
            
        violations = linter.check_file(filepath)
        
        if violations:
            print(f"\n📁 {filepath.relative_to(Path.cwd()) if filepath.is_absolute() else filepath}")
            for line_num, rule, message in violations:
                print(f"  ⚠️  Line {line_num:3d}: [{rule}] {message}")
                total_violations += 1
    
    if total_violations == 0:
        print("✅ No design principle violations found!")
        return 0
    else:
        print(f"\n❌ Found {total_violations} design principle violations")
        return 1

if __name__ == "__main__":
    sys.exit(main())
