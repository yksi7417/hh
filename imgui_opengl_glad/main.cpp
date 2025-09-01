#include "main_context.h"
#include "IMGuiComponents.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "imgui.h"

#include <thread>

using namespace std;

struct EmspConfig {
    uint32_t num_rows = 10000;
    uint32_t writers = 2;
    uint32_t ups = 50000;
};

void initializeHostContext(HostContext& ctx, HostMDSlot& slot, 
                          const EmspConfig& config, 
                          std::vector<int64_t>& ts_ns,
                          std::vector<int64_t>& px_n,
                          std::vector<int64_t>& qty,
                          std::vector<uint8_t>& side) {
    // Initialize HostContext
    ctx.num_rows = config.num_rows;
    ctx.seq = std::make_unique<std::atomic<uint32_t>[]>(config.num_rows);
    for (uint32_t i = 0; i < config.num_rows; ++i) {
        ctx.seq[i].store(0, std::memory_order_relaxed);
    }
    ctx.dirty.assign(config.num_rows, 0);
    ctx.last.resize(config.num_rows);
    ctx.q.init(1u<<18);

    // Initialize HostMDSlot
    slot = HostMDSlot{};
    slot.num_rows = config.num_rows;
    slot.ts_ns = ts_ns.data();
    slot.px_n  = px_n.data();
    slot.qty   = qty.data();
    slot.side  = side.data();
    slot.user = &ctx;
    slot.begin_row_write = &host_begin_row_write;
    slot.end_row_write   = &host_end_row_write;
    slot.notify_row_dirty= &host_notify_row_dirty;
}

struct PluginHandle {
    LibHandle handle;
    MD_API api;
    void Cleanup() {
        if (handle) lib_close(handle);
        handle = nullptr;
        api.stop();
        api = {};
    }
    ~PluginHandle() {
        Cleanup();
    }
};

bool loadMarketDataPlugin(PluginHandle& plugin, HostMDSlot &slot) {
#ifdef _WIN32
    const char* libname = "md_plugin.dll";
#elif __APPLE__
    const char* libname = "libmd_plugin.dylib";
#else
    const char* libname = "libmd_plugin.so";
#endif

    plugin.handle = lib_open(libname);
    if (!plugin.handle) {
#ifdef _WIN32
        fprintf(stderr, "Failed to load %s\n", libname);
#else
        fprintf(stderr, "Failed to load %s: %s\n", libname, dlerror());
#endif
        return false;
    }
    
    typedef MD_API (*GetApiFn)(uint32_t);
    auto get_api = (GetApiFn)lib_sym(plugin.handle, "get_marketdata_api");
    if (!get_api) {
        fprintf(stderr, "Symbol get_marketdata_api not found.\n");
        return false;
    }
    
    const uint32_t EXPECTED_API = 1;
    plugin.api = get_api(EXPECTED_API);
    if (plugin.api.api_version != EXPECTED_API || !plugin.api.bind_host_buffers || !plugin.api.start || !plugin.api.stop) {
        fprintf(stderr, "Plugin API mismatch.\n");
        plugin.api = {}; // Clear invalid API
        return false;
    }

    if (!plugin.handle) {
        fprintf(stderr, "loading handle failed.\n");
        return false;
    }

    if (plugin.api.api_version == 0) {
        fprintf(stderr, "api_version is 0 which is invalid \n");
        return false;
    }

    if (plugin.api.bind_host_buffers(&slot) != 0) {
        fprintf(stderr, "bind_host_buffers failed.\n");
        return false;
    }
    
    return true;
}

EmspConfig parseCommandLineArguments(int argc, char** argv) {
    EmspConfig config;
    
    if (argc > 1) config.num_rows = std::max(100u, (uint32_t)std::strtoul(argv[1], nullptr, 10));
    if (argc > 2) config.writers  = std::max(1u, (uint32_t)std::strtoul(argv[2], nullptr, 10));
    if (argc > 3) config.ups      = std::max(100u, (uint32_t)std::strtoul(argv[3], nullptr, 10));
    
    return config;
}

GLFWwindow* initializeGLFWAndOpenGL(const char** glsl_version_out) {
	if (!glfwInit())
		return nullptr;

	// GL 3.0 + GLSL 130
	const char *glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

	// Create window with graphics context
	GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui - Example", NULL, NULL);
	if (window == NULL)
		return nullptr;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))  // tie window context to glad's opengl funcs
		throw("Unable to context to OpenGL");

	int screen_width, screen_height;
	glfwGetFramebufferSize(window, &screen_width, &screen_height);
	glViewport(0, 0, screen_width, screen_height);

	if (glsl_version_out)
		*glsl_version_out = glsl_version;

	return window;
}


void update_latest_data_from_context(HostContext &ctx, EmspConfig &config, uint64_t t, uint64_t next_paint, HostMDSlot &slot)
{
    uint32_t id;
    while (ctx.q.pop(id))
    {
        if (id < config.num_rows)
            ctx.dirty[id] = 1;
    }

    if (t >= next_paint)
    {
        uint32_t printed = 0;
        for (uint32_t i = 0; i < config.num_rows; ++i)
        {
            if (!ctx.dirty[i])
                continue;
            ctx.dirty[i] = 0;

            HostContext::RowSnap snap{};
            bool ok = false;
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


/**
 * @brief Memory Model Of Main Program
 *
 * This program uses a memory model that relies on stack-allocated vectors
 * to store trading data. The vectors are initialized with a fixed size
 * based on the command line arguments and are not resized during the
 * program's execution.
 *
 * @param argc
 * @param argv
 * @return int , 0 = success, non zero is error
 */
int main(int argc, char** argv) 
{
	const char* glsl_version;
	GLFWwindow* window = initializeGLFWAndOpenGL(&glsl_version);
	if (!window) {
		return 1;
	}

    EmspConfig config = parseCommandLineArguments(argc, argv);

    // Vector Representation of Trading Data  
    // Do not grow these vectors - they are meant to be on stack and maintain the 
    // same size for lifetime of the program 

    std::vector<int64_t> ts_ns(config.num_rows, 0);
    std::vector<int64_t> px_n (config.num_rows, 0);
    std::vector<int64_t> qty  (config.num_rows, 0);
    std::vector<uint8_t> side (config.num_rows, 0);

    HostContext ctx;
    HostMDSlot slot;
    initializeHostContext(ctx, slot, config, ts_ns, px_n, qty, side);
    printf("Host (console) rows=%u writers=%u updates/sec=%u\n", config.num_rows, config.writers, config.ups);

    PluginHandle plugin; 

    if (loadMarketDataPlugin(plugin, slot)) {
        plugin.api.start(config.writers, config.ups);
        uint64_t t = now_ms();
        uint64_t next_paint = t + 100;

        ImGuiComponents myimgui;
        try {
            myimgui.Init(window, glsl_version);
            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();

                update_latest_data_from_context(ctx, config, t, next_paint, slot);

                glClear(GL_COLOR_BUFFER_BIT);
                myimgui.NewFrame();
                myimgui.Update(ctx, slot, next_paint);
                myimgui.Render();
                
                // Update and Render additional Platform Windows
                ImGuiIO& io = ImGui::GetIO();
                if (io.ConfigFlags & (1 << 10)) // ImGuiConfigFlags_ViewportsEnable
                {
                    ImGui::UpdatePlatformWindows();
                    ImGui::RenderPlatformWindowsDefault();
                }
                
                glfwSwapBuffers(window);
                next_paint = t + 250;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        catch (...) {
            fprintf(stderr, "An error occurred, cleaning up\n");
        }
        plugin.Cleanup();
        myimgui.Shutdown();
    }

    return 0;
    }

