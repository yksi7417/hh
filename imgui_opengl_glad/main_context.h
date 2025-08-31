#pragma once

#include "platform.h"
#include "include/md_api.h"
#include "mpsc.h"
#include <chrono>
#include <memory>

/******************************************************************************
    Arena for preallocated memory used for entire lifetime of main program
    Design Philisophy & Trader Off 
    * Performace > Flexibility 
        * Let's decide at program start time if this program is a
        *  XS = 128 MB
        *   S = 256 MB
        *   M = 512 MB
        *   L = 1 GB
        *  XL = 2 GB
        * XXL = 4 GB
        * if you are using M, and realize you need more than M, upgrade to L!
        * Save file should make upgrading easy!
        * Just restart your program and the binary format should be loaded quickly and restore previous state
    * Hot loading of functionalities
        * Functionalities lives in shared library (plugin.dll)
        * You can re-compile and get latest behaviour easily without restart
        * Data is owned by main program, and will not change for the lifetime of the program
*/

struct HostContext {
    std::unique_ptr<std::atomic<uint32_t>[]> seq;
    std::vector<uint8_t> dirty;
    struct RowSnap { int64_t ts, px, qty; uint8_t side; };
    std::vector<RowSnap> last;

    MPSCQueue q;
    std::atomic<bool> running{true};
    uint32_t num_rows{0};
};

static void host_begin_row_write(HostMDSlot* slot, uint32_t i) {
    HostContext* ctx = (HostContext*)slot->user;
    uint32_t s = ctx->seq[i].load(std::memory_order_relaxed);
    ctx->seq[i].store(s+1, std::memory_order_release); // odd
}
static void host_end_row_write(HostMDSlot* slot, uint32_t i) {
    HostContext* ctx = (HostContext*)slot->user;
    uint32_t s = ctx->seq[i].load(std::memory_order_relaxed);
    ctx->seq[i].store(s+1, std::memory_order_release); // even
}
static void host_notify_row_dirty(HostMDSlot* slot, uint32_t i) {
    HostContext* ctx = (HostContext*)slot->user;
    ctx->q.push(i);
}

static bool row_snapshot(const HostContext* ctx, const HostMDSlot* slot, uint32_t i, HostContext::RowSnap& out) {
    uint32_t s1 = ctx->seq[i].load(std::memory_order_acquire);
    if (s1 & 1u) return false;
    HostContext::RowSnap tmp{ slot->ts_ns[i], slot->px_n[i], slot->qty[i], slot->side[i] };
    std::atomic_thread_fence(std::memory_order_acquire);
    uint32_t s2 = ctx->seq[i].load(std::memory_order_acquire);
    if (s1 != s2 || (s2 & 1u)) return false;
    out = tmp;
    return true;
}

static uint64_t now_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(lib_now().time_since_epoch()).count();
}