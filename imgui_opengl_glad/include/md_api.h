
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
  Host-owned SoA buffers
  - Host allocates arrays (ts_ns, px_n, qty, side) with length = num_rows.
  - Plugin writes directly into those arrays.
  - Plugin must call begin_row_write/end_row_write around each row update.
  - After writing a row, plugin calls notify_row_dirty(row_id) to enqueue for UI.
  - C ABI only at the boundary (POD + function pointers).
*/

typedef struct {
    const char* ptr;
    uint32_t    len;
} StrView;

// Forward declare
struct HostMDSlot;

// Host callbacks (implemented by host)
typedef void (*Host_BeginRowWriteFn)(struct HostMDSlot* slot, uint32_t row_id);
typedef void (*Host_EndRowWriteFn  )(struct HostMDSlot* slot, uint32_t row_id);
typedef void (*Host_NotifyDirtyFn  )(struct HostMDSlot* slot, uint32_t row_id);

// Host-owned buffers and context
typedef struct HostMDSlot {
    uint32_t num_rows;
    int64_t* ts_ns;  // timestamp ns
    int64_t* px_n;   // price (fixed-point int64)
    int64_t* qty;    // quantity
    uint8_t* side;   // 0=unk,1=bid,2=ask,3=trade

    void*    user;   // opaque host pointer (context)

    Host_BeginRowWriteFn begin_row_write;
    Host_EndRowWriteFn   end_row_write;
    Host_NotifyDirtyFn   notify_row_dirty;
} HostMDSlot;

// Plugin API
typedef struct {
    uint32_t api_version; // must equal host EXPECTED_API

    // Bind host buffers / callbacks. Return 0 on success.
    int  (*bind_host_buffers)(HostMDSlot* host_slot);

    // Start/stop simulator inside plugin
    void (*start)(uint32_t threads, uint32_t updates_per_sec);
    void (*stop)(void);
} MD_API;

#if defined(_WIN32) || defined(_WIN64)
  #define API_EXPORT __declspec(dllexport)
#else
  #define API_EXPORT __attribute__((visibility("default")))
#endif

// Single stable C symbol to fetch the API table
API_EXPORT MD_API get_marketdata_api(uint32_t expected_api_version);

#ifdef __cplusplus
} // extern "C"
#endif
