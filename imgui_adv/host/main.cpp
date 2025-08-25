
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
#include <string>
#include <algorithm>
#include <memory>

#include "platform.h"
#include "../include/api.h"
#include "mpsc.h"

using namespace std::chrono;

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
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

int main(int argc, char** argv) {
    uint32_t num_rows = 10000;
    uint32_t writers = 2;
    uint32_t ups = 50000;

    if (argc > 1) num_rows = std::max(100u, (uint32_t)std::strtoul(argv[1], nullptr, 10));
    if (argc > 2) writers  = std::max(1u, (uint32_t)std::strtoul(argv[2], nullptr, 10));
    if (argc > 3) ups      = std::max(100u, (uint32_t)std::strtoul(argv[3], nullptr, 10));

#ifdef _WIN32
    const char* libname = "md_plugin.dll";
#elif __APPLE__
    const char* libname = "libmd_plugin.dylib";
#else
    const char* libname = "libmd_plugin.so";
#endif

    printf("Host (console) rows=%u writers=%u updates/sec=%u\n", num_rows, writers, ups);

    std::vector<int64_t> ts_ns(num_rows, 0);
    std::vector<int64_t> px_n (num_rows, 0);
    std::vector<int64_t> qty  (num_rows, 0);
    std::vector<uint8_t> side (num_rows, 0);

    HostContext ctx;
    ctx.num_rows = num_rows;
    ctx.seq = std::make_unique<std::atomic<uint32_t>[]>(num_rows);
    for (uint32_t i = 0; i < num_rows; ++i) ctx.seq[i].store(0, std::memory_order_relaxed);
    ctx.dirty.assign(num_rows, 0);
    ctx.last.resize(num_rows);
    ctx.q.init(1u<<18);

    HostMDSlot slot{};
    slot.num_rows = num_rows;
    slot.ts_ns = ts_ns.data();
    slot.px_n  = px_n.data();
    slot.qty   = qty.data();
    slot.side  = side.data();
    slot.user = &ctx;
    slot.begin_row_write = &host_begin_row_write;
    slot.end_row_write   = &host_end_row_write;
    slot.notify_row_dirty= &host_notify_row_dirty;

    LibHandle h = lib_open(libname);
    if (!h) {
#ifdef _WIN32
        fprintf(stderr, "Failed to load %s\n", libname);
#else
        fprintf(stderr, "Failed to load %s: %s\n", libname, dlerror());
#endif
        return 1;
    }
    typedef MD_API (*GetApiFn)(uint32_t);
    auto get_api = (GetApiFn)lib_sym(h, "get_marketdata_api");
    if (!get_api) { fprintf(stderr, "Symbol get_marketdata_api not found.\n"); return 1; }
    const uint32_t EXPECTED_API = 1;
    MD_API api = get_api(EXPECTED_API);
    if (api.api_version != EXPECTED_API || !api.bind_host_buffers || !api.start || !api.stop) {
        fprintf(stderr, "Plugin API mismatch.\n");
        return 1;
    }
    if (api.bind_host_buffers(&slot) != 0) {
        fprintf(stderr, "bind_host_buffers failed.\n");
        return 1;
    }

    api.start(writers, ups);

    uint64_t next_paint = now_ms() + 250;
    auto end_time = steady_clock::now() + seconds(10);

    while (steady_clock::now() < end_time) {
        uint32_t id;
        while (ctx.q.pop(id)) {
            if (id < num_rows) ctx.dirty[id] = 1;
        }

        uint64_t t = now_ms();
        if (t >= next_paint) {
            uint32_t printed = 0;
            for (uint32_t i=0; i<num_rows; ++i) {
                if (!ctx.dirty[i]) continue;
                ctx.dirty[i] = 0;

                HostContext::RowSnap snap{};
                bool ok=false;
                for (int tries=0; tries<4 && !ok; ++tries) ok = row_snapshot(&ctx, &slot, i, snap);
                if (!ok) continue;

                if (std::memcmp(&snap, &ctx.last[i], sizeof snap) != 0) {
                    std::printf("Row %6u  ts=%lld  px=%lld  qty=%lld  side=%u\n",
                                i, (long long)snap.ts, (long long)snap.px, (long long)snap.qty, snap.side);
                    ctx.last[i] = snap;
                    ++printed;
                }
            }
            next_paint = t + 250;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    api.stop();
    lib_close(h);
    printf("Host (console) exiting.\n");
    return 0;
}
