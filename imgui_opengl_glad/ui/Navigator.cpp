#include "Navigator.h"
#include "MarketDataTable.h"
#include <algorithm>
#include <cstring>

Navigator::Navigator() : initialized_(false), max_rows_(0) {
}

Navigator::~Navigator() {
    if (initialized_) {
        Cleanup();
    }
}

void Navigator::Initialize(uint32_t max_rows) {
    max_rows_ = max_rows;
    initialized_ = true;
}

void Navigator::Render(HostContext& ctx, const HostMDSlot& slot, MarketDataTable* table) {
    if (!initialized_) return;
    
    // Update statistics from current data
    UpdateStatistics(ctx, slot);
    
    // Navigator Window (dockable)
    ImGui::Begin("Navigator");
    
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Market Data Navigator");
    ImGui::Separator();
    
    RenderDataCategoriesTree(ctx, slot);
    ImGui::Spacing();
    
    RenderStatisticsTree(ctx, slot);
    ImGui::Spacing();
    
    RenderQuickFiltersTree(table);
    
    ImGui::End(); // Navigator
}

void Navigator::UpdateStatistics(HostContext& ctx, const HostMDSlot& slot) {
    stats_ = DataStats{}; // Reset
    stats_.total_rows = slot.num_rows;
    
    int64_t total_price = 0;
    uint32_t valid_prices = 0;
    
    // Performance critical: single-pass statistics aggregation over all market data rows
    // This loop processes all rows once per render frame to calculate real-time statistics
    for (uint32_t i = 0; i < slot.num_rows; ++i) {
        // Count by side (per md_api.h: 0=unk, 1=bid, 2=ask, 3=trade)
        uint8_t side = slot.side[i];
        switch (side) {
            case 0: stats_.unknown_count++; break;
            case 1: stats_.buy_count++; break;
            case 2: stats_.sell_count++; break;
            case 3: stats_.trade_count++; break;
            default: break;  // Invalid side value
        }
        
        // Sum quantities
        stats_.total_quantity += slot.qty[i];
        
        // Calculate average price
        if (slot.px_n[i] > 0) {
            total_price += slot.px_n[i];
            valid_prices++;
        }
        
        // Count dirty rows
        if (ctx.dirty[i] != 0) {
            stats_.dirty_rows++;
        }
    }
    
    if (valid_prices > 0) {
        stats_.avg_price = total_price / valid_prices;
    }
}

void Navigator::RenderDataCategoriesTree(HostContext& ctx, const HostMDSlot& slot) {
    if (ImGui::TreeNode("Data Categories")) {
        // Market Sides (per md_api.h: 0=unk, 1=bid, 2=ask, 3=trade)
        if (ImGui::TreeNode("By Side")) {
            ImGuiTreeNodeFlags leaf_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            
            // Unknown
            if (stats_.unknown_count > 0) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // Gray
                ImGui::TreeNodeEx("Unknown", leaf_flags);
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::TextDisabled("(%u)", stats_.unknown_count);
            }
            
            // Buy (Bid)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f)); // Green
            ImGui::TreeNodeEx("Buy Orders", leaf_flags);
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::TextDisabled("(%u)", stats_.buy_count);
            
            // Sell (Ask)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f)); // Red
            ImGui::TreeNodeEx("Sell Orders", leaf_flags);
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::TextDisabled("(%u)", stats_.sell_count);
            
            // Trade
            if (stats_.trade_count > 0) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 1.0f, 1.0f)); // Blue
                ImGui::TreeNodeEx("Trades", leaf_flags);
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::TextDisabled("(%u)", stats_.trade_count);
            }
            
            ImGui::TreePop();
        }
        
        // Price Ranges
        if (ImGui::TreeNode("By Price Range")) {
            ImGuiTreeNodeFlags leaf_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            
            // Count orders in different price ranges
            uint32_t low_price = 0, mid_price = 0, high_price = 0;
            // Performance critical: price range classification for UI display
            // Only executed when price range tree node is expanded
            for (uint32_t i = 0; i < slot.num_rows; ++i) {
                int64_t price = slot.px_n[i];
                if (price < 10000) low_price++;
                else if (price < 50000) mid_price++;
                else high_price++;
            }
            
            ImGui::TreeNodeEx("Low (< 100.00)", leaf_flags);
            ImGui::SameLine();
            ImGui::TextDisabled("(%u)", low_price);
            
            ImGui::TreeNodeEx("Mid (100.00 - 500.00)", leaf_flags);
            ImGui::SameLine();
            ImGui::TextDisabled("(%u)", mid_price);
            
            ImGui::TreeNodeEx("High (> 500.00)", leaf_flags);
            ImGui::SameLine();
            ImGui::TextDisabled("(%u)", high_price);
            
            ImGui::TreePop();
        }
        
        // Activity
        if (ImGui::TreeNode("By Activity")) {
            ImGuiTreeNodeFlags leaf_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            
            ImGui::TreeNodeEx("Active (Recently Updated)", leaf_flags);
            ImGui::SameLine();
            ImGui::TextDisabled("(%u)", stats_.dirty_rows);
            
            ImGui::TreeNodeEx("Inactive", leaf_flags);
            ImGui::SameLine();
            ImGui::TextDisabled("(%u)", stats_.total_rows - stats_.dirty_rows);
            
            ImGui::TreePop();
        }
        
        ImGui::TreePop();
    }
}

void Navigator::RenderStatisticsTree(HostContext& ctx, const HostMDSlot& slot) {
    if (ImGui::TreeNode("Statistics")) {
        ImGui::Text("Total Rows: %u", stats_.total_rows);
        ImGui::Text("Total Quantity: %lld", (long long)stats_.total_quantity);
        ImGui::Text("Average Price: %.2f", stats_.avg_price / 100.0);
        ImGui::Spacing();
        
        // Show all side categories (per md_api.h: 0=unk, 1=bid, 2=ask, 3=trade)
        if (stats_.unknown_count > 0) {
            ImGui::Text("Unknown: %u (%.1f%%)", 
                        stats_.unknown_count,
                        stats_.total_rows > 0 ? (stats_.unknown_count * 100.0f / stats_.total_rows) : 0.0f);
        }
        ImGui::Text("Buy Orders: %u (%.1f%%)", 
                    stats_.buy_count, 
                    stats_.total_rows > 0 ? (stats_.buy_count * 100.0f / stats_.total_rows) : 0.0f);
        ImGui::Text("Sell Orders: %u (%.1f%%)", 
                    stats_.sell_count,
                    stats_.total_rows > 0 ? (stats_.sell_count * 100.0f / stats_.total_rows) : 0.0f);
        if (stats_.trade_count > 0) {
            ImGui::Text("Trades: %u (%.1f%%)", 
                        stats_.trade_count,
                        stats_.total_rows > 0 ? (stats_.trade_count * 100.0f / stats_.total_rows) : 0.0f);
        }
        ImGui::Spacing();
        
        ImGui::Text("Recently Updated: %u", stats_.dirty_rows);
        
        // Queue statistics
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Queue Stats:");
        ImGui::Text("  Capacity: %zu", ctx.q.buf.size());
        ImGui::Text("  Head: %u", ctx.q.head.load(std::memory_order_relaxed));
        ImGui::Text("  Tail: %u", ctx.q.tail.load(std::memory_order_relaxed));
        
        ImGui::TreePop();
    }
}

void Navigator::RenderQuickFiltersTree(MarketDataTable* table) {
    if (ImGui::TreeNode("Quick Filters")) {
        // Show All - clears all filters
        if (ImGui::Selectable("Show All", false)) {
            if (table) {
                table->ClearAllFilters();
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Clear all filters and show all rows");
        }
        
        // Show Buy Only - filter to side == 1
        if (ImGui::Selectable("Show Buy Only", false)) {
            if (table) {
                table->ClearAllFilters();
                ColumnFilter filter;
                filter.enabled = true;
                filter.type = FILTER_TEXT_EQUALS;
                strncpy(filter.text_value, "Buy", sizeof(filter.text_value) - 1);
                filter.text_value[sizeof(filter.text_value) - 1] = '\0';
                table->SetColumnFilter(4, filter);  // Column 4 is SIDE
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Show only Buy orders (side = 1)");
        }
        
        // Show Sell Only - filter to side == 2
        if (ImGui::Selectable("Show Sell Only", false)) {
            if (table) {
                table->ClearAllFilters();
                ColumnFilter filter;
                filter.enabled = true;
                filter.type = FILTER_TEXT_EQUALS;
                strncpy(filter.text_value, "Sell", sizeof(filter.text_value) - 1);
                filter.text_value[sizeof(filter.text_value) - 1] = '\0';
                table->SetColumnFilter(4, filter);  // Column 4 is SIDE
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Show only Sell orders (side = 2)");
        }
        
        // Show Large Orders (Qty > 1000)
        if (ImGui::Selectable("Show Large Orders (Qty > 1000)", false)) {
            if (table) {
                table->ClearAllFilters();
                ColumnFilter filter;
                filter.enabled = true;
                filter.type = FILTER_NUMERIC_GREATER;
                filter.numeric_value = 1000;
                table->SetColumnFilter(3, filter);  // Column 3 is QTY
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Show only orders with quantity > 1000");
        }
        
        // Show Recent Updates - filter to dirty rows
        if (ImGui::Selectable("Show Recent Updates", false)) {
            if (table) {
                table->ClearAllFilters();
                // Note: This would require a special filter type for dirty rows
                // For now, we'll just clear filters as a placeholder
                // TODO: Implement dirty row filtering in MarketDataTable
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Show only recently updated rows");
        }
        
        ImGui::TreePop();
    }
}

void Navigator::Cleanup() {
    initialized_ = false;
}
