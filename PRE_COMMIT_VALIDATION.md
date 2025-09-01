# EMSP Pre-Commit Validation System

## Overview

This repository implements a comprehensive 3-stage pre-commit validation system that ensures both code quality and functional correctness before allowing any commits.

## Validation Stages

### Stage 1: Code Quality and Design Principles
- **Tool**: `lint.bat` → `lint_design.py`
- **Purpose**: Enforce coding standards and design principles
- **Checks**:
  - Performance-critical code documentation requirements
  - Memory management patterns (raw new/delete, malloc/free detection)
  - Static analysis with clang-tidy (if available)
- **Pass Criteria**: Zero design principle violations

### Stage 2: Test Validation
- **Tool**: `test_pre_commit.bat`
- **Purpose**: Ensure all functionality works correctly
- **Process**:
  1. Configure CMake test build (suppressed output)
  2. Build unit tests (suppressed output)
  3. Run full test suite with CTest
- **Pass Criteria**: All 14 unit tests must pass (100% success rate)

### Stage 3: Final Validation
- **Purpose**: Confirm all checks passed
- **Action**: Approve commit for repository

## Files in the System

### Core Scripts
- **`.git/hooks/pre-commit`**: Main pre-commit hook orchestrating all validation
- **`lint.bat`**: Code quality checker with design principle enforcement
- **`lint_design.py`**: Python-based linter with performance pattern detection
- **`test_pre_commit.bat`**: Streamlined test runner for commit validation
- **`build_tests_only.bat`**: Full test build script (development use)

### Test Infrastructure
- **Unit Tests**: 14 comprehensive tests covering:
  - Basic functionality validation
  - MPSC queue operations
  - Data updater functionality
  - Configuration validation
  - Performance characteristics
  - Concurrent operations

## How It Works

```
git commit -m "message"
         ↓
   Pre-commit Hook
         ↓
┌─────────────────────┐
│ Stage 1: Linting    │
│ - Design principles │
│ - Code patterns     │
│ - Static analysis   │
└─────────────────────┘
         ↓ (pass)
┌─────────────────────┐
│ Stage 2: Testing    │
│ - Build tests       │
│ - Run all 14 tests  │
│ - Validate results  │
└─────────────────────┘
         ↓ (pass)
┌─────────────────────┐
│ Stage 3: Approval   │
│ - Final validation  │
│ - Commit approved   │
└─────────────────────┘
         ↓
    Commit Success
```

## Error Handling

### Linting Failures
```
[ERROR] Code quality checks failed
Please fix linting issues before committing
```
- **Action Required**: Fix code quality issues or add proper documentation
- **Examples**: Add performance documentation, fix memory management patterns

### Test Failures
```
[ERROR] Tests failed
Please fix failing tests before committing
```
- **Action Required**: Fix broken functionality causing test failures
- **Details**: Full test output shown with failure details

### Prerequisites Missing
```
ERROR: PowerShell Core required for tests
ERROR: Python is required to run the design linter
```
- **Action Required**: Install missing dependencies

## Configuration

### Windows Compatibility
- Uses ASCII-safe output characters
- Handles both Git Bash and cmd.exe environments
- Proper PowerShell Core detection and usage

### Performance Optimization
- Suppresses verbose build output during commit validation
- Uses parallel test execution when possible
- Minimal output for fast commit feedback

## Benefits

1. **Quality Assurance**: No broken code enters the repository
2. **Performance Focus**: Enforces documentation of performance-critical sections
3. **Functional Correctness**: All features must work before commit
4. **Developer Productivity**: Catches issues early in development cycle
5. **Maintainability**: Consistent code quality standards

## Usage Examples

### Successful Commit
```bash
$ git commit -m "Add new feature"
Running EMSP code quality and test validation...

[1/3] Checking code quality and design principles...
[OK] Design principles check passed!
[OK] Code quality checks passed

[2/3] Building and running tests...
Running unit tests...
100% tests passed, 0 tests failed out of 1
All tests passed successfully
[OK] All tests passed

[3/3] Final validation...
[OK] All pre-commit checks passed

[SUCCESS] Commit approved - code quality and tests verified
```

### Rejected Commit (Test Failure)
```bash
$ git commit -m "Broken feature"
Running EMSP code quality and test validation...

[1/3] Checking code quality and design principles...
[OK] Code quality checks passed

[2/3] Building and running tests...
Running unit tests...
[FAILED] SimpleTest.BasicSetup
0% tests passed, 1 tests failed out of 1
ERROR: Some tests failed

[ERROR] Tests failed
Please fix failing tests before committing
```

### Rejected Commit (Linting Failure)
```bash
$ git commit -m "Undocumented performance code"
Running EMSP code quality and test validation...

[1/3] Checking code quality and design principles...
[WARN] Line 45: [undocumented-performance] Performance-critical code should be documented
[ERROR] Found 1 design principle violations

[ERROR] Code quality checks failed
Please fix linting issues before committing
```

## Maintenance

- **Adding New Tests**: Tests are automatically discovered and run
- **Updating Linting Rules**: Modify `lint_design.py` patterns
- **Platform Support**: Currently optimized for Windows with Git Bash support
- **Performance Tuning**: Adjust parallel test execution and output verbosity in scripts

This system ensures that every commit maintains both high code quality and functional correctness, preventing issues from entering the main repository.
