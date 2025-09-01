#include "data_updater.h"

void update_latest_data_from_context(HostContext& ctx, 
                                   const EmspConfig& config, 
                                   uint64_t t, 
                                   uint64_t next_paint, 
                                   const HostMDSlot& slot)
{
    uint32_t id;
    // Performance critical: tight loop processing queued market data updates
    while (ctx.q.pop(id))
    {
        if (id < config.num_rows)
            ctx.dirty[id] = 1;
    }

    if (t >= next_paint)
    {
        uint32_t printed = 0;
        // Performance critical: batch processing all rows for rendering
        for (uint32_t i = 0; i < config.num_rows; ++i)
        {
            if (!ctx.dirty[i])
                continue;
            ctx.dirty[i] = 0;

            HostContext::RowSnap snap{};
            bool ok = false;
            // Performance critical: retry loop for atomic row snapshot
            for (int tries = 0; tries < 4 && !ok; ++tries)
                ok = row_snapshot(&ctx, &slot, i, snap);
            if (!ok)
                continue;

            if (std::memcmp(&snap, &ctx.last[i], sizeof snap) != 0)
            {
                std::printf("Row %6u  ts=%lld  px=%lld  qty=%lld  side=%u\n",
                            i, (long long)snap.ts, (long long)snap.px, (long long)snap.qty, snap.side);
                ctx.last[i] = snap;
                ++printed;
            }
        }
    }
}
