
#include <atomic>
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include <cstdint>
#include <cstdio>

#include "../include/api.h"

using namespace std::chrono;

static HostMDSlot* g_slot = nullptr;
static std::atomic<bool> g_run{false};
static std::vector<std::thread> g_threads;

static void writer_thread(uint32_t thread_id, uint32_t updates_per_sec) {
    std::mt19937_64 rng((uint64_t)steady_clock::now().time_since_epoch().count() ^ (thread_id*0x9e3779b97f4a7c15ull));
    std::uniform_int_distribution<uint32_t> row_dist(0, g_slot->num_rows-1);
    std::uniform_int_distribution<int64_t>  px_jump(-50, 50);
    std::uniform_int_distribution<int64_t>  qty_jump(1, 100);
    std::uniform_int_distribution<int>      side_pick(1, 3);

    double per_update_ns = 1e9 / (double)updates_per_sec;
    auto next_tick = steady_clock::now();

    while (g_run.load(std::memory_order_acquire)) {
        uint32_t i = row_dist(rng);
        int64_t ts = duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
        int64_t px = g_slot->px_n[i] + px_jump(rng);
        int64_t qt = g_slot->qty[i] + qty_jump(rng);
        uint8_t sd = (uint8_t)side_pick(rng);

        g_slot->begin_row_write(g_slot, i);
        g_slot->ts_ns[i] = ts;
        g_slot->px_n [i] = px;
        g_slot->qty  [i] = qt;
        g_slot->side [i] = sd;
        g_slot->end_row_write(g_slot, i);
        g_slot->notify_row_dirty(g_slot, i);

        next_tick += nanoseconds((long long)per_update_ns);
        std::this_thread::sleep_until(next_tick);
    }
}

extern "C" int bind_host_buffers_c(HostMDSlot* slot) {
    g_slot = slot;
    return 0;
}

extern "C" void start_c(uint32_t threads, uint32_t updates_per_sec) {
    if (!g_slot) return;
    g_run.store(true, std::memory_order_release);
    g_threads.reserve(threads);
    uint32_t ups_per_thread = std::max(1u, updates_per_sec / std::max(1u, threads));
    for (uint32_t t=0; t<threads; ++t) {
        g_threads.emplace_back(writer_thread, t, ups_per_thread);
    }
}

extern "C" void stop_c(void) {
    g_run.store(false, std::memory_order_release);
    for (auto& th : g_threads) if (th.joinable()) th.join();
    g_threads.clear();
}

extern "C" API_EXPORT MD_API get_marketdata_api(uint32_t expected) {
    MD_API api{};
    if (expected != 1) return api;
    api.api_version = 1;
    api.bind_host_buffers = &bind_host_buffers_c;
    api.start = &start_c;
    api.stop  = &stop_c;
    return api;
}
