
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

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

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

static void imgui_render_table(const HostMDSlot* slot, HostContext* ctx, uint64_t& next_paint_ms) {
    uint32_t id;
    while (ctx->q.pop(id)) {
        if (id < ctx->num_rows) ctx->dirty[id] = 1;
    }

    uint64_t t = now_ms();
    bool should_refresh = (t >= next_paint_ms);

    if (ImGui::Begin("Market (SoA / Model A)")) {
        ImGui::Text("Rows: %u", ctx->num_rows);
        ImGui::Separator();
        ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;
        if (ImGui::BeginTable("md_table", 5, flags)) {
            ImGui::TableSetupScrollFreeze(0,1);
            ImGui::TableSetupColumn("ID");
            ImGui::TableSetupColumn("TS (ns)");
            ImGui::TableSetupColumn("PX (n)");
            ImGui::TableSetupColumn("QTY");
            ImGui::TableSetupColumn("SIDE");
            ImGui::TableHeadersRow();

            ImGuiListClipper clipper;
            clipper.Begin((int)ctx->num_rows);
            while (clipper.Step()) {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                    HostContext::RowSnap snap = ctx->last[i];
                    if (should_refresh && ctx->dirty[i]) {
                        ctx->dirty[i] = 0;
                        HostContext::RowSnap tmp{};
                        bool ok=false;
                        for (int tries=0; tries<2 && !ok; ++tries) ok = row_snapshot(ctx, slot, (uint32_t)i, tmp);
                        if (ok) {
                            if (std::memcmp(&tmp, &ctx->last[i], sizeof tmp) != 0) {
                                ctx->last[i] = tmp;
                            }
                            snap = ctx->last[i];
                        }
                    }
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%d", i);
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%lld", (long long)snap.ts);
                    ImGui::TableSetColumnIndex(2); ImGui::Text("%lld", (long long)snap.px);
                    ImGui::TableSetColumnIndex(3); ImGui::Text("%lld", (long long)snap.qty);
                    ImGui::TableSetColumnIndex(4); ImGui::Text("%u",  (unsigned)snap.side);
                }
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }

    if (should_refresh) next_paint_ms = t + 250;
}

int main(int argc, char** argv) {
    uint32_t num_rows = 10000;
    uint32_t writers = 2;
    uint32_t ups = 60000;

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

    std::vector<int64_t> ts_ns(num_rows, 0);
    std::vector<int64_t> px_n (num_rows, 0);
    std::vector<int64_t> qty  (num_rows, 0);
    std::vector<uint8_t> side (num_rows, 0);

    HostContext ctx;
    ctx.num_rows = num_rows;
    ctx.seq = std::make_unique<std::atomic<uint32_t>[]>(num_rows);
    for (uint32_t i = 0; i < num_rows; ++i) ctx.seq[i].store(0, std::memory_order_relaxed);
    ctx.dirty.assign(num_rows, 1); // initial paint
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
    if (!h) { fprintf(stderr, "Failed to load %s\n", libname); return 1; }
    typedef MD_API (*GetApiFn)(uint32_t);
    auto get_api = (GetApiFn)lib_sym(h, "get_marketdata_api");
    if (!get_api) { fprintf(stderr, "Missing get_marketdata_api\n"); return 1; }
    const uint32_t EXPECTED_API = 1;
    MD_API api = get_api(EXPECTED_API);
    if (api.api_version != EXPECTED_API || !api.bind_host_buffers || !api.start || !api.stop) {
        fprintf(stderr, "Plugin API mismatch.\n"); return 1;
    }
    if (api.bind_host_buffers(&slot) != 0) { fprintf(stderr, "bind_host_buffers failed.\n"); return 1; }
    api.start(writers, ups);

    if (!glfwInit()) { fprintf(stderr, "GLFW init failed\n"); return 1; }
#if __APPLE__
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Market Data (Model A, SoA)", nullptr, nullptr);
    if (!window) { fprintf(stderr, "GLFW window failed\n"); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    uint64_t next_paint = now_ms() + 250;
    auto start_time = steady_clock::now();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        imgui_render_table(&slot, &ctx, next_paint);

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        if (duration_cast<seconds>(steady_clock::now() - start_time).count() > 10) break;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    api.stop();
    lib_close(h);
    return 0;
}
