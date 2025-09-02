#include <gtest/gtest.h>
#include "../../core/mpsc.h"
#include "../../core/data_updater.h"
#include <memory>
#include <vector>
#include <cstring>

/**
 * @brief Simple test to verify the basic setup works
 */
TEST(SimpleTest, BasicSetup) {
    EXPECT_EQ(1 + 1, 2);
}

/**
 * @brief Test basic math operations
 */
TEST(SimpleTest, BasicMath) {
    EXPECT_EQ(2 * 2, 4);
    EXPECT_GT(5, 3);
    EXPECT_LT(1, 10);
}

/**
 * @brief Test EmspConfig structure
 */
TEST(EmspConfigTest, DefaultValues) {
    EmspConfig config;
    EXPECT_GT(config.num_rows, 0);
    EXPECT_GT(config.writers, 0);
    EXPECT_GT(config.ups, 0);
}

/**
 * @brief Test MPSCQueue basic functionality
 */
TEST(MPSCQueueTest, BasicOperations) {
    MPSCQueue queue;
    queue.init(16);
    
    // Test push and pop
    EXPECT_TRUE(queue.push(42));
    
    uint32_t value;
    EXPECT_TRUE(queue.pop(value));
    EXPECT_EQ(value, 42);
    
    // Test empty queue
    EXPECT_FALSE(queue.pop(value));
}

/**
 * @brief Test MPSCQueue multiple elements
 */
TEST(MPSCQueueTest, MultipleElements) {
    MPSCQueue queue;
    queue.init(8);
    
    // Push multiple values
    for (uint32_t i = 0; i < 5; ++i) {
        EXPECT_TRUE(queue.push(i * 10));
    }
    
    // Pop and verify values
    uint32_t value;
    for (uint32_t i = 0; i < 5; ++i) {
        EXPECT_TRUE(queue.pop(value));
        EXPECT_EQ(value, i * 10);
    }
    
    // Queue should be empty now
    EXPECT_FALSE(queue.pop(value));
}

/**
 * @brief Simple test for update_latest_data_from_context with minimal setup
 */
TEST(BasicDataUpdaterTest, MinimalTest) {
    EmspConfig config;
    config.num_rows = 10;  // Small number for testing
    
    HostContext ctx;
    ctx.num_rows = config.num_rows;
    ctx.seq = std::make_unique<std::atomic<uint32_t>[]>(config.num_rows);
    for (uint32_t i = 0; i < config.num_rows; ++i) {
        ctx.seq[i].store(0, std::memory_order_relaxed);
    }
    ctx.dirty.assign(config.num_rows, 0);
    ctx.last.resize(config.num_rows);
    ctx.q.init(1u << 8);

    // Create test data arrays
    std::vector<int64_t> ts_ns(config.num_rows, 0);
    std::vector<int64_t> px_n(config.num_rows, 0);
    std::vector<int64_t> qty(config.num_rows, 0);
    std::vector<uint8_t> side(config.num_rows, 0);

    HostMDSlot slot = {};
    slot.num_rows = config.num_rows;
    slot.ts_ns = ts_ns.data();
    slot.px_n = px_n.data();
    slot.qty = qty.data();
    slot.side = side.data();
    slot.user = &ctx;
    slot.begin_row_write = &host_begin_row_write;
    slot.end_row_write = &host_end_row_write;
    slot.notify_row_dirty = &host_notify_row_dirty;

    // Test with empty queue - should not crash
    uint64_t current_time = 100;
    uint64_t next_paint = 200;
    
    EXPECT_NO_THROW(update_latest_data_from_context(ctx, config, current_time, next_paint, slot));
    
    // Add some updates to queue
    ctx.q.push(1);
    ctx.q.push(3);
    
    EXPECT_NO_THROW(update_latest_data_from_context(ctx, config, current_time, next_paint, slot));
    
    // Verify that dirty flags were set
    EXPECT_EQ(ctx.dirty[1], 1);
    EXPECT_EQ(ctx.dirty[3], 1);
    EXPECT_EQ(ctx.dirty[0], 0);  // Should not be dirty
}
