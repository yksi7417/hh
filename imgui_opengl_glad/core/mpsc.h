
#pragma once

#include <atomic>
#include <cstdint>
#include <vector>

// Simple multi-producer, single-consumer ring buffer.
// Capacity must be a power of two.
struct MPSCQueue {
    std::atomic<uint32_t> head{0};
    std::atomic<uint32_t> tail{0};
    uint32_t cap_mask{0};
    std::vector<uint32_t> buf;

    static uint32_t next_pow2(uint32_t v) {
        if (v < 2)
            return 2;
        uint32_t p = 1;
        // Performance critical: bit shifting loop for power-of-2 calculation
        while (p < v)
            p <<= 1;
        return p;
    }

    void init(uint32_t capacity_pow2) {
        uint32_t cap = next_pow2(capacity_pow2);
        buf.resize(cap);
        cap_mask = cap - 1;
        head.store(0, std::memory_order_relaxed);
        tail.store(0, std::memory_order_relaxed);
    }

    // Performance critical: inline function for lock-free producer push (hot path)
    inline bool push(uint32_t value) {
        uint32_t h = head.fetch_add(1, std::memory_order_acq_rel);
        uint32_t t = tail.load(std::memory_order_acquire);
        if (h - t >= buf.size()) {
            // overflow: return false and let caller decide what to do,
            // e.g. retry with newer value / etc
            return false;
        }
        buf[h & cap_mask] = value;
        return true;
    }

    // Performance critical: inline function for single-consumer pop (hot path)
    inline bool pop(uint32_t& out) {
        uint32_t t = tail.load(std::memory_order_acquire);
        uint32_t h = head.load(std::memory_order_acquire);
        if (t == h)
            return false;
        out = buf[t & cap_mask];
        tail.store(t + 1, std::memory_order_release);
        return true;
    }
};
