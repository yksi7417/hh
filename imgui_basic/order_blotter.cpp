#include "order_blotter.h"
#include <algorithm>
#include <random>
#include <cmath>
#include <cstring>
#include <map>

// Sample data arrays for random generation
static const char* first_names[] = {
    "John", "Jane", "Mike", "Sarah", "Tom", "Lisa", "Bob", "Amy", "David", "Emma",
    "Chris", "Anna", "Mark", "Kate", "Paul", "Laura", "Steve", "Maria", "Dan", "Sophie",
    "Alex", "Grace", "Matt", "Eva", "Nick", "Lily", "Sam", "Rose", "Ben", "Zoe",
    "Jake", "Mia", "Luke", "Ella", "Ryan", "Chloe", "Adam", "Olivia", "Josh", "Maya"
};

static const char* last_names[] = {
    "Smith", "Johnson", "Brown", "Davis", "Wilson", "Chen", "Martinez", "Anderson",
    "Garcia", "Miller", "Taylor", "Moore", "Jackson", "Martin", "Lee", "Thompson",
    "White", "Harris", "Clark", "Lewis", "Walker", "Hall", "Allen", "Young",
    "King", "Wright", "Lopez", "Hill", "Scott", "Green", "Adams", "Baker",
    "Gonzalez", "Nelson", "Carter", "Mitchell", "Perez", "Roberts", "Turner", "Phillips"
};

static const char* products[] = {
    "Widget A", "Widget B", "Widget C", "Widget D", "Widget E",
    "Gadget Pro", "Gadget Lite", "Gadget Max", "Device X1", "Device X2",
    "Tool Basic", "Tool Premium", "Kit Standard", "Kit Deluxe", "Module Alpha",
    "Module Beta", "Component Z", "Assembly Unit", "Part 2000", "Part 3000"
};

static const char* statuses[] = {
    "Pending", "Processing", "Shipped", "Delivered", "Cancelled"
};

OrderBlotter::OrderBlotter() 
    : orders_generated_(false), last_selected_row_(-1), filters_dirty_(true) {
    // Initialize all column filters
    for (int i = 0; i < 6; i++) {
        column_filters_[i] = ColumnFilter{};
    }
}

OrderBlotter::~OrderBlotter() {
}

void OrderBlotter::Initialize(int order_count) {
    GenerateOrders(order_count);
}

void OrderBlotter::GenerateOrders(int count) {
    if (orders_generated_) return;
    
    orders_.reserve(count);
    std::random_device rd;
    std::mt19937 gen(rd());
    
    std::uniform_int_distribution<> first_name_dist(0, sizeof(first_names)/sizeof(first_names[0]) - 1);
    std::uniform_int_distribution<> last_name_dist(0, sizeof(last_names)/sizeof(last_names[0]) - 1);
    std::uniform_int_distribution<> product_dist(0, sizeof(products)/sizeof(products[0]) - 1);
    std::uniform_int_distribution<> status_dist(0, sizeof(statuses)/sizeof(statuses[0]) - 1);
    std::uniform_int_distribution<> quantity_dist(1, 20);
    std::uniform_real_distribution<float> price_dist(5.99f, 299.99f);
    
    for (int i = 0; i < count; ++i) {
        Order order;
        order.id = 1000 + i;
        
        // Use snprintf for safe string formatting into fixed buffers
        snprintf(order.customer, sizeof(order.customer), "%s %s", 
                 first_names[first_name_dist(gen)], 
                 last_names[last_name_dist(gen)]);
        
        strncpy_s(order.product, sizeof(order.product), products[product_dist(gen)], _TRUNCATE);
        
        order.quantity = quantity_dist(gen);
        order.price = price_dist(gen);
        
        strncpy_s(order.status, sizeof(order.status), statuses[status_dist(gen)], _TRUNCATE);
        
        orders_.push_back(order);
    }
    
    orders_generated_ = true;
}

bool OrderBlotter::IsOrderSelected(int order_id) {
    return std::find(selected_order_ids_.begin(), selected_order_ids_.end(), order_id) != selected_order_ids_.end();
}

void OrderBlotter::ToggleOrderSelection(int order_id) {
    auto it = std::find(selected_order_ids_.begin(), selected_order_ids_.end(), order_id);
    if (it != selected_order_ids_.end()) {
        selected_order_ids_.erase(it);  // Deselect
    } else {
        selected_order_ids_.push_back(order_id);  // Select
    }
}

void OrderBlotter::SelectOrderRange(int start_row, int end_row) {
    if (start_row > end_row) std::swap(start_row, end_row);
    
    for (int row = start_row; row <= end_row; row++) {
        if (row >= 0 && row < (int)orders_.size()) {
            int order_id = orders_[row].id;
            if (!IsOrderSelected(order_id)) {
                selected_order_ids_.push_back(order_id);
            }
        }
    }
}

void OrderBlotter::ClearSelection() {
    selected_order_ids_.clear();
    last_selected_row_ = -1;
}

void OrderBlotter::Render() {
    ImGui::Begin("Orders");
    
    // Apply filters if needed
    ApplyFilters();
    
    // Apply grouping if needed
    if (HasActiveGrouping()) {
        ApplyGrouping();
    }
    
    RenderSelectionInfo();
    ImGui::Separator();
    
    // Render appropriate table based on grouping state
    if (HasActiveGrouping()) {
        RenderGroupedTable();
    } else {
        RenderTable();
    }
    
    ImGui::End();
}

void OrderBlotter::RenderSelectionInfo() {
    ApplyFilters(); // Ensure filters are applied
    
    ImGui::Text("Total Orders: %d", (int)orders_.size());
    if (HasActiveFilters()) {
        ImGui::SameLine();
        ImVec4 filterColor = ImVec4(0.0f, 0.7f, 1.0f, 1.0f);
        ImGui::TextColored(filterColor, "(Filtered: %d)", (int)filtered_orders_.size());
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
        const char* column_names[] = {"Order ID", "Customer", "Product", "Quantity", "Price", "Status"};
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Grouped by: %s", column_names[group_by_column_]);
        ImGui::SameLine();
        if (ImGui::SmallButton("Clear Grouping")) {
            ClearGrouping();
        }
    } else {
        if (ImGui::SmallButton("Group By...")) {
            ImGui::OpenPopup("GroupByPopup");
        }
        
        if (ImGui::BeginPopup("GroupByPopup")) {
            const char* column_names[] = {"Order ID", "Customer", "Product", "Quantity", "Price", "Status"};
            for (int i = 0; i < 6; i++) {
                if (ImGui::MenuItem(column_names[i])) {
                    SetGroupByColumn(i);
                }
            }
            ImGui::EndPopup();
        }
    }
    
    ImGui::Text("Selected: %d orders", (int)selected_order_ids_.size());
    
    // Show selected Order IDs (limited to first 10 for display)
    if (!selected_order_ids_.empty()) {
        ImGui::Text("Selected IDs: ");
        ImGui::SameLine();
        std::string selected_ids_text;
        for (size_t i = 0; i < selected_order_ids_.size() && i < 10; ++i) {
            if (i > 0) selected_ids_text += ", ";
            selected_ids_text += std::to_string(selected_order_ids_[i]);
        }
        if (selected_order_ids_.size() > 10) {
            selected_ids_text += "...";
        }
        ImGui::TextWrapped("%s", selected_ids_text.c_str());
        
        // Clear selection button
        if (ImGui::Button("Clear Selection")) {
            ClearSelection();
        }
    }
}

void OrderBlotter::RenderTable() {
    // Use filtered orders for display
    auto& display_orders = HasActiveFilters() ? filtered_orders_ : orders_;
    
    // Orders table with virtualization for large datasets
    if (ImGui::BeginTable("OrdersTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY))
    {
        // Column setup - future-proof for dynamic columns
        struct ColumnInfo {
            const char* name;
            ImGuiTableColumnFlags flags;
            float width;
            int user_id;
        };
        
        ColumnInfo columns[] = {
            {"Order ID", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 80.0f, 0},
            {"Customer", ImGuiTableColumnFlags_WidthFixed, 120.0f, 1},
            {"Product", ImGuiTableColumnFlags_WidthFixed, 100.0f, 2},
            {"Quantity", ImGuiTableColumnFlags_WidthFixed, 80.0f, 3},
            {"Price", ImGuiTableColumnFlags_WidthFixed, 80.0f, 4},
            {"Status", ImGuiTableColumnFlags_WidthStretch, 0.0f, 5}
        };
        
        // Setup columns dynamically
        for (int i = 0; i < 6; i++) {
            ImGui::TableSetupColumn(columns[i].name, columns[i].flags, columns[i].width, columns[i].user_id);
        }
        ImGui::TableHeadersRow();
        
        // Add filter row as the first row after headers
        ImGui::TableNextRow(ImGuiTableRowFlags_None);
        
        for (int column = 0; column < 6; column++) {
            ImGui::TableSetColumnIndex(column);
            ImGui::PushID(column);
            
            ColumnFilter& filter = column_filters_[column];
            
            // Create unique IDs for each filter control
            char filter_id[64];
            snprintf(filter_id, sizeof(filter_id), "##filter_%d", column);
            
            if (column == 0) { // Order ID - numeric
                ImGui::PushItemWidth(-1);
                if (ImGui::InputInt(filter_id, &filter.int_value)) {
                    filter.type = FILTER_NUMERIC_EQUALS;
                    filter.enabled = (filter.int_value != 0);
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
                    if (ImGui::MenuItem("Greater than", nullptr, filter.type == FILTER_NUMERIC_GREATER)) {
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
                
            } else if (column == 3) { // Quantity - numeric
                ImGui::PushItemWidth(-1);
                if (ImGui::InputInt(filter_id, &filter.int_value)) {
                    filter.type = FILTER_NUMERIC_EQUALS;
                    filter.enabled = (filter.int_value != 0);
                    filters_dirty_ = true;
                }
                
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                    ImGui::OpenPopup("FilterType");
                }
                
                if (ImGui::BeginPopup("FilterType")) {
                    if (ImGui::MenuItem("Equals", nullptr, filter.type == FILTER_NUMERIC_EQUALS)) {
                        filter.type = FILTER_NUMERIC_EQUALS;
                        filter.enabled = true;
                        filters_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Greater than", nullptr, filter.type == FILTER_NUMERIC_GREATER)) {
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
                
            } else if (column == 4) { // Price - float numeric
                ImGui::PushItemWidth(-1);
                if (ImGui::InputFloat(filter_id, &filter.numeric_value, 0.0f, 0.0f, "%.2f")) {
                    filter.type = FILTER_NUMERIC_EQUALS;
                    filter.enabled = (filter.numeric_value != 0.0f);
                    filters_dirty_ = true;
                }
                
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                    ImGui::OpenPopup("FilterType");
                }
                
                if (ImGui::BeginPopup("FilterType")) {
                    if (ImGui::MenuItem("Equals", nullptr, filter.type == FILTER_NUMERIC_EQUALS)) {
                        filter.type = FILTER_NUMERIC_EQUALS;
                        filter.enabled = true;
                        filters_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Greater than", nullptr, filter.type == FILTER_NUMERIC_GREATER)) {
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
                
            } else { // Text columns (Customer, Product, Status)
                ImGui::PushItemWidth(-1);
                if (ImGui::InputText(filter_id, filter.text_value, sizeof(filter.text_value), ImGuiInputTextFlags_EnterReturnsTrue)) {
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
                    if (ImGui::MenuItem("Starts with", nullptr, filter.type == FILTER_TEXT_STARTS_WITH)) {
                        filter.type = FILTER_TEXT_STARTS_WITH;
                        filter.enabled = true;
                        filters_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Ends with", nullptr, filter.type == FILTER_TEXT_ENDS_WITH)) {
                        filter.type = FILTER_TEXT_ENDS_WITH;
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
        
        // Handle sorting
        if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs())
        {
            if (sort_specs->SpecsDirty)
            {
                if (display_orders.size() > 1)
                {
                    std::sort(display_orders.begin(), display_orders.end(), [sort_specs](const Order& a, const Order& b) {
                        for (int n = 0; n < sort_specs->SpecsCount; n++)
                        {
                            const ImGuiTableColumnSortSpecs* sort_spec = &sort_specs->Specs[n];
                            int delta = 0;
                            
                            switch (sort_spec->ColumnIndex)
                            {
                            case 0: // Order ID
                                delta = (a.id < b.id) ? -1 : (a.id > b.id) ? 1 : 0;
                                break;
                            case 1: // Customer
                                delta = strcmp(a.customer, b.customer);
                                break;
                            case 2: // Product
                                delta = strcmp(a.product, b.product);
                                break;
                            case 3: // Quantity
                                delta = (a.quantity < b.quantity) ? -1 : (a.quantity > b.quantity) ? 1 : 0;
                                break;
                            case 4: // Price
                                delta = (a.price < b.price) ? -1 : (a.price > b.price) ? 1 : 0;
                                break;
                            case 5: // Status
                                delta = strcmp(a.status, b.status);
                                break;
                            }
                            
                            if (delta != 0)
                            {
                                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? (delta < 0) : (delta > 0);
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
        clipper.Begin((int)display_orders.size());
        
        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
            {
                const Order& order = display_orders[row];
                bool is_selected = IsOrderSelected(order.id);
                
                ImGui::TableNextRow();
                
                // Make the entire row selectable
                ImGui::TableSetColumnIndex(0);
                char label[32];
                snprintf(label, sizeof(label), "##row_%d", row);
                
                if (ImGui::Selectable(label, is_selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap))
                {
                    ImGuiIO& input_io = ImGui::GetIO();
                    
                    if (input_io.KeyCtrl)
                    {
                        // Ctrl+Click: Toggle selection
                        ToggleOrderSelection(order.id);
                        last_selected_row_ = row;
                    }
                    else if (input_io.KeyShift && last_selected_row_ != -1)
                    {
                        // Shift+Click: Select range
                        SelectOrderRange(last_selected_row_, row);
                    }
                    else
                    {
                        // Normal click: Select only this row
                        selected_order_ids_.clear();
                        selected_order_ids_.push_back(order.id);
                        last_selected_row_ = row;
                    }
                }
                
                // Draw the row content
                ImGui::SameLine();
                ImGui::Text("%d", order.id);
                
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", order.customer);
                
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", order.product);
                
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%d", order.quantity);
                
                ImGui::TableSetColumnIndex(4);
                ImGui::Text("$%.2f", order.price);
                
                ImGui::TableSetColumnIndex(5);
                // Color code status
                if (strcmp(order.status, "Delivered") == 0)
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", order.status);
                else if (strcmp(order.status, "Shipped") == 0)
                    ImGui::TextColored(ImVec4(0.0f, 0.7f, 1.0f, 1.0f), "%s", order.status);
                else if (strcmp(order.status, "Processing") == 0)
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", order.status);
                else if (strcmp(order.status, "Cancelled") == 0)
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", order.status);
                else
                    ImGui::Text("%s", order.status);
            }
        }
        
        ImGui::EndTable();
    }
}

// Filter management methods
void OrderBlotter::ClearAllFilters() {
    for (int i = 0; i < 6; i++) {
        column_filters_[i] = ColumnFilter{};
    }
    filters_dirty_ = true;
}

void OrderBlotter::SetColumnFilter(int column, const ColumnFilter& filter) {
    if (column >= 0 && column < 6) {
        column_filters_[column] = filter;
        filters_dirty_ = true;
    }
}

bool OrderBlotter::HasActiveFilters() const {
    for (int i = 0; i < 6; i++) {
        if (column_filters_[i].enabled) {
            return true;
        }
    }
    return false;
}

void OrderBlotter::ApplyFilters() {
    if (!filters_dirty_) return;
    
    filtered_orders_.clear();
    
    if (!HasActiveFilters()) {
        // No filters, show all orders
        filtered_orders_ = orders_;
    } else {
        // Apply filters
        for (const auto& order : orders_) {
            if (PassesFilter(order)) {
                filtered_orders_.push_back(order);
            }
        }
    }
    
    filters_dirty_ = false;
}

bool OrderBlotter::PassesFilter(const Order& order) const {
    for (int i = 0; i < 6; i++) {
        if (column_filters_[i].enabled) {
            if (!PassesColumnFilter(order, i, column_filters_[i])) {
                return false;
            }
        }
    }
    return true;
}

bool OrderBlotter::PassesColumnFilter(const Order& order, int column, const ColumnFilter& filter) const {
    switch (column) {
        case 0: // Order ID
        {
            int value = order.id;
            switch (filter.type) {
                case FILTER_NUMERIC_EQUALS:
                    return value == filter.int_value;
                case FILTER_NUMERIC_GREATER:
                    return value > filter.int_value;
                case FILTER_NUMERIC_LESS:
                    return value < filter.int_value;
                case FILTER_NUMERIC_GREATER_EQUAL:
                    return value >= filter.int_value;
                case FILTER_NUMERIC_LESS_EQUAL:
                    return value <= filter.int_value;
                case FILTER_NUMERIC_RANGE:
                    return value >= filter.int_range_min && value <= filter.int_range_max;
                default:
                    return true;
            }
        }
        
        case 1: // Customer
        {
            const char* text = order.customer;
            switch (filter.type) {
                case FILTER_TEXT_CONTAINS:
                    return strstr(text, filter.text_value) != nullptr;
                case FILTER_TEXT_EQUALS:
                    return strcmp(text, filter.text_value) == 0;
                case FILTER_TEXT_STARTS_WITH:
                    return strncmp(text, filter.text_value, strlen(filter.text_value)) == 0;
                case FILTER_TEXT_ENDS_WITH:
                {
                    size_t text_len = strlen(text);
                    size_t filter_len = strlen(filter.text_value);
                    if (filter_len > text_len) return false;
                    return strcmp(text + text_len - filter_len, filter.text_value) == 0;
                }
                default:
                    return true;
            }
        }
        
        case 2: // Product
        {
            const char* text = order.product;
            switch (filter.type) {
                case FILTER_TEXT_CONTAINS:
                    return strstr(text, filter.text_value) != nullptr;
                case FILTER_TEXT_EQUALS:
                    return strcmp(text, filter.text_value) == 0;
                case FILTER_TEXT_STARTS_WITH:
                    return strncmp(text, filter.text_value, strlen(filter.text_value)) == 0;
                case FILTER_TEXT_ENDS_WITH:
                {
                    size_t text_len = strlen(text);
                    size_t filter_len = strlen(filter.text_value);
                    if (filter_len > text_len) return false;
                    return strcmp(text + text_len - filter_len, filter.text_value) == 0;
                }
                default:
                    return true;
            }
        }
        
        case 3: // Quantity
        {
            int value = order.quantity;
            switch (filter.type) {
                case FILTER_NUMERIC_EQUALS:
                    return value == filter.int_value;
                case FILTER_NUMERIC_GREATER:
                    return value > filter.int_value;
                case FILTER_NUMERIC_LESS:
                    return value < filter.int_value;
                case FILTER_NUMERIC_GREATER_EQUAL:
                    return value >= filter.int_value;
                case FILTER_NUMERIC_LESS_EQUAL:
                    return value <= filter.int_value;
                case FILTER_NUMERIC_RANGE:
                    return value >= filter.int_range_min && value <= filter.int_range_max;
                default:
                    return true;
            }
        }
        
        case 4: // Price
        {
            float value = order.price;
            switch (filter.type) {
                case FILTER_NUMERIC_EQUALS:
                    return fabsf(value - filter.numeric_value) < 0.01f;
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
        
        case 5: // Status
        {
            const char* text = order.status;
            switch (filter.type) {
                case FILTER_TEXT_CONTAINS:
                    return strstr(text, filter.text_value) != nullptr;
                case FILTER_TEXT_EQUALS:
                    return strcmp(text, filter.text_value) == 0;
                case FILTER_TEXT_STARTS_WITH:
                    return strncmp(text, filter.text_value, strlen(filter.text_value)) == 0;
                case FILTER_TEXT_ENDS_WITH:
                {
                    size_t text_len = strlen(text);
                    size_t filter_len = strlen(filter.text_value);
                    if (filter_len > text_len) return false;
                    return strcmp(text + text_len - filter_len, filter.text_value) == 0;
                }
                default:
                    return true;
            }
        }
    }
    return true;
}

// Grouping management methods
void OrderBlotter::SetGroupByColumn(int column) {
    if (column >= 0 && column < 6) {
        group_by_column_ = column;
        groups_dirty_ = true;
    }
}

void OrderBlotter::ClearGrouping() {
    group_by_column_ = -1;
    groups_.clear();
    groups_dirty_ = true;
}

void OrderBlotter::ApplyGrouping() {
    if (!groups_dirty_ || group_by_column_ < 0) return;
    
    BuildGroups();
    groups_dirty_ = false;
}

void OrderBlotter::BuildGroups() {
    groups_.clear();
    
    auto& display_orders = HasActiveFilters() ? filtered_orders_ : orders_;
    
    // Create a map to group orders by the selected column
    std::map<std::string, std::vector<int>> group_map;
    
    for (int i = 0; i < (int)display_orders.size(); i++) {
        std::string group_key = GetGroupKey(display_orders[i], group_by_column_);
        group_map[group_key].push_back(i);
    }
    
    // Convert map to vector of GroupInfo
    groups_.reserve(group_map.size());
    for (const auto& pair : group_map) {
        GroupInfo group;
        group.group_key = pair.first;
        group.order_indices = pair.second;
        group.order_count = (int)pair.second.size();
        
        CalculateGroupAggregates(group);
        groups_.push_back(group);
    }
    
    // Sort groups by group key
    std::sort(groups_.begin(), groups_.end(), [](const GroupInfo& a, const GroupInfo& b) {
        return a.group_key < b.group_key;
    });
}

std::string OrderBlotter::GetGroupKey(const Order& order, int column) const {
    switch (column) {
        case 0: return std::to_string(order.id);
        case 1: return std::string(order.customer);
        case 2: return std::string(order.product);
        case 3: return std::to_string(order.quantity);
        case 4: return std::to_string((int)(order.price * 100)); // Group by price rounded to cents
        case 5: return std::string(order.status);
        default: return "";
    }
}

void OrderBlotter::CalculateGroupAggregates(GroupInfo& group) const {
    auto& display_orders = HasActiveFilters() ? filtered_orders_ : orders_;
    
    group.total_quantity = 0;
    group.total_value = 0.0f;
    float total_price = 0.0f;
    
    for (int index : group.order_indices) {
        const Order& order = display_orders[index];
        group.total_quantity += order.quantity;
        group.total_value += order.quantity * order.price;
        total_price += order.price;
    }
    
    group.average_price = group.order_count > 0 ? total_price / group.order_count : 0.0f;
}

void OrderBlotter::RenderGroupedTable() {
    if (ImGui::BeginTable("GroupedOrdersTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
    {
        // Column setup - same as regular table
        struct ColumnInfo {
            const char* name;
            ImGuiTableColumnFlags flags;
            float width;
            int user_id;
        };
        
        ColumnInfo columns[] = {
            {"Order ID", ImGuiTableColumnFlags_WidthFixed, 80.0f, 0},
            {"Customer", ImGuiTableColumnFlags_WidthFixed, 120.0f, 1},
            {"Product", ImGuiTableColumnFlags_WidthFixed, 100.0f, 2},
            {"Quantity", ImGuiTableColumnFlags_WidthFixed, 80.0f, 3},
            {"Price", ImGuiTableColumnFlags_WidthFixed, 80.0f, 4},
            {"Status", ImGuiTableColumnFlags_WidthStretch, 0.0f, 5}
        };
        
        // Setup columns dynamically
        for (int i = 0; i < 6; i++) {
            ImGui::TableSetupColumn(columns[i].name, columns[i].flags, columns[i].width, columns[i].user_id);
        }
        ImGui::TableHeadersRow();
        
        // Filter row (same as regular table)
        ImGui::TableNextRow(ImGuiTableRowFlags_None);
        for (int column = 0; column < 6; column++) {
            ImGui::TableSetColumnIndex(column);
            ImGui::PushID(column);
            
            ColumnFilter& filter = column_filters_[column];
            char filter_id[64];
            snprintf(filter_id, sizeof(filter_id), "##filter_%d", column);
            
            if (column == 0) { // Order ID - numeric
                ImGui::PushItemWidth(-1);
                if (ImGui::InputInt(filter_id, &filter.int_value)) {
                    filter.type = FILTER_NUMERIC_EQUALS;
                    filter.enabled = (filter.int_value != 0);
                    filters_dirty_ = true;
                    groups_dirty_ = true;
                }
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                    ImGui::OpenPopup("FilterType");
                }
                if (ImGui::BeginPopup("FilterType")) {
                    if (ImGui::MenuItem("Equals", nullptr, filter.type == FILTER_NUMERIC_EQUALS)) {
                        filter.type = FILTER_NUMERIC_EQUALS; filter.enabled = true; filters_dirty_ = true; groups_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Greater than", nullptr, filter.type == FILTER_NUMERIC_GREATER)) {
                        filter.type = FILTER_NUMERIC_GREATER; filter.enabled = true; filters_dirty_ = true; groups_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Less than", nullptr, filter.type == FILTER_NUMERIC_LESS)) {
                        filter.type = FILTER_NUMERIC_LESS; filter.enabled = true; filters_dirty_ = true; groups_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Clear", nullptr, !filter.enabled)) {
                        filter.enabled = false; filter.type = FILTER_NONE; filters_dirty_ = true; groups_dirty_ = true;
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopItemWidth();
            } else if (column == 3) { // Quantity - numeric
                ImGui::PushItemWidth(-1);
                if (ImGui::InputInt(filter_id, &filter.int_value)) {
                    filter.type = FILTER_NUMERIC_EQUALS; filter.enabled = (filter.int_value != 0); filters_dirty_ = true; groups_dirty_ = true;
                }
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                    ImGui::OpenPopup("FilterType");
                }
                if (ImGui::BeginPopup("FilterType")) {
                    if (ImGui::MenuItem("Equals", nullptr, filter.type == FILTER_NUMERIC_EQUALS)) {
                        filter.type = FILTER_NUMERIC_EQUALS; filter.enabled = true; filters_dirty_ = true; groups_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Greater than", nullptr, filter.type == FILTER_NUMERIC_GREATER)) {
                        filter.type = FILTER_NUMERIC_GREATER; filter.enabled = true; filters_dirty_ = true; groups_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Less than", nullptr, filter.type == FILTER_NUMERIC_LESS)) {
                        filter.type = FILTER_NUMERIC_LESS; filter.enabled = true; filters_dirty_ = true; groups_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Clear", nullptr, !filter.enabled)) {
                        filter.enabled = false; filter.type = FILTER_NONE; filters_dirty_ = true; groups_dirty_ = true;
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopItemWidth();
            } else if (column == 4) { // Price - float numeric
                ImGui::PushItemWidth(-1);
                if (ImGui::InputFloat(filter_id, &filter.numeric_value, 0.0f, 0.0f, "%.2f")) {
                    filter.type = FILTER_NUMERIC_EQUALS; filter.enabled = (filter.numeric_value != 0.0f); filters_dirty_ = true; groups_dirty_ = true;
                }
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                    ImGui::OpenPopup("FilterType");
                }
                if (ImGui::BeginPopup("FilterType")) {
                    if (ImGui::MenuItem("Equals", nullptr, filter.type == FILTER_NUMERIC_EQUALS)) {
                        filter.type = FILTER_NUMERIC_EQUALS; filter.enabled = true; filters_dirty_ = true; groups_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Greater than", nullptr, filter.type == FILTER_NUMERIC_GREATER)) {
                        filter.type = FILTER_NUMERIC_GREATER; filter.enabled = true; filters_dirty_ = true; groups_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Less than", nullptr, filter.type == FILTER_NUMERIC_LESS)) {
                        filter.type = FILTER_NUMERIC_LESS; filter.enabled = true; filters_dirty_ = true; groups_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Clear", nullptr, !filter.enabled)) {
                        filter.enabled = false; filter.type = FILTER_NONE; filters_dirty_ = true; groups_dirty_ = true;
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopItemWidth();
            } else { // Text columns
                ImGui::PushItemWidth(-1);
                if (ImGui::InputText(filter_id, filter.text_value, sizeof(filter.text_value), ImGuiInputTextFlags_EnterReturnsTrue)) {
                    filter.type = FILTER_TEXT_CONTAINS; filter.enabled = (strlen(filter.text_value) > 0); filters_dirty_ = true; groups_dirty_ = true;
                }
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                    ImGui::OpenPopup("FilterType");
                }
                if (ImGui::BeginPopup("FilterType")) {
                    if (ImGui::MenuItem("Contains", nullptr, filter.type == FILTER_TEXT_CONTAINS)) {
                        filter.type = FILTER_TEXT_CONTAINS; filter.enabled = true; filters_dirty_ = true; groups_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Equals", nullptr, filter.type == FILTER_TEXT_EQUALS)) {
                        filter.type = FILTER_TEXT_EQUALS; filter.enabled = true; filters_dirty_ = true; groups_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Starts with", nullptr, filter.type == FILTER_TEXT_STARTS_WITH)) {
                        filter.type = FILTER_TEXT_STARTS_WITH; filter.enabled = true; filters_dirty_ = true; groups_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Ends with", nullptr, filter.type == FILTER_TEXT_ENDS_WITH)) {
                        filter.type = FILTER_TEXT_ENDS_WITH; filter.enabled = true; filters_dirty_ = true; groups_dirty_ = true;
                    }
                    if (ImGui::MenuItem("Clear", nullptr, !filter.enabled)) {
                        filter.enabled = false; filter.type = FILTER_NONE; filters_dirty_ = true; groups_dirty_ = true;
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopItemWidth();
            }
            ImGui::PopID();
        }
        
        auto& display_orders = HasActiveFilters() ? filtered_orders_ : orders_;
        
        // Render grouped data
        for (int group_index = 0; group_index < (int)groups_.size(); group_index++) {
            GroupInfo& group = groups_[group_index];
            
            // Render group header
            RenderGroupHeader(group, group_index);
            
            // Render group rows if not collapsed
            if (!group.is_collapsed) {
                for (int order_index : group.order_indices) {
                    RenderGroupRow(display_orders[order_index], order_index);
                }
            }
        }
        
        ImGui::EndTable();
    }
}

void OrderBlotter::RenderGroupHeader(const GroupInfo& group, int group_index) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    
    // Create collapsible header that spans all columns
    char group_header[256];
    snprintf(group_header, sizeof(group_header), "%s (%d orders)##group_%d", 
             group.group_key.c_str(), group.order_count, group_index);
    
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.4f, 0.6f, 0.8f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.5f, 0.7f, 0.9f, 0.8f));
    
    bool header_open = ImGui::CollapsingHeader(group_header, ImGuiTreeNodeFlags_DefaultOpen);
    
    // Toggle collapse state when header is clicked
    GroupInfo& mutable_group = const_cast<GroupInfo&>(group);
    mutable_group.is_collapsed = !header_open;
    
    ImGui::PopStyleColor(2);
    
    // Show aggregate information in other columns
    ImGui::TableSetColumnIndex(1);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Total Qty: %d", group.total_quantity);
    
    ImGui::TableSetColumnIndex(2);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Avg Price: $%.2f", group.average_price);
    
    ImGui::TableSetColumnIndex(3);
    // Empty
    
    ImGui::TableSetColumnIndex(4);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Total Value: $%.2f", group.total_value);
    
    ImGui::TableSetColumnIndex(5);
    // Empty
}

void OrderBlotter::RenderGroupRow(const Order& order, int row_index) {
    bool is_selected = IsOrderSelected(order.id);
    
    ImGui::TableNextRow();
    
    // Make the entire row selectable
    ImGui::TableSetColumnIndex(0);
    char label[32];
    snprintf(label, sizeof(label), "##row_%d", row_index);
    
    if (ImGui::Selectable(label, is_selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap))
    {
        ImGuiIO& input_io = ImGui::GetIO();
        
        if (input_io.KeyCtrl)
        {
            // Ctrl+Click: Toggle selection
            ToggleOrderSelection(order.id);
            last_selected_row_ = row_index;
        }
        else if (input_io.KeyShift && last_selected_row_ != -1)
        {
            // Shift+Click: Select range
            SelectOrderRange(last_selected_row_, row_index);
        }
        else
        {
            // Normal click: Select only this row
            selected_order_ids_.clear();
            selected_order_ids_.push_back(order.id);
            last_selected_row_ = row_index;
        }
    }
    
    // Indent the first column to show it's part of a group
    ImGui::SameLine();
    ImGui::Indent(20.0f);
    ImGui::Text("%d", order.id);
    ImGui::Unindent(20.0f);
    
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%s", order.customer);
    
    ImGui::TableSetColumnIndex(2);
    ImGui::Text("%s", order.product);
    
    ImGui::TableSetColumnIndex(3);
    ImGui::Text("%d", order.quantity);
    
    ImGui::TableSetColumnIndex(4);
    ImGui::Text("$%.2f", order.price);
    
    ImGui::TableSetColumnIndex(5);
    // Color code status
    if (strcmp(order.status, "Delivered") == 0)
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", order.status);
    else if (strcmp(order.status, "Shipped") == 0)
        ImGui::TextColored(ImVec4(0.0f, 0.7f, 1.0f, 1.0f), "%s", order.status);
    else if (strcmp(order.status, "Processing") == 0)
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", order.status);
    else if (strcmp(order.status, "Cancelled") == 0)
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", order.status);
    else
        ImGui::Text("%s", order.status);
}

