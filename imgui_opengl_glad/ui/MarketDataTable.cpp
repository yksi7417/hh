#include "MarketDataTable.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <map>

MarketDataTable::MarketDataTable()
    : num_rows_(0), last_selected_row_(-1), filters_dirty_(true), groups_dirty_(true) {
    // Initialize all column filters
    // Performance critical: initialization loop for column filters
    for (int i = 0; i < 5; i++) {
        column_filters_[i] = ColumnFilter{};
    }
}

MarketDataTable::~MarketDataTable() {}

void MarketDataTable::Initialize(uint32_t max_rows) {
    num_rows_ = max_rows;
    // Pre-allocate index vectors (much smaller than data)
    all_row_indices_.reserve(max_rows);
    filtered_indices_.reserve(max_rows);

    // Initialize all row indices (0, 1, 2, ..., max_rows-1)
    all_row_indices_.clear();
    // Performance critical: initialization loop for row index array
    for (uint32_t i = 0; i < max_rows; ++i) {
        all_row_indices_.push_back(i);
    }
}

void MarketDataTable::UpdateFromContext(HostContext& ctx, const HostMDSlot& slot,
                                        bool should_refresh) {
    // Process dirty queue - no copying, just queue management
    uint32_t id;
    // Performance critical: tight loop processing queued UI updates
    while (ctx.q.pop(id)) {
        if (id < ctx.num_rows) {
            ctx.dirty[id] = 1;
        }
    }

    if (!should_refresh)
        return;

    // Update snapshots in-place within the context (no copying to our data)
    // Performance critical: bulk processing all dirty rows for UI refresh
    for (uint32_t i = 0; i < ctx.num_rows; ++i) {
        if (ctx.dirty[i]) {
            ctx.dirty[i] = 0;
            HostContext::RowSnap tmp{};
            bool ok = false;
            // Performance critical: retry loop for consistent row snapshot
            for (int tries = 0; tries < 2 && !ok; ++tries) {
                ok = row_snapshot(&ctx, &slot, i, tmp);
            }
            if (ok) {
                if (std::memcmp(&tmp, &ctx.last[i], sizeof(tmp)) != 0) {
                    ctx.last[i] = tmp;  // Update in place in the context
                }
            }
        }
    }

    // Update our index count if context changed
    if (ctx.num_rows != num_rows_) {
        num_rows_ = ctx.num_rows;
        all_row_indices_.clear();
        // Performance critical: rebuilding row index array for UI display
        for (uint32_t i = 0; i < num_rows_; ++i) {
            all_row_indices_.push_back(i);
        }
    }

    // Mark filters and groups as dirty since data may have changed
    filters_dirty_ = true;
    groups_dirty_ = true;
}

bool MarketDataTable::IsRowSelected(uint32_t row_id) {
    return std::find(selected_row_ids_.begin(), selected_row_ids_.end(), row_id) !=
           selected_row_ids_.end();
}

void MarketDataTable::ToggleRowSelection(uint32_t row_id) {
    auto it = std::find(selected_row_ids_.begin(), selected_row_ids_.end(), row_id);
    if (it != selected_row_ids_.end()) {
        selected_row_ids_.erase(it);  // Deselect
    } else {
        selected_row_ids_.push_back(row_id);  // Select
    }
}

void MarketDataTable::SelectRowRange(int start_row, int end_row) {
    if (start_row > end_row)
        std::swap(start_row, end_row);

    auto& display_indices = HasActiveFilters() ? filtered_indices_ : all_row_indices_;
    // Performance critical: loop over row range for multi-selection
    for (int row = start_row; row <= end_row; row++) {
        if (row >= 0 && row < (int)display_indices.size()) {
            uint32_t row_id = display_indices[row];  // Use the actual row index as ID
            if (!IsRowSelected(row_id)) {
                selected_row_ids_.push_back(row_id);
            }
        }
    }
}

void MarketDataTable::ClearSelection() {
    selected_row_ids_.clear();
    last_selected_row_ = -1;
}

void MarketDataTable::Render(HostContext& ctx, const HostMDSlot& slot) {
    if (!ImGui::Begin("MarketData")) {
        ImGui::End();
        return;
    }

    // Apply filters if needed
    ApplyFilters(ctx, slot);

    // Apply grouping if needed
    if (HasActiveGrouping()) {
        ApplyGrouping(ctx, slot);
    }

    RenderSelectionInfo();
    ImGui::Separator();

    // Render appropriate table based on grouping state
    if (HasActiveGrouping()) {
        RenderGroupedTable(ctx, slot);
    } else {
        RenderTable(ctx, slot);
    }

    ImGui::End();
}

void MarketDataTable::RenderSelectionInfo() {
    ImGui::Text("Total Rows: %d", (int)num_rows_);
    if (HasActiveFilters()) {
        ImGui::SameLine();
        ImVec4 filterColor = ImVec4(0.0f, 0.7f, 1.0f, 1.0f);
        ImGui::TextColored(filterColor, "(Filtered: %d)", (int)filtered_indices_.size());
        ImGui::SameLine();
        if (ImGui::SmallButton("Clear Filters")) {
            ClearAllFilters();
        }
    }

    // Grouping controls
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();

    if (HasActiveGrouping()) {
        const char* column_names[] = {"ID", "Timestamp", "Price", "Quantity", "Side"};
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Grouped by: %s",
                           column_names[group_by_column_]);
        ImGui::SameLine();
        if (ImGui::SmallButton("Clear Grouping")) {
            ClearGrouping();
        }
    } else {
        if (ImGui::SmallButton("Group By...")) {
            ImGui::OpenPopup("GroupByPopup");
        }

        if (ImGui::BeginPopup("GroupByPopup")) {
            const char* column_names[] = {"ID", "Timestamp", "Price", "Quantity", "Side"};
            // Performance critical: column selection loop for grouping UI
            for (int i = 0; i < 5; i++) {
                if (ImGui::MenuItem(column_names[i])) {
                    SetGroupByColumn(i);
                }
            }
            ImGui::EndPopup();
        }
    }

    ImGui::Text("Selected: %d rows", (int)selected_row_ids_.size());

    // Show selected Row IDs (limited to first 10 for display)
    if (!selected_row_ids_.empty()) {
        ImGui::Text("Selected IDs: ");
        ImGui::SameLine();
        std::string selected_ids_text;
        for (size_t i = 0; i < selected_row_ids_.size() && i < 10; ++i) {
            if (i > 0)
                selected_ids_text += ", ";
            selected_ids_text += std::to_string(selected_row_ids_[i]);
        }
        if (selected_row_ids_.size() > 10) {
            selected_ids_text += "...";
        }
        ImGui::TextWrapped("%s", selected_ids_text.c_str());

        // Clear selection button
        if (ImGui::Button("Clear Selection")) {
            ClearSelection();
        }
    }
}

void MarketDataTable::RenderTable(HostContext& ctx, const HostMDSlot& slot) {
    // Use filtered indices for display - NO COPYING OF DATA!
    auto& display_indices = HasActiveFilters() ? filtered_indices_ : all_row_indices_;

    // Market data table with virtualization for large datasets
    ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable |
                            ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;

    if (ImGui::BeginTable("MarketDataTable", 5, flags)) {
        // Column setup
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn(
            "ID", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 80.0f, 0);
        ImGui::TableSetupColumn("Timestamp", ImGuiTableColumnFlags_WidthFixed, 120.0f, 1);
        ImGui::TableSetupColumn("Price", ImGuiTableColumnFlags_WidthFixed, 100.0f, 2);
        ImGui::TableSetupColumn("Quantity", ImGuiTableColumnFlags_WidthFixed, 100.0f, 3);
        ImGui::TableSetupColumn("Side", ImGuiTableColumnFlags_WidthStretch, 0.0f, 4);
        ImGui::TableHeadersRow();

        // Add filter row as the first row after headers
        ImGui::TableNextRow(ImGuiTableRowFlags_None);

        // Performance critical: filter UI rendering loop for all columns
        for (int column = 0; column < 5; column++) {
            ImGui::TableSetColumnIndex(column);
            ImGui::PushID(column);

            ColumnFilter& filter = column_filters_[column];

            // Create unique IDs for each filter control
            char filter_id[64];
            snprintf(filter_id, sizeof(filter_id), "##filter_%d", column);

            if (column == 0 || column == 1 || column == 2 || column == 3) {  // Numeric columns
                ImGui::PushItemWidth(-1);
                char input_buffer[64];
                snprintf(input_buffer, sizeof(input_buffer), "%lld",
                         (long long)filter.numeric_value);

                if (ImGui::InputText(filter_id, input_buffer, sizeof(input_buffer),
                                     ImGuiInputTextFlags_EnterReturnsTrue)) {
                    filter.numeric_value = strtoll(input_buffer, nullptr, 10);
                    filter.type = FILTER_NUMERIC_EQUALS;
                    filter.enabled = (filter.numeric_value != 0);
                    filters_dirty_ = true;
                }

                // Right-click context menu for filter type
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                    ImGui::OpenPopup("FilterType");
                }

                if (ImGui::BeginPopup("FilterType")) {
                    if (ImGui::MenuItem("Equals", nullptr, filter.type == FILTER_NUMERIC_EQUALS)) {
                        filter.type = FILTER_NUMERIC_EQUALS;
                        filter.enabled = true;
                        filters_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Greater than", nullptr,
                                        filter.type == FILTER_NUMERIC_GREATER)) {
                        filter.type = FILTER_NUMERIC_GREATER;
                        filter.enabled = true;
                        filters_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Less than", nullptr, filter.type == FILTER_NUMERIC_LESS)) {
                        filter.type = FILTER_NUMERIC_LESS;
                        filter.enabled = true;
                        filters_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Clear", nullptr, !filter.enabled)) {
                        filter.enabled = false;
                        filter.type = FILTER_NONE;
                        filters_dirty_ = true;
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopItemWidth();

            } else {  // Side column (text/enum)
                ImGui::PushItemWidth(-1);
                if (ImGui::InputText(filter_id, filter.text_value, sizeof(filter.text_value),
                                     ImGuiInputTextFlags_EnterReturnsTrue)) {
                    filter.type = FILTER_TEXT_CONTAINS;
                    filter.enabled = (strlen(filter.text_value) > 0);
                    filters_dirty_ = true;
                }

                if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                    ImGui::OpenPopup("FilterType");
                }

                if (ImGui::BeginPopup("FilterType")) {
                    if (ImGui::MenuItem("Contains", nullptr, filter.type == FILTER_TEXT_CONTAINS)) {
                        filter.type = FILTER_TEXT_CONTAINS;
                        filter.enabled = true;
                        filters_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Equals", nullptr, filter.type == FILTER_TEXT_EQUALS)) {
                        filter.type = FILTER_TEXT_EQUALS;
                        filter.enabled = true;
                        filters_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Clear", nullptr, !filter.enabled)) {
                        filter.enabled = false;
                        filter.type = FILTER_NONE;
                        filters_dirty_ = true;
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopItemWidth();
            }

            ImGui::PopID();
        }

        // Handle sorting - sort indices based on values from raw memory
        if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs()) {
            if (sort_specs->SpecsDirty) {
                if (display_indices.size() > 1) {
                    std::sort(
                        display_indices.begin(), display_indices.end(),
                        [&](uint32_t a_index, uint32_t b_index) {
                            // Performance critical: multi-column sort comparison loop
                            for (int n = 0; n < sort_specs->SpecsCount; n++) {
                                const ImGuiTableColumnSortSpecs* sort_spec = &sort_specs->Specs[n];

                                int64_t a_val =
                                    GetColumnValue(a_index, sort_spec->ColumnIndex, ctx, slot);
                                int64_t b_val =
                                    GetColumnValue(b_index, sort_spec->ColumnIndex, ctx, slot);

                                int delta = (a_val < b_val) ? -1 : (a_val > b_val) ? 1 : 0;

                                if (delta != 0) {
                                    return (sort_spec->SortDirection ==
                                            ImGuiSortDirection_Ascending)
                                               ? (delta < 0)
                                               : (delta > 0);
                                }
                            }
                            return false;
                        });
                }
                sort_specs->SpecsDirty = false;
            }
        }

        // Use clipper for efficient rendering of large lists
        ImGuiListClipper clipper;
        clipper.Begin((int)display_indices.size());

        // Performance critical: main UI rendering loop with virtual scrolling
        while (clipper.Step()) {
            // Performance critical: render only visible rows for performance
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                uint32_t row_index = display_indices[row];
                const HostContext::RowSnap& snap = ctx.last[row_index];
                bool is_selected = IsRowSelected(row_index);

                ImGui::TableNextRow();

                // Make the entire row selectable
                ImGui::TableSetColumnIndex(0);
                char label[32];
                snprintf(label, sizeof(label), "##row_%d", row);

                if (ImGui::Selectable(label, is_selected,
                                      ImGuiSelectableFlags_SpanAllColumns |
                                          ImGuiSelectableFlags_AllowItemOverlap)) {
                    ImGuiIO& input_io = ImGui::GetIO();

                    if (input_io.KeyCtrl) {
                        // Ctrl+Click: Toggle selection
                        ToggleRowSelection(row_index);
                        last_selected_row_ = row;
                    } else if (input_io.KeyShift && last_selected_row_ != -1) {
                        // Shift+Click: Select range
                        SelectRowRange(last_selected_row_, row);
                    } else {
                        // Normal click: Select only this row
                        selected_row_ids_.clear();
                        selected_row_ids_.push_back(row_index);
                        last_selected_row_ = row;
                    }
                }

                // Draw the row content - accessing raw memory directly
                ImGui::SameLine();
                ImGui::Text("%u", row_index);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%lld", (long long)snap.ts);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%lld", (long long)snap.px);

                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%lld", (long long)snap.qty);

                ImGui::TableSetColumnIndex(4);
                uint8_t side_val = GetSideValue(row_index, slot);  // Direct immutable access
                ImVec4 side_color = GetSideColor(side_val);
                ImGui::TextColored(side_color, "%s", GetSideString(side_val));
            }
        }

        ImGui::EndTable();
    }
}

// Filter management methods
void MarketDataTable::ClearAllFilters() {
    // Performance critical: filter clearing loop for all columns
    for (int i = 0; i < 5; i++) {
        column_filters_[i] = ColumnFilter{};
    }
    filters_dirty_ = true;
}

void MarketDataTable::SetColumnFilter(int column, const ColumnFilter& filter) {
    if (column >= 0 && column < 5) {
        column_filters_[column] = filter;
        filters_dirty_ = true;
    }
}

bool MarketDataTable::HasActiveFilters() const {
    // Performance critical: filter checking loop for all columns
    for (int i = 0; i < 5; i++) {
        if (column_filters_[i].enabled) {
            return true;
        }
    }
    return false;
}

// Filter management methods - work with indices only
void MarketDataTable::ApplyFilters(HostContext& ctx, const HostMDSlot& slot) {
    if (!filters_dirty_)
        return;

    filtered_indices_.clear();

    if (!HasActiveFilters()) {
        // No filters, use all indices
        filtered_indices_ = all_row_indices_;
    } else {
        // Apply filters by testing each row index
        // Performance critical: filtering loop over all rows
        for (uint32_t row_index : all_row_indices_) {
            if (PassesFilter(row_index, ctx, slot)) {
                filtered_indices_.push_back(row_index);
            }
        }
    }

    filters_dirty_ = false;
}

bool MarketDataTable::PassesFilter(uint32_t row_index, HostContext& ctx,
                                   const HostMDSlot& slot) const {
    // Performance critical: filter validation loop for all columns
    for (int i = 0; i < 5; i++) {
        if (column_filters_[i].enabled) {
            if (!PassesColumnFilter(row_index, i, column_filters_[i], ctx, slot)) {
                return false;
            }
        }
    }
    return true;
}

bool MarketDataTable::PassesColumnFilter(uint32_t row_index, int column, const ColumnFilter& filter,
                                         HostContext& ctx, const HostMDSlot& slot) const {
    if (row_index >= ctx.num_rows)
        return false;

    switch (column) {
    case 0:
    case 1:
    case 2:
    case 3:  // Numeric columns
    {
        int64_t value = GetColumnValue(row_index, column, ctx, slot);
        switch (filter.type) {
        case FILTER_NUMERIC_EQUALS:
            return value == filter.numeric_value;
        case FILTER_NUMERIC_GREATER:
            return value > filter.numeric_value;
        case FILTER_NUMERIC_LESS:
            return value < filter.numeric_value;
        case FILTER_NUMERIC_GREATER_EQUAL:
            return value >= filter.numeric_value;
        case FILTER_NUMERIC_LESS_EQUAL:
            return value <= filter.numeric_value;
        case FILTER_NUMERIC_RANGE:
            return value >= filter.range_min && value <= filter.range_max;
        default:
            return true;
        }
    }

    case 4:  // Side column (text)
    {
        uint8_t side_val = GetSideValue(row_index, slot);  // Direct immutable access
        const char* text = GetSideString(side_val);
        switch (filter.type) {
        case FILTER_TEXT_CONTAINS:
            return strstr(text, filter.text_value) != nullptr;
        case FILTER_TEXT_EQUALS:
            return strcmp(text, filter.text_value) == 0;
        case FILTER_TEXT_STARTS_WITH:
            return strncmp(text, filter.text_value, strlen(filter.text_value)) == 0;
        case FILTER_TEXT_ENDS_WITH: {
            size_t text_len = strlen(text);
            size_t filter_len = strlen(filter.text_value);
            if (filter_len > text_len)
                return false;
            return strcmp(text + text_len - filter_len, filter.text_value) == 0;
        }
        default:
            return true;
        }
    }
    }
    return true;
}

// Grouping management methods - simplified for now
void MarketDataTable::SetGroupByColumn(int column) {
    if (column >= 0 && column < 5) {
        group_by_column_ = column;
        groups_dirty_ = true;
    }
}

void MarketDataTable::ClearGrouping() {
    group_by_column_ = -1;
    groups_.clear();
    groups_dirty_ = true;
}

void MarketDataTable::ApplyGrouping(HostContext& ctx, const HostMDSlot& slot) {
    if (!groups_dirty_ || group_by_column_ < 0)
        return;

    BuildGroups(ctx, slot);
    groups_dirty_ = false;
}

void MarketDataTable::BuildGroups(HostContext& ctx, const HostMDSlot& slot) {
    groups_.clear();

    auto& display_indices = HasActiveFilters() ? filtered_indices_ : all_row_indices_;

    // Create a map to group rows by the selected column
    std::map<std::string, std::vector<uint32_t>> group_map;

    // Performance critical: grouping all displayed rows by column value
    for (uint32_t row_index : display_indices) {
        std::string group_key = GetGroupKey(row_index, group_by_column_, ctx, slot);
        group_map[group_key].push_back(row_index);
    }

    // Convert map to vector of GroupInfo
    groups_.reserve(group_map.size());
    // Performance critical: converting group map to display vector
    for (const auto& pair : group_map) {
        GroupInfo group;
        group.group_key = pair.first;
        group.row_indices = pair.second;
        group.row_count = (int)pair.second.size();

        CalculateGroupAggregates(group, ctx, slot);
        groups_.push_back(group);
    }

    // Sort groups by group key
    std::sort(groups_.begin(), groups_.end(),
              [](const GroupInfo& a, const GroupInfo& b) { return a.group_key < b.group_key; });
}

void MarketDataTable::RenderGroupedTable(HostContext& ctx, const HostMDSlot& slot) {
    ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY |
                            ImGuiTableFlags_SizingStretchProp;

    if (ImGui::BeginTable("GroupedMarketDataTable", 5, flags)) {
        // Column setup - same as regular table
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 80.0f, 0);
        ImGui::TableSetupColumn("Timestamp", ImGuiTableColumnFlags_WidthFixed, 120.0f, 1);
        ImGui::TableSetupColumn("Price", ImGuiTableColumnFlags_WidthFixed, 100.0f, 2);
        ImGui::TableSetupColumn("Quantity", ImGuiTableColumnFlags_WidthFixed, 100.0f, 3);
        ImGui::TableSetupColumn("Side", ImGuiTableColumnFlags_WidthStretch, 0.0f, 4);
        ImGui::TableHeadersRow();

        // Render grouped data
        // Performance critical: main loop for rendering grouped table
        for (int group_index = 0; group_index < (int)groups_.size(); group_index++) {
            GroupInfo& group = groups_[group_index];

            // Render group header
            RenderGroupHeader(group, group_index);

            // Render group rows if not collapsed
            if (!group.is_collapsed) {
                // Performance critical: loop rendering all rows within group
                for (int i = 0; i < (int)group.row_indices.size(); i++) {
                    uint32_t row_index = group.row_indices[i];
                    RenderGroupRow(row_index, i, ctx, slot);
                }
            }
        }

        ImGui::EndTable();
    }
}

void MarketDataTable::RenderGroupHeader(const GroupInfo& group, int group_index) {
    ImGui::TableNextRow();

    // Set a subtle background color for the entire group header row
    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(70, 90, 120, 60));

    ImGui::TableSetColumnIndex(0);

    // Create collapsible header with group name and count
    char group_header[256];
    snprintf(group_header, sizeof(group_header), "%s (%d rows)##group_%d", group.group_key.c_str(),
             group.row_count, group_index);

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.4f, 0.6f, 0.8f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.5f, 0.7f, 0.9f, 0.8f));

    bool header_open = ImGui::CollapsingHeader(group_header, ImGuiTreeNodeFlags_DefaultOpen);

    // Toggle collapse state when header is clicked
    GroupInfo& mutable_group = const_cast<GroupInfo&>(group);
    mutable_group.is_collapsed = !header_open;

    ImGui::PopStyleColor(2);

    // Show aggregate information aligned with columns
    ImGui::TableSetColumnIndex(1);
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Avg TS: %lld",
                       (long long)group.avg_timestamp);

    ImGui::TableSetColumnIndex(2);
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Avg Price: %lld",
                       (long long)group.avg_price);

    ImGui::TableSetColumnIndex(3);
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Total Qty: %lld",
                       (long long)group.total_qty);

    ImGui::TableSetColumnIndex(4);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Count: %d", group.row_count);
}

void MarketDataTable::RenderGroupRow(uint32_t row_index, int display_row, HostContext& ctx,
                                     const HostMDSlot& slot) {
    const HostContext::RowSnap& snap = ctx.last[row_index];
    bool is_selected = IsRowSelected(row_index);

    ImGui::TableNextRow();

    // Make the entire row selectable
    ImGui::TableSetColumnIndex(0);
    char label[32];
    snprintf(label, sizeof(label), "##row_%d", display_row);

    if (ImGui::Selectable(
            label, is_selected,
            ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
        ImGuiIO& input_io = ImGui::GetIO();

        if (input_io.KeyCtrl) {
            // Ctrl+Click: Toggle selection
            ToggleRowSelection(row_index);
            last_selected_row_ = display_row;
        } else if (input_io.KeyShift && last_selected_row_ != -1) {
            // Shift+Click: Select range
            SelectRowRange(last_selected_row_, display_row);
        } else {
            // Normal click: Select only this row
            selected_row_ids_.clear();
            selected_row_ids_.push_back(row_index);
            last_selected_row_ = display_row;
        }
    }

    // Indent the first column to show it's part of a group
    ImGui::SameLine();
    ImGui::Indent(20.0f);
    ImGui::Text("%u", row_index);
    ImGui::Unindent(20.0f);

    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%lld", (long long)snap.ts);

    ImGui::TableSetColumnIndex(2);
    ImGui::Text("%lld", (long long)snap.px);

    ImGui::TableSetColumnIndex(3);
    ImGui::Text("%lld", (long long)snap.qty);

    ImGui::TableSetColumnIndex(4);
    uint8_t side_val = GetSideValue(row_index, slot);  // Direct immutable access
    ImVec4 side_color = GetSideColor(side_val);
    ImGui::TextColored(side_color, "%s", GetSideString(side_val));
}

std::string MarketDataTable::GetGroupKey(uint32_t row_index, int column, HostContext& ctx,
                                         const HostMDSlot& slot) const {
    int64_t value = GetColumnValue(row_index, column, ctx, slot);
    if (column == 4) {
        uint8_t side_val = GetSideValue(row_index, slot);  // Direct immutable access
        return std::string(GetSideString(side_val));
    }
    return std::to_string(value);
}

void MarketDataTable::CalculateGroupAggregates(GroupInfo& group, HostContext& ctx,
                                               const HostMDSlot& slot) const {
    group.total_qty = 0;
    int64_t total_price = 0;
    int64_t total_timestamp = 0;

    for (uint32_t row_index : group.row_indices) {
        if (row_index < ctx.num_rows) {
            const HostContext::RowSnap& snap = ctx.last[row_index];
            group.total_qty += snap.qty;
            total_price += snap.px;
            total_timestamp += snap.ts;
        }
    }

    group.avg_price = group.row_count > 0 ? total_price / group.row_count : 0;
    group.avg_timestamp = group.row_count > 0 ? total_timestamp / group.row_count : 0;
}

// Utility functions to access raw data by index - NO COPYING!
int64_t MarketDataTable::GetColumnValue(uint32_t row_index, int column, HostContext& ctx,
                                        const HostMDSlot& slot) const {
    if (row_index >= ctx.num_rows)
        return 0;

    const HostContext::RowSnap& snap = ctx.last[row_index];
    switch (column) {
    case 0:
        return (int64_t)row_index;  // ID
    case 1:
        return snap.ts;  // Timestamp
    case 2:
        return snap.px;  // Price
    case 3:
        return snap.qty;  // Quantity
    case 4:
        return (int64_t)GetSideValue(row_index, slot);  // Side (immutable)
    default:
        return 0;
    }
}

// Direct access to immutable side value
uint8_t MarketDataTable::GetSideValue(uint32_t row_index, const HostMDSlot& slot) const {
    if (row_index >= slot.num_rows)
        return 0;
    return slot.side[row_index];  // Direct access - no snapshot needed since it's immutable
}

// Utility functions
const char* MarketDataTable::GetSideString(uint8_t side) const {
    switch (side) {
    case 0:
        return "Buy";
    case 1:
        return "Sell";
    default:
        return "Unknown";
    }
}

ImVec4 MarketDataTable::GetSideColor(uint8_t side) const {
    switch (side) {
    case 0:
        return ImVec4(0.0f, 1.0f, 0.0f, 1.0f);  // Green for Buy
    case 1:
        return ImVec4(1.0f, 0.3f, 0.3f, 1.0f);  // Red for Sell
    default:
        return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);  // White for Unknown
    }
}
