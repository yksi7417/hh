
# SoA Market Data Demo (Model A) — with ImGui Table

A small, hot-reload–friendly demo where the **host owns Struct-of-Arrays (SoA)** buffers and the **plugin** writes updates directly into them. The host can be a console app or an **ImGui** GUI that shows a big table, refreshing only dirty rows every 250 ms.

## Build

### Linux / macOS
```bash
cd soa_md_modelA_demo
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DWITH_IMGUI=ON
cmake --build . -j
```
> CMake will fetch **GLFW** and **Dear ImGui** via `FetchContent` (internet required).

### Windows (MSVC)
```bat
cd soa_md_modelA_demo
mkdir build && cd build
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DWITH_IMGUI=ON
cmake --build . -j
```
> If not using Ninja, omit `-G "Ninja"` and use the default generator.

### Outputs
- Console host: `host/host_demo`
- ImGui host:   `host/host_imgui`
- Plugin:       `plugin/md_plugin.(dll|so|dylib)`

Place the plugin library alongside the host executable you plan to run.

## Run

Console (prints up to 25 changed rows per 250 ms frame for ~10s):
```
./host_demo 20000 4 80000
```

ImGui table (scrollable, clipped rows, ~10s demo window):
```
./host_imgui 20000 4 80000
```

## Design (Model A)
- **C ABI boundary:** single unmangled symbol `get_marketdata_api()` returning a versioned function table (POD).
- **Host-owned SoA:** columns `ts_ns`, `px_n`, `qty`, `side` are contiguous arrays owned by the host → zero-copy reads for UI.
- **Seqlock per row:** plugin calls `begin_row_write → write columns → end_row_write`; host snapshot reads with acquire loads.
- **Dirty signaling:** plugin pushes `row_id` into a host **MPSC queue**; host sets a `dirty[]` flag, refreshes only those rows each frame.
- **Hot-reload ready:** on reload, call `bind_host_buffers()` again; ABI remains C-only and stable.

## Notes
- No STL/C++ objects cross the boundary; the plugin can still use full C++ internally.
- If you hit reader starvation under extreme write rates, switch to a per-row **double-buffer + flip index** (append-only ABI change).
- For a real app, replace the 10s loop with your main run loop and add DLL copy/reload mechanics.

Have fun!
