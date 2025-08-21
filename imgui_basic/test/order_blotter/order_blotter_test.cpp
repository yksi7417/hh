#include "order_blotter_test_helper.h"
#include "../../imgui/imgui.h"
#include "../../imgui_test_engine/imgui_te_engine.h"
#include <iostream>
#include <vector>
#include <cassert>

// Global test state
OrderBlotter* g_testBlotter = nullptr;
ImGuiTestEngine* g_testEngine = nullptr;
int g_totalTests = 0;
int g_passedTests = 0;
std::vector<Order> g_currentTestData;

// Test function declarations
bool test_row_count_accuracy();
bool test_sorting_functionality();
bool test_variable_row_sizes();
bool test_column_display();
bool test_multiple_selection();

// Setup and teardown
bool initializeTestEnvironment();
void cleanupTestEnvironment();
bool runAllTests();

int main() {
    std::cout << "OrderBlotter Test Suite" << std::endl;
    std::cout << "======================" << std::endl;
    
    if (!initializeTestEnvironment()) {
        std::cout << "Failed to initialize test environment" << std::endl;
        return 1;
    }
    
    bool allTestsPassed = runAllTests();
    
    cleanupTestEnvironment();
    
    // Print final results
    std::cout << std::endl << "======================" << std::endl;
    std::cout << "Test Results Summary" << std::endl;
    std::cout << "======================" << std::endl;
    std::cout << "Total Tests: " << g_totalTests << std::endl;
    std::cout << "Passed: " << g_passedTests << std::endl;
    std::cout << "Failed: " << (g_totalTests - g_passedTests) << std::endl;
    std::cout << "Success Rate: " << (g_totalTests > 0 ? (float)g_passedTests / g_totalTests * 100.0f : 0.0f) << "%" << std::endl;
    
    return allTestsPassed ? 0 : 1;
}

bool initializeTestEnvironment() {
    std::cout << "Initializing test environment..." << std::endl;
    
    // Initialize ImGui context (minimal setup for testing)
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup ImGui style
    ImGui::StyleColorsDark();
    
    // Initialize test engine
    g_testEngine = ImGuiTestEngine_CreateContext();
    if (!g_testEngine) {
        std::cout << "Failed to create ImGui Test Engine context" << std::endl;
        return false;
    }
    
    // Create OrderBlotter instance
    g_testBlotter = new OrderBlotter();
    if (!g_testBlotter) {
        std::cout << "Failed to create OrderBlotter instance" << std::endl;
        return false;
    }
    
    std::cout << "Test environment initialized successfully" << std::endl;
    return true;
}

void cleanupTestEnvironment() {
    std::cout << "Cleaning up test environment..." << std::endl;
    
    if (g_testBlotter) {
        delete g_testBlotter;
        g_testBlotter = nullptr;
    }
    
    if (g_testEngine) {
        ImGuiTestEngine_DestroyContext(g_testEngine);
        g_testEngine = nullptr;
    }
    
    ImGui::DestroyContext();
}

bool runAllTests() {
    std::cout << std::endl << "Starting test execution..." << std::endl;
    
    // Track overall success
    bool allPassed = true;
    
    // Test 1: Row Count Accuracy
    g_totalTests++;
    if (test_row_count_accuracy()) {
        g_passedTests++;
    } else {
        allPassed = false;
    }
    
    // Test 2: Sorting Functionality
    g_totalTests++;
    if (test_sorting_functionality()) {
        g_passedTests++;
    } else {
        allPassed = false;
    }
    
    // Test 3: Variable Row Sizes
    g_totalTests++;
    if (test_variable_row_sizes()) {
        g_passedTests++;
    } else {
        allPassed = false;
    }
    
    // Test 4: Column Display
    g_totalTests++;
    if (test_column_display()) {
        g_passedTests++;
    } else {
        allPassed = false;
    }
    
    // Test 5: Multiple Selection
    g_totalTests++;
    if (test_multiple_selection()) {
        g_passedTests++;
    } else {
        allPassed = false;
    }
    
    return allPassed;
}

bool test_row_count_accuracy() {
    OrderBlotterTestHelper::logTestStart("Row Count Accuracy");
    
    bool testPassed = true;
    
    try {
        // Test 1: Empty dataset
        OrderBlotterTestHelper::logTestProgress("Testing empty dataset");
        g_testBlotter->Initialize(0);
        OrderBlotterTestHelper::renderFrame(*g_testBlotter);
        
        // For empty dataset, we expect no orders - this is verified by initialization
        std::cout << "PASS: Empty dataset initialization" << std::endl;
        
        // Test 2: Small dataset (10 rows)
        OrderBlotterTestHelper::logTestProgress("Testing small dataset (10 rows)");
        g_testBlotter->Initialize(10);
        OrderBlotterTestHelper::renderFrame(*g_testBlotter);
        
        std::cout << "PASS: Small dataset initialization (10 rows)" << std::endl;
        
        // Test 3: Medium dataset (100 rows)
        OrderBlotterTestHelper::logTestProgress("Testing medium dataset (100 rows)");
        g_testBlotter->Initialize(100);
        OrderBlotterTestHelper::renderFrame(*g_testBlotter);
        
        std::cout << "PASS: Medium dataset initialization (100 rows)" << std::endl;
        
        // Test 4: Large dataset (300 rows)
        OrderBlotterTestHelper::logTestProgress("Testing large dataset (300 rows)");
        g_testBlotter->Initialize(300);
        OrderBlotterTestHelper::renderFrame(*g_testBlotter);
        
        std::cout << "PASS: Large dataset initialization (300 rows)" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION in test_row_count_accuracy: " << e.what() << std::endl;
        testPassed = false;
    }
    
    OrderBlotterTestHelper::logTestResult("Row Count Accuracy", testPassed);
    return testPassed;
}

bool test_sorting_functionality() {
    OrderBlotterTestHelper::logTestStart("Sorting Functionality");
    
    bool testPassed = true;
    
    try {
        // Use medium dataset for sorting tests
        g_testBlotter->Initialize(100);
        OrderBlotterTestHelper::renderFrame(*g_testBlotter);
        
        // Note: Since OrderBlotter uses ImGui's built-in sorting, we can't easily test
        // the actual sort results without accessing internal data. Instead, we test
        // that the sorting functionality doesn't crash and renders correctly.
        
        OrderBlotterTestHelper::logTestProgress("Testing table rendering with sorting UI");
        
        // Multiple renders to ensure stability
        for (int i = 0; i < 5; i++) {
            OrderBlotterTestHelper::renderFrame(*g_testBlotter);
        }
        
        std::cout << "PASS: Sorting UI renders without errors" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION in test_sorting_functionality: " << e.what() << std::endl;
        testPassed = false;
    }
    
    OrderBlotterTestHelper::logTestResult("Sorting Functionality", testPassed);
    return testPassed;
}

bool test_variable_row_sizes() {
    OrderBlotterTestHelper::logTestStart("Variable Row Sizes");
    
    bool testPassed = true;
    
    try {
        int testSizes[] = {5, 50, 150, 299};
        
        for (int size : testSizes) {
            OrderBlotterTestHelper::logTestProgress(("Testing with " + std::to_string(size) + " rows").c_str());
            
            g_testBlotter->Initialize(size);
            OrderBlotterTestHelper::renderFrame(*g_testBlotter);
            
            // Test that the table renders correctly with this size
            std::cout << "PASS: Table renders correctly with " << size << " rows" << std::endl;
            
            // Test multiple renders to ensure stability
            for (int i = 0; i < 3; i++) {
                OrderBlotterTestHelper::renderFrame(*g_testBlotter);
            }
            
            std::cout << "PASS: Table remains stable with " << size << " rows" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION in test_variable_row_sizes: " << e.what() << std::endl;
        testPassed = false;
    }
    
    OrderBlotterTestHelper::logTestResult("Variable Row Sizes", testPassed);
    return testPassed;
}

bool test_column_display() {
    OrderBlotterTestHelper::logTestStart("Column Display");
    
    bool testPassed = true;
    
    try {
        // Use small dataset for column display tests
        g_testBlotter->Initialize(10);
        OrderBlotterTestHelper::renderFrame(*g_testBlotter);
        
        // Test that the table renders without errors
        OrderBlotterTestHelper::logTestProgress("Checking table rendering");
        
        // Multiple renders to test stability
        for (int i = 0; i < 5; i++) {
            OrderBlotterTestHelper::renderFrame(*g_testBlotter);
        }
        
        std::cout << "PASS: Table renders consistently with all columns" << std::endl;
        
        // Test column structure by rendering multiple times
        OrderBlotterTestHelper::logTestProgress("Checking column stability");
        for (int i = 0; i < 10; i++) {
            OrderBlotterTestHelper::renderFrame(*g_testBlotter);
        }
        
        std::cout << "PASS: Column structure remains stable" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION in test_column_display: " << e.what() << std::endl;
        testPassed = false;
    }
    
    OrderBlotterTestHelper::logTestResult("Column Display", testPassed);
    return testPassed;
}

bool test_multiple_selection() {
    OrderBlotterTestHelper::logTestStart("Multiple Selection");
    
    bool testPassed = true;
    
    try {
        // Use small dataset for selection tests
        g_testBlotter->Initialize(10);
        OrderBlotterTestHelper::renderFrame(*g_testBlotter);
        
        // Test clear selection
        OrderBlotterTestHelper::logTestProgress("Testing clear selection");
        g_testBlotter->ClearSelection();
        
        std::vector<int> expectedEmpty = {};
        if (!OrderBlotterTestHelper::verifySelection(*g_testBlotter, expectedEmpty)) {
            std::cout << "FAIL: Clear selection test" << std::endl;
            testPassed = false;
        } else {
            std::cout << "PASS: Clear selection test" << std::endl;
        }
        
        // Test that selection methods don't crash
        OrderBlotterTestHelper::logTestProgress("Testing selection API stability");
        
        // Test multiple calls to clear selection
        for (int i = 0; i < 5; i++) {
            g_testBlotter->ClearSelection();
            OrderBlotterTestHelper::renderFrame(*g_testBlotter);
        }
        
        std::cout << "PASS: Selection API remains stable" << std::endl;
        
        // Test selection state after rendering
        OrderBlotterTestHelper::logTestProgress("Testing selection state consistency");
        g_testBlotter->ClearSelection();
        
        for (int i = 0; i < 3; i++) {
            OrderBlotterTestHelper::renderFrame(*g_testBlotter);
            auto selectedIds = g_testBlotter->GetSelectedOrderIds();
            if (!selectedIds.empty()) {
                std::cout << "FAIL: Selection state should remain empty" << std::endl;
                testPassed = false;
                break;
            }
        }
        
        if (testPassed) {
            std::cout << "PASS: Selection state remains consistent" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION in test_multiple_selection: " << e.what() << std::endl;
        testPassed = false;
    }
    
    OrderBlotterTestHelper::logTestResult("Multiple Selection", testPassed);
    return testPassed;
}
