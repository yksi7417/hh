#pragma once
#include "../../imgui_test_engine/imgui_te_engine.h"
#include "../../order_blotter.h"
#include <vector>
#include <string>

// Test helper functions for OrderBlotter testing

class OrderBlotterTestHelper {
public:
    // Test data generation
    static std::vector<Order> generateTestOrders(int count);
    static std::vector<Order> generateSmallTestSet(); // 10 orders
    static std::vector<Order> generateMediumTestSet(); // 100 orders  
    static std::vector<Order> generateLargeTestSet(); // 300 orders
    static std::vector<Order> generateVariableTestSet(int size); // custom size
    
    // Test verification helpers
    static bool verifyRowCount(OrderBlotter& blotter, int expectedCount);
    static bool verifyColumnHeaders(ImGuiTestEngine* engine);
    static bool verifySortOrder(const std::vector<Order>& orders, int column, bool ascending);
    static bool verifySelection(OrderBlotter& blotter, const std::vector<int>& expectedIds);
    static std::vector<Order> getOrdersFromBlotter(OrderBlotter& blotter);
    static bool verifyDataFormatting(ImGuiTestEngine* engine);
    
    // UI interaction helpers
    static void clickColumnHeader(ImGuiTestEngine* engine, int column);
    static void selectRow(ImGuiTestEngine* engine, int row, bool ctrlHeld = false);
    static void selectRange(ImGuiTestEngine* engine, int startRow, int endRow);
    static void clearSelection(ImGuiTestEngine* engine);
    
    // Test setup and teardown
    static void setupTest(OrderBlotter& blotter, const std::vector<Order>& testData);
    static void renderFrame(OrderBlotter& blotter);
    static void cleanupTest();
    
    // Assertion helpers
    static void assertRowCount(OrderBlotter& blotter, int expected, const char* testName);
    static void assertSortOrder(const std::vector<Order>& orders, int column, bool ascending, const char* testName);
    static void assertSelection(OrderBlotter& blotter, const std::vector<int>& expectedIds, const char* testName);
    static void assertColumnVisible(ImGuiTestEngine* engine, const char* columnName, const char* testName);
    
    // Status and progress reporting
    static void logTestStart(const char* testName);
    static void logTestResult(const char* testName, bool passed);
    static void logTestProgress(const char* message);
    
private:
    static int s_testCounter;
    static char s_customerNames[20][32];
    static char s_productNames[15][32];
    static char s_statusOptions[5][16];
    
    // Internal helpers
    static void initializeTestData();
    static Order createRandomOrder(int id);
    static float getRandomPrice();
    static int getRandomQuantity();
};

// Test constants
const int MAX_SMALL_TEST_SIZE = 10;
const int MAX_MEDIUM_TEST_SIZE = 100;
const int MAX_LARGE_TEST_SIZE = 300;
const int MIN_VARIABLE_TEST_SIZE = 5;

// Column indices for sorting tests
enum OrderColumns {
    COL_ID = 0,
    COL_CUSTOMER = 1,
    COL_PRODUCT = 2,
    COL_QUANTITY = 3,
    COL_PRICE = 4,
    COL_STATUS = 5
};

// Test result structure
struct TestResult {
    const char* testName;
    bool passed;
    const char* message;
    double executionTime;
};
