# Feature: Interactive Navigator Quick Filters

## Overview
The Navigator's Quick Filters are now interactive! Clicking on a filter will automatically apply it to the Market Data Table, providing instant data filtering.

## Implementation

### Changes Made

1. **`Navigator.h`**
   - Added `MarketDataTable*` forward declaration
   - Updated `Render()` to accept optional `MarketDataTable* table` parameter
   - Updated `RenderQuickFiltersTree()` to accept `MarketDataTable* table` parameter

2. **`Navigator.cpp`**
   - Added `#include "MarketDataTable.h"` and `#include <cstring>`
   - Converted static tree nodes to `ImGui::Selectable()` for click interaction
   - Added filter application logic for each Quick Filter
   - Added tooltips to explain what each filter does

3. **`IMGuiComponents.cpp`**
   - Updated Navigator render call to pass `market_data_table_.get()` pointer
   - Enables bi-directional communication between Navigator and MarketDataTable

## Available Quick Filters

### 1. Show All
**Action**: Clears all active filters  
**Tooltip**: "Clear all filters and show all rows"  
**Use Case**: Return to viewing all market data after applying filters

### 2. Show Buy Only
**Action**: Applies text filter on SIDE column (column 4) for "Buy"  
**Tooltip**: "Show only Buy orders (side = 1)"  
**Filter Details**:
- Type: `FILTER_TEXT_EQUALS`
- Column: 4 (SIDE)
- Value: "Buy"

### 3. Show Sell Only
**Action**: Applies text filter on SIDE column (column 4) for "Sell"  
**Tooltip**: "Show only Sell orders (side = 2)"  
**Filter Details**:
- Type: `FILTER_TEXT_EQUALS`
- Column: 4 (SIDE)
- Value: "Sell"

### 4. Show Large Orders (Qty > 1000)
**Action**: Applies numeric filter on QTY column (column 3)  
**Tooltip**: "Show only orders with quantity > 1000"  
**Filter Details**:
- Type: `FILTER_NUMERIC_GREATER`
- Column: 3 (QTY)
- Value: 1000

### 5. Show Recent Updates
**Action**: Currently clears all filters (placeholder)  
**Tooltip**: "Show only recently updated rows"  
**Status**: ⚠️ TODO - Requires special filter type for dirty rows in MarketDataTable

## User Experience

### Before
- Quick Filters were displayed as static tree nodes
- Clicking had no effect
- Filters were decorative only

### After
- Quick Filters are now clickable selectable items
- Clicking a filter instantly applies it to the Market Data Table
- Visual feedback on hover with tooltips
- Filters take effect immediately
- Only one quick filter active at a time (each clears previous filters)

## Technical Details

### Filter Application Flow
```
1. User clicks Quick Filter in Navigator
2. Navigator calls table->ClearAllFilters()
3. Navigator creates appropriate ColumnFilter
4. Navigator calls table->SetColumnFilter(column, filter)
5. MarketDataTable marks filters as dirty
6. On next render, MarketDataTable rebuilds filtered indices
7. Table displays only matching rows
```

### Column Indices
- 0: ID (row index)
- 1: Timestamp
- 2: Price
- 3: Quantity
- 4: Side

### Safety
- All filter operations check if `table` pointer is valid
- Default parameter `nullptr` maintains backward compatibility
- Tests work without MarketDataTable (pass nullptr)

## Future Enhancements

### Planned Improvements
1. **Show Recent Updates Filter**
   - Add special filter type for dirty rows
   - Implement in MarketDataTable filtering logic
   - Show only rows with `ctx.dirty[i] != 0`

2. **Visual Feedback**
   - Highlight active filter in Navigator
   - Show filter count in Quick Filters title
   - Display current filter in MarketDataTable header

3. **Additional Quick Filters**
   - Show Unknown/Trade side types (when present)
   - Price range filters (Low/Mid/High)
   - Time-based filters (Last minute, Last hour)
   - Custom saved filters

4. **Filter Persistence**
   - Remember last active filter
   - Save/load filter presets
   - Recent filters history

## Testing

### Manual Testing
1. Run the application
2. Open Navigator panel
3. Expand "Quick Filters"
4. Click "Show Buy Only" → Table should show only Buy orders
5. Click "Show Sell Only" → Table should show only Sell orders
6. Click "Show Large Orders" → Table should show only Qty > 1000
7. Click "Show All" → Table should show all rows again

### Automated Testing
All existing tests pass (11/11) because:
- Tests use default `nullptr` parameter
- Navigator gracefully handles null table pointer
- No filter operations attempted when table is null

```bash
cd imgui_opengl_glad\tests\guitests\build\Debug
.\simple_gui_test.exe --headless
# Expected: Total Tests: 11, Passed: 11, Failed: 0
```

## Benefits

✅ **Instant Filtering**: One-click access to common data views  
✅ **User-Friendly**: No need to manually configure column filters  
✅ **Discoverable**: Filters are visible and self-explanatory  
✅ **Consistent**: Uses existing MarketDataTable filter system  
✅ **Safe**: Null-pointer checks and backward compatible  
✅ **Extensible**: Easy to add more quick filters

## Related Files
- `imgui_opengl_glad/ui/Navigator.h`
- `imgui_opengl_glad/ui/Navigator.cpp`
- `imgui_opengl_glad/ui/IMGuiComponents.cpp`
- `imgui_opengl_glad/ui/MarketDataTable.h`
- `imgui_opengl_glad/ui/MarketDataTable.cpp`
