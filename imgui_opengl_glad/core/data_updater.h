#pragma once

#include "main_context.h"
#include <cstdint>
#include <cstdio>
#include <cstring>

/**
 * @brief Configuration structure for EMSP (Electronic Market Simulation Platform)
 */
struct EmspConfig {
    uint32_t num_rows = 10000;  ///< Number of data rows
    uint32_t writers = 2;       ///< Number of writer threads
    uint32_t ups = 50000;       ///< Updates per second
};

/**
 * @brief Updates the latest market data from the context
 * 
 * This function processes queued row updates and optionally prints changed rows
 * when the paint interval has elapsed. It's designed to be called in the main
 * rendering loop to keep the display synchronized with incoming market data.
 * 
 * @param ctx The host context containing the data structures
 * @param config Configuration parameters for the system
 * @param t Current timestamp in milliseconds
 * @param next_paint Timestamp when next paint/display update should occur
 * @param slot Market data slot containing the actual data arrays
 */
void update_latest_data_from_context(HostContext& ctx, 
                                   const EmspConfig& config, 
                                   uint64_t t, 
                                   uint64_t next_paint, 
                                   const HostMDSlot& slot);
