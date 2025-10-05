// Tests for the actual application GUI components
// This tests the real market data table (without GLFW/ImGuiComponents wrapper)

#include "imgui.h"
#include "imgui_test_engine/imgui_te_engine.h"
#include "imgui_test_engine/imgui_te_context.h"

// Include your application headers
#include "../../ui/MarketDataTable.h"
#include "../../core/main_context.h"

// Forward declare the registration function
void RegisterAppGuiTests(ImGuiTestEngine* engine);

// Test variables to hold application state
struct AppGuiTestVars {
    // Mock data for testing
    HostContext ctx;
    HostMDSlot slot;
    std::vector<int64_t> ts_ns;
    std::vector<int64_t> px_n;
    std::vector<int64_t> qty;
    std::vector<uint8_t> side;
    
    std::unique_ptr<MarketDataTable> market_data_table;
    bool initialized = false;
    
    void Initialize(uint32_t num_rows = 100) {
        if (initialized) return;
        
        // Setup mock data
        ts_ns.resize(num_rows, 0);
        px_n.resize(num_rows, 0);
        qty.resize(num_rows, 0);
        side.resize(num_rows, 0);
        
        // Initialize context
        ctx.num_rows = num_rows;
        ctx.seq = std::make_unique<std::atomic<uint32_t>[]>(num_rows);
        for (uint32_t i = 0; i < num_rows; ++i) {
            ctx.seq[i].store(0, std::memory_order_relaxed);
        }
        ctx.dirty.assign(num_rows, 0);
        ctx.last.resize(num_rows);
        ctx.q.init(1u << 18);
        
        // Initialize slot
        slot.num_rows = num_rows;
        slot.ts_ns = ts_ns.data();
        slot.px_n = px_n.data();
        slot.qty = qty.data();
        slot.side = side.data();
        slot.user = &ctx;
        
        initialized = true;
        
        // Initialize MarketDataTable directly (Test Engine provides ImGui context)
        market_data_table = std::make_unique<MarketDataTable>();
        market_data_table->Initialize(num_rows);
    }
    
    void Cleanup() {
        if (initialized) {
            market_data_table.reset();
            initialized = false;
        }
    }
};

void RegisterAppGuiTests(ImGuiTestEngine* engine) {
    // Test 1: Market Data Table Window Exists
    ImGuiTest* t = IM_REGISTER_TEST(engine, "app_gui", "market_data_table_window");
    t->SetVarsDataType<AppGuiTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        AppGuiTestVars& vars = ctx->GetVars<AppGuiTestVars>();
        vars.Initialize();
        
        // Render the Market Data Table directly
        // Test Engine handles ImGui::NewFrame()/Render() for us
        if (vars.market_data_table) {
            uint64_t t = now_ms();
            static uint64_t next_paint_ms = 0;
            bool should_refresh = (t >= next_paint_ms);
            
            vars.market_data_table->UpdateFromContext(vars.ctx, vars.slot, should_refresh);
            vars.market_data_table->Render(vars.ctx, vars.slot);
            
            if (should_refresh) {
                next_paint_ms = t + 250;
            }
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        // Check if the Market Data window exists (window name is "MarketData")
        ctx->SetRef("MarketData");
        IM_CHECK(ctx->GetWindowByRef("") != nullptr);
    };
    
    // Test 2: Verify Market Data Table Has Content
    t = IM_REGISTER_TEST(engine, "app_gui", "market_data_table_has_rows");
    t->SetVarsDataType<AppGuiTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        AppGuiTestVars& vars = ctx->GetVars<AppGuiTestVars>();
        vars.Initialize(10);  // Create with 10 rows
        
        // Add some test data
        for (int i = 0; i < 10; i++) {
            vars.ts_ns[i] = 1000000 * (i + 1);
            vars.px_n[i] = 10000 + i * 100;
            vars.qty[i] = (i + 1) * 10;
            vars.side[i] = i % 2;  // Alternate buy/sell
            vars.ctx.seq[i].store(1, std::memory_order_relaxed);
        }
        
        if (vars.market_data_table) {
            uint64_t t = now_ms();
            static uint64_t next_paint_ms = 0;
            bool should_refresh = (t >= next_paint_ms);
            
            vars.market_data_table->UpdateFromContext(vars.ctx, vars.slot, should_refresh);
            vars.market_data_table->Render(vars.ctx, vars.slot);
            
            if (should_refresh) {
                next_paint_ms = t + 250;
            }
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        AppGuiTestVars& vars = ctx->GetVars<AppGuiTestVars>();
        
        ctx->SetRef("MarketData");
        IM_CHECK(ctx->GetWindowByRef("") != nullptr);
        
        // The table should exist (we can add more specific checks here)
        // For example, checking specific table cells or data values
    };
    
    // Test 3: Data Update Test
    t = IM_REGISTER_TEST(engine, "app_gui", "data_updates");
    t->SetVarsDataType<AppGuiTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        AppGuiTestVars& vars = ctx->GetVars<AppGuiTestVars>();
        vars.Initialize(5);
        
        // Simulate data updates over time
        static int update_count = 0;
        update_count++;
        
        for (int i = 0; i < 5; i++) {
            vars.ts_ns[i] = 1000000 * update_count;
            vars.px_n[i] = 10000 + update_count * 10;
            vars.qty[i] = update_count;
            vars.ctx.seq[i].store(update_count, std::memory_order_relaxed);
        }
        
        if (vars.market_data_table) {
            uint64_t t = now_ms();
            static uint64_t next_paint_ms = 0;
            bool should_refresh = (t >= next_paint_ms);
            
            vars.market_data_table->UpdateFromContext(vars.ctx, vars.slot, should_refresh);
            vars.market_data_table->Render(vars.ctx, vars.slot);
            
            if (should_refresh) {
                next_paint_ms = t + 250;
            }
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        ctx->SetRef("MarketData");
        
        // Wait a few frames for updates to render
        ctx->Sleep(0.1f);  // Sleep 100ms instead of Yield
        
        // Verify window is still present after updates
        IM_CHECK(ctx->GetWindowByRef("") != nullptr);
    };
}
