#include "IMGuiComponents.h"
#include "main_context.h"
#include <cstring>
#include "imgui.h"

void ImGuiComponents::Init(GLFWwindow* window, const char* glsl_version) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	
	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
}

void ImGuiComponents::NewFrame() {
	// feed inputs to dear imgui, start new frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}


void imgui_render_table(HostContext& ctx, const HostMDSlot& slot, const bool should_refresh) {
    uint32_t id;
    while (ctx.q.pop(id)) {
        if (id < ctx.num_rows) ctx.dirty[id] = 1;
    }

    if (ImGui::Begin("MarketData")) {
        ImGui::Text("Rows: %u", ctx.num_rows);
        ImGui::Separator();
        ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;
        if (ImGui::BeginTable("md_table", 5, flags)) {
            ImGui::TableSetupScrollFreeze(0,1);
            ImGui::TableSetupColumn("ID");
            ImGui::TableSetupColumn("TS (ns)");
            ImGui::TableSetupColumn("PX (n)");
            ImGui::TableSetupColumn("QT (n)");
            ImGui::TableSetupColumn("SIDE");
            ImGui::TableHeadersRow();

            ImGuiListClipper clipper;
            clipper.Begin((int)ctx.num_rows);
            while (clipper.Step()) {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                    HostContext::RowSnap snap = ctx.last[i];
                    if (should_refresh && ctx.dirty[i]) {
                        ctx.dirty[i] = 0;
                        HostContext::RowSnap tmp{};
                        bool ok=false;
                        for (int tries=0; tries<2 && !ok; ++tries) ok = row_snapshot(&ctx, &slot, (uint32_t)i, tmp);
                        if (ok) {
                            if (std::memcmp(&tmp, &ctx.last[i], sizeof tmp) != 0) {
                                ctx.last[i] = tmp;
                            }
                            snap = ctx.last[i];
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
    }
    ImGui::End();  // Always call End() regardless of Begin() return value

}

void ImGuiComponents::Update(HostContext& ctx, const HostMDSlot& slot, uint64_t& next_paint_ms) {
    // Create main window with dockspace (similar to imgui_basic)
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("MainWindow", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    // Create dockspace
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    ImGui::End(); // End MainWindow

    // Now render the market data table as a separate dockable window
    uint64_t t = now_ms();
    bool should_refresh = (t >= next_paint_ms);
    imgui_render_table(ctx, slot, should_refresh);
    if (should_refresh) next_paint_ms = t + 250;
}

void ImGuiComponents::Render() {
	// Render dear imgui into screen
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Update and Render additional Platform Windows
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
}

void ImGuiComponents::Shutdown() {
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

