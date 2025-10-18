# Bug Fix: Side Encoding Mismatch Between Navigator and MarketDataTable

## Issue Description
The Navigator and MarketDataTable were showing different counts for Buy/Sell orders:
- **MarketDataTable**: Showed 5111 Buy, 4889 Sell
- **Navigator**: Showed 4889 Buy, 0 Sell

The numbers were swapped and one showed 0, indicating they were not interpreting the side field correctly.

## Root Cause
The components were using **different encoding schemes** for the `side` field:

### Incorrect (MarketDataTable - OLD):
```cpp
case 0: return "Buy";   // WRONG!
case 1: return "Sell";  // WRONG!
```

### Correct (API Specification from md_api.h):
```c
uint8_t* side;   // 0=unk, 1=bid, 2=ask, 3=trade
```

So the correct encoding is:
- `0` = Unknown
- `1` = Bid (Buy)
- `2` = Ask (Sell)
- `3` = Trade

## The Fix

### Fixed Files:

1. **`imgui_opengl_glad/ui/MarketDataTable.cpp`**
   - Updated `GetSideString()` to use correct encoding: `1=Buy, 2=Sell, 3=Trade`
   - Updated `GetSideColor()` to match:
     - `1` (Buy/Bid) = Green
     - `2` (Sell/Ask) = Red
     - `3` (Trade) = Blue
     - `0` (Unknown) = Gray

2. **`imgui_opengl_glad/tests/guitests/app_gui_test.cpp`**
   - Fixed test data generation from `i % 2` (gives 0,1) to `(i % 2 == 0) ? 1 : 2` (gives 1,2)

### Navigator (Already Correct):
The Navigator was already using the correct encoding:
```cpp
if (side == 1) {
    stats_.buy_count++;
} else if (side == 2) {
    stats_.sell_count++;
}
```

## Verification
✅ All 11 tests pass after the fix
✅ Both components now use the same API specification
✅ Side counts should now match between Navigator and MarketDataTable

## Impact
- **Breaking Change**: If any saved data or external code expected the old `0=Buy, 1=Sell` encoding, it will need to be updated.
- **Data Consistency**: Navigator and MarketDataTable now correctly share the same data source and interpret it identically.
- **API Compliance**: Code now matches the official `md_api.h` specification.

## Testing
Run the GUI application and verify:
1. Navigator "By Side" counts match MarketDataTable filter counts
2. Buy orders show in green (both components)
3. Sell orders show in red (both components)
4. The totals add up correctly

```bash
# Run automated tests
cd imgui_opengl_glad\tests\guitests\build\Debug
.\simple_gui_test.exe --headless

# Expected: Total Tests: 11, Passed: 11, Failed: 0
```
