#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include "cJSON.h"


#if defined(_WIN32)
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3native.h>
    #include <windows.h>
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten!
// If you are compiling with Emscripten, please use your browser current target image notes.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Desktop Pages Structure
    struct Page {
        char Name[32];
        int id; // Unique ID for each page
        Page(const char* name, int page_id) : id(page_id) {
            snprintf(Name, IM_ARRAYSIZE(Name), "%s", name);
        }
    };

    // Zone Structure
    struct Zone {
        char Name[32];
        ImRect Rect;
        int id; // Unique ID for each zone
        int page_id; // ID of the page this zone belongs to
        // Future: color, transparency, etc.

        Zone(const char* name, ImRect rect, int zone_id, int p_id) : Rect(rect), id(zone_id), page_id(p_id) {
            snprintf(Name, IM_ARRAYSIZE(Name), "%s", name);
        }
    };

    ImVector<Page> pages;
    ImVector<Zone> zones;
    int nextPageId = 1; // Used for generating unique page IDs
    int nextZoneId = 1; // Used for generating unique zone IDs
    int active_page_idx = 0; // Keep track of the currently selected page index

    const char* configFilePath = "config.json";

    // Functions to Save and Load Configuration
    auto SaveConfiguration = [&]() {
        cJSON *root = cJSON_CreateObject();

        // Save Pages
        cJSON *pagesArray = cJSON_CreateArray();
        cJSON_AddItemToObject(root, "pages", pagesArray);
        for (int i = 0; i < pages.Size; ++i) {
            cJSON *page_obj = cJSON_CreateObject();
            cJSON_AddStringToObject(page_obj, "name", pages[i].Name);
            cJSON_AddNumberToObject(page_obj, "id", pages[i].id);
            cJSON_AddItemToArray(pagesArray, page_obj);
        }
        cJSON_AddNumberToObject(root, "nextPageId", nextPageId);

        // Save Zones
        cJSON *zonesArray = cJSON_CreateArray();
        cJSON_AddItemToObject(root, "zones", zonesArray);
        for (int i = 0; i < zones.Size; ++i) {
            cJSON *zone_obj = cJSON_CreateObject();
            cJSON_AddStringToObject(zone_obj, "name", zones[i].Name);
            cJSON_AddNumberToObject(zone_obj, "id", zones[i].id);
            cJSON_AddNumberToObject(zone_obj, "page_id", zones[i].page_id);
            cJSON *rect_obj = cJSON_CreateObject();
            cJSON_AddNumberToObject(rect_obj, "x", zones[i].Rect.Min.x);
            cJSON_AddNumberToObject(rect_obj, "y", zones[i].Rect.Min.y);
            cJSON_AddNumberToObject(rect_obj, "w", zones[i].Rect.GetWidth());
            cJSON_AddNumberToObject(rect_obj, "h", zones[i].Rect.GetHeight());
            cJSON_AddItemToObject(zone_obj, "rect", rect_obj);
            cJSON_AddItemToArray(zonesArray, zone_obj);
        }
        cJSON_AddNumberToObject(root, "nextZoneId", nextZoneId);


        char *json_string = cJSON_Print(root);
        std::ofstream configFile(configFilePath);
        if (configFile.is_open()) {
            configFile << json_string;
            configFile.close();
        }
        cJSON_free(json_string);
        cJSON_Delete(root);
    };

    auto LoadConfiguration = [&]() {
        std::ifstream configFile(configFilePath);
        if (!configFile.is_open()) {
            pages.push_back(Page("Desktop 1", nextPageId++)); // Default page
            SaveConfiguration(); // Create a default config file
            return;
        }

        std::stringstream buffer;
        buffer << configFile.rdbuf();
        configFile.close();
        std::string json_string = buffer.str();

        if (json_string.empty()) {
             pages.push_back(Page("Desktop 1", nextPageId++)); // Default page
             SaveConfiguration(); // Create a default config file
             return;
        }

        cJSON *root = cJSON_Parse(json_string.c_str());
        if (!root) {
            fprintf(stderr, "Error parsing config.json: [%s]\n", cJSON_GetErrorPtr());
            pages.push_back(Page("Desktop 1", nextPageId++)); // Default on error
            return;
        }

        // Load Pages
        cJSON *pagesArray = cJSON_GetObjectItem(root, "pages");
        if (cJSON_IsArray(pagesArray)) {
            pages.clear();
            cJSON *page_obj = NULL;
            cJSON_ArrayForEach(page_obj, pagesArray) {
                cJSON *name_item = cJSON_GetObjectItem(page_obj, "name");
                cJSON *id_item = cJSON_GetObjectItem(page_obj, "id");
                if (cJSON_IsString(name_item) && (name_item->valuestring != NULL) && cJSON_IsNumber(id_item)) {
                    pages.push_back(Page(name_item->valuestring, id_item->valueint));
                }
            }
        }

        cJSON *nextPageIdItem = cJSON_GetObjectItem(root, "nextPageId");
        if(cJSON_IsNumber(nextPageIdItem)) {
            nextPageId = nextPageIdItem->valueint;
        } else {
            nextPageId = pages.Size + 1;
        }

        // Load Zones
        cJSON *zonesArray = cJSON_GetObjectItem(root, "zones");
        if (cJSON_IsArray(zonesArray)) {
            zones.clear();
            cJSON *zone_obj = NULL;
            cJSON_ArrayForEach(zone_obj, zonesArray) {
                cJSON *name_item = cJSON_GetObjectItem(zone_obj, "name");
                cJSON *id_item = cJSON_GetObjectItem(zone_obj, "id");
                cJSON *page_id_item = cJSON_GetObjectItem(zone_obj, "page_id");
                cJSON *rect_obj = cJSON_GetObjectItem(zone_obj, "rect");

                if (cJSON_IsString(name_item) && name_item->valuestring != NULL &&
                    cJSON_IsNumber(id_item) && cJSON_IsNumber(page_id_item) && cJSON_IsObject(rect_obj)) {

                    cJSON *rect_x = cJSON_GetObjectItem(rect_obj, "x");
                    cJSON *rect_y = cJSON_GetObjectItem(rect_obj, "y");
                    cJSON *rect_w = cJSON_GetObjectItem(rect_obj, "w");
                    cJSON *rect_h = cJSON_GetObjectItem(rect_obj, "h");

                    if (cJSON_IsNumber(rect_x) && cJSON_IsNumber(rect_y) && cJSON_IsNumber(rect_w) && cJSON_IsNumber(rect_h)) {
                        ImRect rect( (float)rect_x->valuedouble, (float)rect_y->valuedouble,
                                     (float)(rect_x->valuedouble + rect_w->valuedouble), (float)(rect_y->valuedouble + rect_h->valuedouble));
                        zones.push_back(Zone(name_item->valuestring, rect, id_item->valueint, page_id_item->valueint));
                    }
                }
            }
        }

        cJSON *nextZoneIdItem = cJSON_GetObjectItem(root, "nextZoneId");
        if(cJSON_IsNumber(nextZoneIdItem)) {
            nextZoneId = nextZoneIdItem->valueint;
        } else {
            nextZoneId = zones.Size + 1;
        }

        cJSON_Delete(root);

        if (pages.empty()) { // Ensure there's at least one page
            pages.push_back(Page("Desktop 1", nextPageId++));
            SaveConfiguration();
        }
    };

    LoadConfiguration();


    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // For a transparent window, we need to disable decoration and enable framebuffer transparency
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);


    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Desktop Overlay", nullptr, nullptr);
    if (window == nullptr)
        return 1;

#ifdef _WIN32
    HWND hwnd = glfwGetWin32Window(window);
    // Set window to be always on top
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    // Further transparency setup for click-through might be needed depending on exact requirements,
    // e.g. WS_EX_LAYERED and SetLayeredWindowAttributes for click-through,
    // or handling WM_NCHITTEST to return HTTRANSPARENT.
    // For now, this makes it borderless, transparent framebuffer, and always on top.
#endif

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will build the font atlas during runtime.
    // - If you want to queries for font data, e.g. for a custom font selector:
    //      io.Fonts->AddFontDefault();
    //      io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //      io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //      io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //      io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //      ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    // - Remember that finding a path to a font file can be tricky tricky. Refer to your platform/engine documentation to know how to reliably load a file.
    // - For icons fonts, see: https://github.com/ocornut/imgui/wiki/Using-Font-Icons
    // io.Fonts->AddFontDefault();

    bool show_demo_window = false; // Disable demo window by default for our app
    // Set clear color to be transparent
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    // Desktop Pages
    struct Page {
        char Name[32];
        // In the future, add content for each page here
        // For now, just a name
        Page(const char* name) {
            snprintf(Name, IM_ARRAYSIZE(Name), "%s", name);
        }
    };
    ImVector<Page> pages;
    pages.push_back(Page("Desktop 1")); // Start with one page
    int nextPageNumber = 2;


    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen().
    // You may ignore this check with Define IMGUI_DISABLE_FILE_FUNCTIONS.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // Create a full-screen, transparent window for the overlay content
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGuiWindowFlags host_window_flags = 0;
        host_window_flags |= ImGuiWindowFlags_NoTitleBar;
        host_window_flags |= ImGuiWindowFlags_NoResize;
        host_window_flags |= ImGuiWindowFlags_NoMove;
        host_window_flags |= ImGuiWindowFlags_NoScrollbar;
        host_window_flags |= ImGuiWindowFlags_NoSavedSettings;
        host_window_flags |= ImGuiWindowFlags_NoBackground; // Make host window transparent

        ImGui::Begin("DesktopOverlayHost", nullptr, host_window_flags);

        // Tab Bar for Desktop Pages
        if (ImGui::BeginTabBar("DesktopPagesTabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_FittingPolicyScroll)) {
            // Add New Page Button
            if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip)) {
                char new_page_name[32];
                snprintf(new_page_name, IM_ARRAYSIZE(new_page_name), "Desktop %d", nextPageId); // Use nextPageId here
                pages.push_back(Page(new_page_name, nextPageId++));
                SaveConfiguration();
            }

            int current_active_page_id = -1;
            if (!pages.empty() && active_page_idx >=0 && active_page_idx < pages.Size) {
                 current_active_page_id = pages[active_page_idx].id;
            }


            for (int i = 0; i < pages.Size; ) {
                bool open = true;
                // Store the current page index if this tab is active
                if (ImGui::BeginTabItem(pages[i].Name, &open, ImGuiTabItemFlags_None)) {
                    active_page_idx = i; // Update active page index
                    current_active_page_id = pages[i].id;

                    // "Add Zone" button for the current page
                    if (ImGui::Button("Add Zone")) {
                        char zone_name[32];
                        snprintf(zone_name, IM_ARRAYSIZE(zone_name), "Zone %d", nextZoneId);
                        // Default position and size for new zone
                        ImRect zone_rect(viewport->Pos.x + 50, viewport->Pos.y + 100,
                                         viewport->Pos.x + 250, viewport->Pos.y + 300);
                        zones.push_back(Zone(zone_name, zone_rect, nextZoneId++, current_active_page_id));
                        SaveConfiguration();
                    }
                    ImGui::Separator();

                    // Draw zones for the current active page
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    for (int z_idx = 0; z_idx < zones.Size; ++z_idx) {
                        if (zones[z_idx].page_id == current_active_page_id) {
                            ImVec2 zone_pos = zones[z_idx].Rect.Min;
                            ImVec2 zone_size = zones[z_idx].Rect.GetSize();
                            draw_list->AddRectFilled(zone_pos, zones[z_idx].Rect.Max, IM_COL32(50, 50, 50, 150));
                            draw_list->AddRect(zone_pos, zones[z_idx].Rect.Max, IM_COL32(200, 200, 200, 200));
                            draw_list->AddText(zone_pos + ImVec2(5,5), IM_COL32(255,255,255,255), zones[z_idx].Name);
                        }
                    }

                    ImGui::EndTabItem();
                }
                if (!open && pages.Size > 1) { // Don't close the last tab
                    // If the closed tab was the active one, adjust active_page_idx
                    if (active_page_idx == i) {
                        active_page_idx = ImMax(0, i - 1);
                    } else if (active_page_idx > i) {
                        active_page_idx--;
                    }
                    pages.erase(pages.Data + i);
                    SaveConfiguration();
                    // No increment for i here because the vector shifted.
                } else {
                    i++;
                }
            }
            ImGui::EndTabBar();
        }

        ImGui::End(); // End DesktopOverlayHost

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    SaveConfiguration(); // Save config before exiting
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
