#pragma once

#include <imgui.h>
#include <vector>
#include <string>
#include "../core/main_context.h"

// Forward declarations
struct HostContext;
struct HostMDSlot;

/**
 * @brief Navigator component for hierarchical data browsing
 * 
 * Provides a tree-based navigation interface that can display:
 * - Market data categories (grouped by side: Buy/Sell)
 * - Data statistics and aggregations
 * - Quick filters and views
 * 
 * Shares the same HostContext and HostMDSlot data source as MarketDataTable
 */
class Navigator {
public:
    Navigator();
    ~Navigator();
    
    /**
     * @brief Initialize the navigator
     * @param max_rows Maximum number of rows to handle
     */
    void Initialize(uint32_t max_rows);
    
    /**
     * @brief Render the navigator window
     * @param ctx HostContext with market data
     * @param slot HostMDSlot with raw data buffers
     */
    void Render(HostContext& ctx, const HostMDSlot& slot);
    
    /**
     * @brief Cleanup resources
     */
    void Cleanup();

private:
    // State
    bool initialized_;
    uint32_t max_rows_;
    
    // Cached statistics (updated during render)
    struct DataStats {
        uint32_t total_rows = 0;
        uint32_t unknown_count = 0;  // side == 0
        uint32_t buy_count = 0;      // side == 1
        uint32_t sell_count = 0;     // side == 2
        uint32_t trade_count = 0;    // side == 3
        int64_t total_quantity = 0;
        int64_t avg_price = 0;
        uint32_t dirty_rows = 0;
    };
    DataStats stats_;
    
    // Helper rendering methods
    void RenderDataCategoriesTree(HostContext& ctx, const HostMDSlot& slot);
    void RenderStatisticsTree(HostContext& ctx, const HostMDSlot& slot);
    void RenderQuickFiltersTree();
    
    // Data analysis helpers
    void UpdateStatistics(HostContext& ctx, const HostMDSlot& slot);
};
