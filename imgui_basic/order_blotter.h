#pragma once

#include "imgui.h"
#include <vector>
#include <string>
#include <functional>

// Filter types for different column types
enum FilterType {
    FILTER_NONE = 0,
    FILTER_TEXT_CONTAINS,
    FILTER_TEXT_EQUALS,
    FILTER_TEXT_STARTS_WITH,
    FILTER_TEXT_ENDS_WITH,
    FILTER_NUMERIC_EQUALS,
    FILTER_NUMERIC_GREATER,
    FILTER_NUMERIC_LESS,
    FILTER_NUMERIC_GREATER_EQUAL,
    FILTER_NUMERIC_LESS_EQUAL,
    FILTER_NUMERIC_RANGE
};

// Column filter structure
struct ColumnFilter {
    FilterType type = FILTER_NONE;
    bool enabled = false;
    
    // Text filters
    char text_value[256] = "";
    
    // Numeric filters
    float numeric_value = 0.0f;
    float range_min = 0.0f;
    float range_max = 0.0f;
    int int_value = 0;
    int int_range_min = 0;
    int int_range_max = 0;
};

// Order structure
struct Order {
    int id;
    char customer[64];    // Fixed-size buffers - more cache-friendly
    char product[32];     // Avoids heap allocations
    int quantity;
    float price;
    char status[16];      // Most status strings are short
};

// OrderBlotter class to manage the orders table UI
class OrderBlotter {
public:
    OrderBlotter();
    ~OrderBlotter();

    // Initialize the blotter (generate sample data)
    void Initialize(int order_count = 100000);
    
    // Render the Orders window
    void Render();
    
    // Get selected order IDs
    const std::vector<int>& GetSelectedOrderIds() const { return selected_order_ids_; }
    
    // Clear selection
    void ClearSelection();
    
    // Filter management
    void ClearAllFilters();
    void SetColumnFilter(int column, const ColumnFilter& filter);
    bool HasActiveFilters() const;

private:
    // Data
    std::vector<Order> orders_;
    std::vector<Order> filtered_orders_;  // Filtered view of orders
    bool orders_generated_;
    
    // Filtering state
    ColumnFilter column_filters_[6];  // One for each column
    bool filters_dirty_ = true;       // Flag to rebuild filtered view
    
    // Selection state
    std::vector<int> selected_order_ids_;  // Store Order IDs, not row indices
    int last_selected_row_;                // For shift-click selection
    
    // Helper functions
    void GenerateOrders(int count);
    bool IsOrderSelected(int order_id);
    void ToggleOrderSelection(int order_id);
    void SelectOrderRange(int start_row, int end_row);
    void RenderTable();
    void RenderSelectionInfo();
    
    // Filtering functions
    void ApplyFilters();
    bool PassesFilter(const Order& order) const;
    bool PassesColumnFilter(const Order& order, int column, const ColumnFilter& filter) const;
};
