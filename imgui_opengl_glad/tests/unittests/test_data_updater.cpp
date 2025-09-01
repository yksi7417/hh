#include <gtest/gtest.h>
#include "../../data_updater.h"
#include <memory>
#include <vector>
#include <chrono>
#include <cstring>

/**
 * @brief Test fixture for DataUpdater tests using Google Test
 */
class DataUpdaterTest : public ::testing::Test {
protected:
    EmspConfig config;
    HostContext ctx;
    HostMDSlot slot;
    std::vector<int64_t> ts_ns;
    std::vector<int64_t> px_n;
    std::vector<int64_t> qty;
    std::vector<uint8_t> side;

    void SetUp() override {
        config.num_rows = 10;  // Small number for testing
        config.writers = 1;
        config.ups = 100;
        
        // Initialize test data
        ts_ns.resize(config.num_rows, 0);
        px_n.resize(config.num_rows, 0);
        qty.resize(config.num_rows, 0);
        side.resize(config.num_rows, 0);
        
        initializeContext();
    }

    void TearDown() override {
        // Cleanup is automatic due to RAII
    }

private:
    void initializeContext() {
        // Initialize HostContext
        ctx.num_rows = config.num_rows;
        ctx.seq = std::make_unique<std::atomic<uint32_t>[]>(config.num_rows);
        for (uint32_t i = 0; i < config.num_rows; ++i) {
            ctx.seq[i].store(0, std::memory_order_relaxed);
        }
        ctx.dirty.assign(config.num_rows, 0);
        ctx.last.resize(config.num_rows);
        ctx.q.init(1u << 10);  // Smaller queue for testing

        // Initialize HostMDSlot
        slot = HostMDSlot{};
        slot.num_rows = config.num_rows;
        slot.ts_ns = ts_ns.data();
        slot.px_n = px_n.data();
        slot.qty = qty.data();
        slot.side = side.data();
        slot.user = &ctx;
        slot.begin_row_write = &host_begin_row_write;
        slot.end_row_write = &host_end_row_write;
        slot.notify_row_dirty = &host_notify_row_dirty;
    }
};

/**
 * @brief Test that function processes queued updates correctly
 */
TEST_F(DataUpdaterTest, ProcessesQueuedUpdates) {
    // Add some updates to the queue
    ctx.q.push(1);
    ctx.q.push(3);
    ctx.q.push(5);
    
    // Call the function with a time that won't trigger printing
    uint64_t current_time = 100;
    uint64_t next_paint = 200;
    
    update_latest_data_from_context(ctx, config, current_time, next_paint, slot);
    
    // Verify that dirty flags were set
    EXPECT_EQ(ctx.dirty[1], 1);
    EXPECT_EQ(ctx.dirty[3], 1);
    EXPECT_EQ(ctx.dirty[5], 1);
    EXPECT_EQ(ctx.dirty[0], 0);  // Should not be dirty
    EXPECT_EQ(ctx.dirty[2], 0);  // Should not be dirty
}

/**
 * @brief Test that function ignores invalid row IDs
 */
TEST_F(DataUpdaterTest, IgnoresInvalidRowIds) {
    // Add valid and invalid updates to the queue
    ctx.q.push(1);                    // valid
    ctx.q.push(config.num_rows + 5);  // invalid - out of bounds
    ctx.q.push(3);                    // valid
    
    uint64_t current_time = 100;
    uint64_t next_paint = 200;
    
    update_latest_data_from_context(ctx, config, current_time, next_paint, slot);
    
    // Verify that only valid rows were marked dirty
    EXPECT_EQ(ctx.dirty[1], 1);
    EXPECT_EQ(ctx.dirty[3], 1);
    
    // Verify no out-of-bounds access occurred (test would crash if it did)
}

/**
 * @brief Test that function processes snapshots when time >= next_paint
 */
TEST_F(DataUpdaterTest, ProcessesSnapshotsWhenTimeReached) {
    // Set up some test data
    ts_ns[2] = 123456789;
    px_n[2] = 100500;  // Price in nano units
    qty[2] = 1000;
    side[2] = 1;  // Buy side
    
    // Mark row as dirty
    ctx.dirty[2] = 1;
    
    // Set up sequence numbers to allow successful snapshot
    ctx.seq[2].store(0, std::memory_order_relaxed);  // Even number = not being written
    
    uint64_t current_time = 200;
    uint64_t next_paint = 200;  // Time matches, should process
    
    update_latest_data_from_context(ctx, config, current_time, next_paint, slot);
    
    // Verify that dirty flag was cleared and last snapshot was updated
    EXPECT_EQ(ctx.dirty[2], 0);
    EXPECT_EQ(ctx.last[2].ts, 123456789);
    EXPECT_EQ(ctx.last[2].px, 100500);
    EXPECT_EQ(ctx.last[2].qty, 1000);
    EXPECT_EQ(ctx.last[2].side, 1);
}

/**
 * @brief Test that function skips processing when time < next_paint
 */
TEST_F(DataUpdaterTest, SkipsProcessingWhenTimeNotReached) {
    // Mark a row as dirty
    ctx.dirty[1] = 1;
    
    uint64_t current_time = 100;
    uint64_t next_paint = 200;  // Time hasn't been reached yet
    
    update_latest_data_from_context(ctx, config, current_time, next_paint, slot);
    
    // Verify that dirty flag remains set (wasn't processed)
    EXPECT_EQ(ctx.dirty[1], 1);
}

/**
 * @brief Test that function handles concurrent writes gracefully
 */
TEST_F(DataUpdaterTest, HandlesConcurrentWrites) {
    // Set up data
    ts_ns[4] = 987654321;
    px_n[4] = 200750;
    qty[4] = 500;
    side[4] = 0;  // Sell side
    
    // Mark row as dirty
    ctx.dirty[4] = 1;
    
    // Simulate a writer in progress (odd sequence number)
    ctx.seq[4].store(1, std::memory_order_relaxed);
    
    uint64_t current_time = 300;
    uint64_t next_paint = 300;
    
    update_latest_data_from_context(ctx, config, current_time, next_paint, slot);
    
    // Verify that dirty flag was cleared even though snapshot failed
    EXPECT_EQ(ctx.dirty[4], 0);
    
    // Verify that last snapshot wasn't updated (should still be default)
    EXPECT_EQ(ctx.last[4].ts, 0);
    EXPECT_EQ(ctx.last[4].px, 0);
    EXPECT_EQ(ctx.last[4].qty, 0);
    EXPECT_EQ(ctx.last[4].side, 0);
}

/**
 * @brief Test the complete workflow
 */
TEST_F(DataUpdaterTest, CompleteWorkflow) {
    // Set up test data for multiple rows
    for (uint32_t i = 0; i < 3; ++i) {
        ts_ns[i] = 1000000 + i * 1000;
        px_n[i] = 100000 + i * 50;
        qty[i] = 100 + i * 10;
        side[i] = i % 2;
    }
    
    // Simulate updates arriving
    ctx.q.push(0);
    ctx.q.push(1);
    ctx.q.push(2);
    
    // First call - should process queue but not snapshots
    uint64_t current_time = 100;
    uint64_t next_paint = 200;
    update_latest_data_from_context(ctx, config, current_time, next_paint, slot);
    
    // Verify dirty flags are set
    EXPECT_EQ(ctx.dirty[0], 1);
    EXPECT_EQ(ctx.dirty[1], 1);
    EXPECT_EQ(ctx.dirty[2], 1);
    
    // Second call - should process snapshots
    current_time = 200;
    update_latest_data_from_context(ctx, config, current_time, next_paint, slot);
    
    // Verify all dirty flags are cleared and snapshots are taken
    EXPECT_EQ(ctx.dirty[0], 0);
    EXPECT_EQ(ctx.dirty[1], 0);
    EXPECT_EQ(ctx.dirty[2], 0);
    
    // Verify snapshots match expected data
    EXPECT_EQ(ctx.last[0].ts, 1000000);
    EXPECT_EQ(ctx.last[0].px, 100000);
    EXPECT_EQ(ctx.last[0].qty, 100);
    EXPECT_EQ(ctx.last[0].side, 0);
    
    EXPECT_EQ(ctx.last[1].ts, 1001000);
    EXPECT_EQ(ctx.last[1].px, 100050);
    EXPECT_EQ(ctx.last[1].qty, 110);
    EXPECT_EQ(ctx.last[1].side, 1);
}

/**
 * @brief Test edge case with empty queue
 */
TEST_F(DataUpdaterTest, HandlesEmptyQueue) {
    // Don't add anything to queue
    
    uint64_t current_time = 200;
    uint64_t next_paint = 200;
    
    // Should not crash or cause issues
    EXPECT_NO_THROW(update_latest_data_from_context(ctx, config, current_time, next_paint, slot));
    
    // All dirty flags should remain 0
    for (uint32_t i = 0; i < config.num_rows; ++i) {
        EXPECT_EQ(ctx.dirty[i], 0);
    }
}

/**
 * @brief Test performance characteristics (basic timing test)
 */
TEST_F(DataUpdaterTest, PerformanceCharacteristics) {
    // Set up a larger dataset for performance testing
    EmspConfig large_config;
    large_config.num_rows = 1000;
    
    // Add many updates to queue
    for (uint32_t i = 0; i < 100; ++i) {
        ctx.q.push(i % config.num_rows);
    }
    
    uint64_t current_time = 100;
    uint64_t next_paint = 200;
    
    // Function should complete quickly
    auto start = std::chrono::high_resolution_clock::now();
    update_latest_data_from_context(ctx, config, current_time, next_paint, slot);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Should complete in reasonable time (less than 1ms for this small dataset)
    EXPECT_LT(duration.count(), 1000);
}
