#include "main_context.h"
#include "IMGuiComponents.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

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
};

PluginHandle loadMarketDataPlugin() {
#ifdef _WIN32
    const char* libname = "md_plugin.dll";
#elif __APPLE__
    const char* libname = "libmd_plugin.dylib";
#else
    const char* libname = "libmd_plugin.so";
#endif

    PluginHandle plugin{};
    
    plugin.handle = lib_open(libname);
    if (!plugin.handle) {
#ifdef _WIN32
        fprintf(stderr, "Failed to load %s\n", libname);
#else
        fprintf(stderr, "Failed to load %s: %s\n", libname, dlerror());
#endif
        return plugin;
    }
    
    typedef MD_API (*GetApiFn)(uint32_t);
    auto get_api = (GetApiFn)lib_sym(plugin.handle, "get_marketdata_api");
    if (!get_api) {
        fprintf(stderr, "Symbol get_marketdata_api not found.\n");
        return plugin;
    }
    
    const uint32_t EXPECTED_API = 1;
    plugin.api = get_api(EXPECTED_API);
    if (plugin.api.api_version != EXPECTED_API || !plugin.api.bind_host_buffers || !plugin.api.start || !plugin.api.stop) {
        fprintf(stderr, "Plugin API mismatch.\n");
        plugin.api = {}; // Clear invalid API
        return plugin;
    }
    
    return plugin;
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

    PluginHandle plugin = loadMarketDataPlugin();
    if (!plugin.handle || plugin.api.api_version == 0) {
        return 1;
    }
    
    // Extract api to stack variable for cleaner usage
    MD_API api = plugin.api;
    
    if (api.bind_host_buffers(&slot) != 0) {
        fprintf(stderr, "bind_host_buffers failed.\n");
        return 1;
    }

    api.start(config.writers, config.ups);


	ImGuiComponents myimgui;
	myimgui.Init(window, glsl_version);
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);
		myimgui.NewFrame();
		myimgui.Update();
		myimgui.Render();
		glfwSwapBuffers(window);
	}
	myimgui.Shutdown();

	return 0;
}