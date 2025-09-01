#pragma once

#include "imgui.h"
#include "main_context.h"
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

// Grouping support
struct GroupInfo {
    std::string group_key;          // The value that defines this group
    std::vector<uint32_t> row_indices; // Indices into the raw context data (NOT copied data)
    bool is_collapsed = false;      // Whether this group is collapsed
    
    // Aggregate data for the group
    int64_t total_qty = 0;
    int64_t avg_price = 0;
    int64_t avg_timestamp = 0;
    int row_count = 0;
};

// Column filter structure
struct ColumnFilter {
    FilterType type = FILTER_NONE;
    bool enabled = false;
    
    // Text filters
    char text_value[256] = "";
    
    // Numeric filters
    int64_t numeric_value = 0;
    int64_t range_min = 0;
    int64_t range_max = 0;
};

// Instead of copying data, we work with indices into the raw data
// No separate data structures - just views into the original memory

// Enhanced MarketDataTable class with sorting, filtering, and grouping
class MarketDataTable {
public:
    MarketDataTable();
    ~MarketDataTable();

    // Initialize the table
    void Initialize(uint32_t max_rows);
    
    // Update data from HostContext - NO COPYING, just refresh views
    void UpdateFromContext(HostContext& ctx, const HostMDSlot& slot, bool should_refresh);
    
    // Render the table window
    void Render(HostContext& ctx, const HostMDSlot& slot);
    
    // Get selected row IDs
    const std::vector<uint32_t>& GetSelectedRowIds() const { return selected_row_ids_; }
    
    // Clear selection
    void ClearSelection();
    
    // Filter management
    void ClearAllFilters();
    void SetColumnFilter(int column, const ColumnFilter& filter);
    bool HasActiveFilters() const;
    
    // Grouping management
    void SetGroupByColumn(int column);
    void ClearGrouping();
    bool HasActiveGrouping() const { return group_by_column_ >= 0; }
    int GetGroupByColumn() const { return group_by_column_; }

private:
    // NO DATA STORAGE - we work directly with raw memory from HostContext
    // Only store indices and views, never copy the actual market data
    std::vector<uint32_t> all_row_indices_;     // All valid row indices (0 to num_rows-1)
    std::vector<uint32_t> filtered_indices_;    // Indices that pass filters
    uint32_t num_rows_;                         // Total number of rows
    
    // Filtering state
    ColumnFilter column_filters_[5];  // One for each column (ID, TS, PX, QTY, SIDE)
    bool filters_dirty_ = true;       // Flag to rebuild filtered view
    
    // Grouping state
    int group_by_column_ = -1;        // Column to group by (-1 = no grouping)
    std::vector<GroupInfo> groups_;   // Group information
    bool groups_dirty_ = true;        // Flag to rebuild groups
    
    // Selection state
    std::vector<uint32_t> selected_row_ids_;  // Store row IDs, not indices
    int last_selected_row_ = -1;              // For shift-click selection
    
    // Helper functions - all work with raw context data via indices
    bool IsRowSelected(uint32_t row_id);
    void ToggleRowSelection(uint32_t row_id);
    void SelectRowRange(int start_row, int end_row);
    void RenderTable(HostContext& ctx, const HostMDSlot& slot);
    void RenderSelectionInfo();
    
    // Filtering functions - work directly with context data
    void ApplyFilters(HostContext& ctx, const HostMDSlot& slot);
    bool PassesFilter(uint32_t row_index, HostContext& ctx, const HostMDSlot& slot) const;
    bool PassesColumnFilter(uint32_t row_index, int column, const ColumnFilter& filter, HostContext& ctx, const HostMDSlot& slot) const;
    
    // Grouping functions - work with indices only
    void ApplyGrouping(HostContext& ctx, const HostMDSlot& slot);
    void BuildGroups(HostContext& ctx, const HostMDSlot& slot);
    void RenderGroupedTable(HostContext& ctx, const HostMDSlot& slot);
    void RenderGroupHeader(const GroupInfo& group, int group_index);
    void RenderGroupRow(uint32_t row_index, int display_row, HostContext& ctx, const HostMDSlot& slot);
    std::string GetGroupKey(uint32_t row_index, int column, HostContext& ctx, const HostMDSlot& slot) const;
    void CalculateGroupAggregates(GroupInfo& group, HostContext& ctx, const HostMDSlot& slot) const;
    
    // Utility functions to access raw data by index
    int64_t GetColumnValue(uint32_t row_index, int column, HostContext& ctx, const HostMDSlot& slot) const;
    uint8_t GetSideValue(uint32_t row_index, const HostMDSlot& slot) const; // Direct immutable access
    const char* GetSideString(uint8_t side) const;
    ImVec4 GetSideColor(uint8_t side) const;
};
