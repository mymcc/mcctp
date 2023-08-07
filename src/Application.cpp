#include "mcctppch.h"
#include "Application.h"
#include "Layer/StreamedImage.h"

// App Instance
extern void *g_Instance;
extern bool g_ApplicationRunning;
static int g_Width;
static int g_Height;

// Data
HGLRC g_hRC;
WGL_WindowData g_MainWindow;

bool CreateDeviceWGL(HWND hWnd, WGL_WindowData *data);
void CleanupDeviceWGL(HWND hWnd, WGL_WindowData *data);

template <typename T> struct SmartProperty {
  public:
    T m_Value; // The value to be changed/checked

    SmartProperty(T value) : m_Value(value), m_LastValue(value), m_Changed(FALSE) {
    }

    BOOL update() {
        if (m_Value == m_LastValue)
            m_Changed = FALSE;
        else
            m_Changed = TRUE;
        m_LastValue = m_Value;
        return m_Changed;
    }

    BOOL has_changed() const {
        return m_Changed;
    }

  private:
    T m_LastValue;
    BOOL m_Changed;
};

// Helper functions
bool CreateDeviceWGL(HWND hWnd, WGL_WindowData *data) {
    HDC hDc = ::GetDC(hWnd);
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,                            // Version Number
        PFD_DRAW_TO_WINDOW |          // Format Must Support Window
            PFD_SUPPORT_OPENGL |      // Format Must Support OpenGL
            PFD_SUPPORT_COMPOSITION | // Format Must Support Composition
            PFD_DOUBLEBUFFER,         // Must Support Double Buffering
        PFD_TYPE_RGBA,                // Request An RGBA Format
        32,                           // Select Our Color Depth
        0,
        0,
        0,
        0,
        0,
        0, // Color Bits Ignored
        8, // An Alpha Buffer
        0, // Shift Bit Ignored
        0, // No Accumulation Buffer
        0,
        0,
        0,
        0,              // Accumulation Bits Ignored
        24,             // 16Bit Z-Buffer (Depth Buffer)
        8,              // Some Stencil Buffer
        0,              // No Auxiliary Buffer
        PFD_MAIN_PLANE, // Main Drawing Layer
        0,              // Reserved
        0,
        0,
        0 // Layer Masks Ignored
    };

    const int pf = ::ChoosePixelFormat(hDc, &pfd);
    if (pf == 0)
        return false;
    if (::SetPixelFormat(hDc, pf, &pfd) == FALSE)
        return false;
    ::ReleaseDC(hWnd, hDc);

    data->hDC = ::GetDC(hWnd);
    if (!g_hRC)
        g_hRC = wglCreateContext(data->hDC);
    return true;
}

void CleanupDeviceWGL(HWND hWnd, WGL_WindowData *data) {
    wglMakeCurrent(NULL, NULL);
    ::ReleaseDC(hWnd, data->hDC);
}

static void Hook_Renderer_CreateWindow(ImGuiViewport *viewport) {
    assert(viewport->RendererUserData == NULL);

    WGL_WindowData *data = IM_NEW(WGL_WindowData);
    CreateDeviceWGL((HWND)viewport->PlatformHandle, data);
    viewport->RendererUserData = data;
}

static void Hook_Renderer_DestroyWindow(ImGuiViewport *viewport) {
    if (viewport->RendererUserData != NULL) {
        WGL_WindowData *data = (WGL_WindowData *)viewport->RendererUserData;
        CleanupDeviceWGL((HWND)viewport->PlatformHandle, data);
        IM_DELETE(data);
        viewport->RendererUserData = NULL;
    }
}

static void Hook_Platform_RenderWindow(ImGuiViewport *viewport, void *) {
    // Activate the platform window DC in the OpenGL rendering context
    if (WGL_WindowData *data = (WGL_WindowData *)viewport->RendererUserData) {
        wglMakeCurrent(data->hDC, g_hRC);
    }
}

static void Hook_Renderer_SwapBuffers(ImGuiViewport *viewport, void *) {
    if (WGL_WindowData *data = (WGL_WindowData *)viewport->RendererUserData)
        ::SwapBuffers(data->hDC);
}

extern bool g_ApplicationRunning;

namespace mcctp {
Application::Application(const ApplicationSpecification &applicationSpecification) {
    m_Window = std::make_unique<BorderlessWindow>(applicationSpecification.Width,
                                                  applicationSpecification.Height);

    Init();
    CenterWindow();

    PushLayer<StreamedImage>();

    g_Instance = this;
}

Application::~Application() {
}

void Application::Run() {
    m_Running = true;
    bool done = false;

    MSG msg;
    ImVec4 clear_color = ImVec4(.0f, .0f, .0f, 0.f);

    static float dt = 0.0f;
    float frame_time = 0.0f;
    static int desired_fr = 120;
    mcctp::utils::Timer clock;
    static bool performance_mode = true;

    ImVec4 menu_col = ImGui::GetStyle().Colors[ImGuiCol_MenuBarBg];
    menu_col.w = 150.f / 255.f;
    ImGui::GetStyle().Colors[ImGuiCol_MenuBarBg] = menu_col;

    ImVec4 win_col = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
    win_col.w = 96.f / 255.f;
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = win_col;

    clock.Reset();
    while (!done) {
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        static auto render = [&]() {
            StartNewAppFrame();
            {
                for (auto &layer : m_LayerStack)
                    layer->OnUpdate(frame_time);

                // Dockspace
                {
                    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(),
                                                 ImGuiDockNodeFlags_PassthruCentralNode);
                }

                // App Layers
                {
                    for (auto &layer : m_LayerStack)
                        layer->OnUIRender();
                }

                // Demo
                { ImGui::ShowDemoWindow(); }

                ImGuiWindowFlags window_flags =
                    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
                    ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing |
                    ImGuiWindowFlags_NoNav;
                {
                    if (ImGui::Begin("Borderless Settings", 0, window_flags)) {
                        static SmartProperty<INT> window_mode{1};
                        ImGui::RadioButton("Windowed", &window_mode.m_Value, 0);
                        ImGui::RadioButton("Borderless", &window_mode.m_Value, 1);
                        if (window_mode.update())
                            m_Window->set_borderless(window_mode.m_Value);
                    }
                    ImGui::End();
                }

                // Borderless Demo
                {
                    // DWM api is undocumented, and has varying behavior between
                    // Windows Releases Therefore Accent Flags and Animation
                    // Flags are essentially trial and error :) On my system,
                    // Accent Flags set to 1 allows for the desired effects in
                    // regards to Accent State.
                    //
                    // These are just default values that achieve the desired
                    // effect on my system Windows 10 Build 19044

                    ImGui::Begin("DWM Accent State", 0, window_flags);
                    static SmartProperty<INT> accent_policy{ACCENT_ENABLE_BLURBEHIND};
                    ImGui::SeparatorText("DWM Accent State");
                    ImGui::RadioButton("DISABLED", &accent_policy.m_Value, ACCENT_DISABLED);
                    ImGui::RadioButton("GRADIENT", &accent_policy.m_Value, ACCENT_ENABLE_GRADIENT);
                    ImGui::RadioButton("TRANSPARENT GRADIENT", &accent_policy.m_Value,
                                       ACCENT_ENABLE_TRANSPARENTGRADIENT);
                    ImGui::RadioButton("BLUR BEHIND", &accent_policy.m_Value,
                                       ACCENT_ENABLE_BLURBEHIND);
                    ImGui::RadioButton("ACRYLIC BLUR BEHIND", &accent_policy.m_Value,
                                       ACCENT_ENABLE_ACRYLICBLURBEHIND);
                    ImGui::RadioButton("HOST BACKDROP", &accent_policy.m_Value,
                                       ACCENT_ENABLE_HOSTBACKDROP);
                    ImGui::RadioButton("INVALID STATE", &accent_policy.m_Value,
                                       ACCENT_INVALID_STATE);
                    ImGui::End();

                    ImGui::Begin("DWM Accent Flags", 0, window_flags);
                    ImGui::SeparatorText("DWM Accent Flags");
                    static SmartProperty<INT> accent_flags{1};
                    ImGui::SliderInt("Accent Flags", &accent_flags.m_Value, 0, 255);
                    ImGui::End();

                    ImGui::Begin("DWM Gradient", 0, window_flags);
                    ImGui::SeparatorText("DWM Gradient");
                    static ImVec4 color =
                        ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
                    static SmartProperty<INT> gradient_col = {
                        (((int)(color.w * 255)) << 24) | (((int)(color.z * 255)) << 16) |
                        (((int)(color.y * 255)) << 8) | ((int)(color.x * 255))};
                    ImGui::ColorPicker4("##picker", (float *)&color,
                                        ImGuiColorEditFlags_NoSidePreview |
                                            ImGuiColorEditFlags_NoSmallPreview);
                    gradient_col.m_Value = (((int)(color.w * 255)) << 24) |
                                           (((int)(color.z * 255)) << 16) |
                                           (((int)(color.y * 255)) << 8) | ((int)(color.x * 255));
                    ImGui::End();

                    ImGui::Begin("DWM Animation id", 0, window_flags);
                    ImGui::SeparatorText("DWM Animation id");
                    static SmartProperty<INT> animation_id{0};
                    ImGui::SliderInt("Accent Flags", &animation_id.m_Value, 0, 32);
                    ImGui::End();

                    accent_policy.update();
                    accent_flags.update();
                    gradient_col.update();
                    animation_id.update();

                    static bool init_accents = true; // to apply default initialization
                    if (accent_policy.has_changed() || accent_flags.has_changed() ||
                        gradient_col.has_changed() || animation_id.has_changed() || init_accents) {
                        if (init_accents)
                            init_accents = false;

                        ACCENT_POLICY policy = {ACCENT_STATE(accent_policy.m_Value),
                                                accent_flags.m_Value, gradient_col.m_Value,
                                                animation_id.m_Value};

                        const WINDOWCOMPOSITIONATTRIBDATA data = {WCA_ACCENT_POLICY, &policy,
                                                                  sizeof(policy)};

                        SetWindowCompositionAttribute(m_Window->m_hHWND.get(), &data);
                    }
                }
            }
            // Rendering
            RenderAppFrame();

            // Frame-rate limiter
            float desired_frame_time = (1000.0f / (desired_fr * 1000.0f));
            dt += clock.Elapsed();
            if (performance_mode) {
                while (dt <= desired_frame_time) {
                    mcctp::utils::Timer splt;
                    // Yeah yeah giving up cpu time is 'spensive but the cpu
                    // load reduction here is epic
                    LONGLONG ns = ((
                        ((((1000.0f / (desired_fr * 1000.0f)) - dt) * 1000000000LL) / 100) * 100));
                    if (ns > 0) {
                        // assert(qnanosleep(ns) != FALSE);
                        mcctp::utils::qnanosleep(ns);
                    }
                    dt += splt.Elapsed();
                }
            }
            frame_time = dt;
            dt = 0.0f;

            clock.Reset();
        };

        static bool set_render = false;
        if (!set_render) {
            m_Window->set_render_callback(render); // render when we are stuck in the message loop
                                                   // doing wm_sizing and wm_move
            set_render = true;
        }

        render(); // render when we leave message loop
    }
}

void Application::Close() {
}

Application *Application::Instance() {
    return (Application *)g_Instance;
}

void Application::Init() {
    ImGui_ImplWin32_EnableDpiAwareness();

    if (!::CreateDeviceWGL(m_Window->m_hHWND.get(), &g_MainWindow)) {
        ::CleanupDeviceWGL(m_Window->m_hHWND.get(), &g_MainWindow);
        ::DestroyWindow(m_Window->m_hHWND.get());
        ::UnregisterClassW((LPCWSTR)m_Window->m_wstrWC, GetModuleHandle(NULL));
        std::exit(1);
    }

    wglMakeCurrent(g_MainWindow.hDC, g_hRC);

    ::ShowWindow(m_Window->m_hHWND.get(), SW_SHOWDEFAULT);
    ::UpdateWindow(m_Window->m_hHWND.get());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad
    // Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport
                                                        // / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_InitForOpenGL(m_Window->m_hHWND.get());
    ImGui_ImplOpenGL3_Init();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();
        IM_ASSERT(platform_io.Renderer_CreateWindow == NULL);
        IM_ASSERT(platform_io.Renderer_DestroyWindow == NULL);
        IM_ASSERT(platform_io.Renderer_SwapBuffers == NULL);
        IM_ASSERT(platform_io.Platform_RenderWindow == NULL);
        platform_io.Renderer_CreateWindow = Hook_Renderer_CreateWindow;
        platform_io.Renderer_DestroyWindow = Hook_Renderer_DestroyWindow;
        platform_io.Renderer_SwapBuffers = Hook_Renderer_SwapBuffers;
        platform_io.Platform_RenderWindow = Hook_Platform_RenderWindow;
    }

    ImFontConfig fontConfig;
    fontConfig.FontDataOwnedByAtlas = false;
    // ImFont* robotoFont =
    // io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular,
    // sizeof(g_RobotoRegular), 20.0f, &fontConfig);
    ImFont *myriadPro = io.Fonts->AddFontFromFileTTF("MyriadPro-Light.ttf", 16.0f);
    // io.FontDefault = robotoFont;
    io.FontDefault = myriadPro;
}
void Application::Shutdown() {
}
void Application::StartNewAppFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}
void Application::RenderAppFrame() {
    // Render
    ImGui::Render();
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update imgui window rects for hit testing
    {
        // Get ScreenPos offset
        ImGuiViewport *vp = ImGui::GetMainViewport();
        HWND handle = (HWND)vp->PlatformHandle;
        RECT r;
        GetWindowRect(handle, &r);

        // Only apply offset if Multi-viewports are not enabled
        ImVec2 origin = {(float)r.left, (float)r.top};
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            origin = {0, 0};
        }

        // Add imgui windows that aren't default rects/dockspaces/etc to client
        // area whitelist, but explicitly include imgui demo
        std::vector<RECT> WindowRects;
        for (ImGuiWindow *window : ImGui::GetCurrentContext()->Windows) {
            if ((!(std::string(window->Name).find("Default") != std::string::npos) &&
                 (!(std::string(window->Name).find("Dock") != std::string::npos)) &&
                 (!(std::string(window->Name).find("Menu") != std::string::npos))) ||
                (std::string(window->Name).find("Dear ImGui Demo") != std::string::npos)) {
                ImVec2 pos = window->Pos;
                ImVec2 size = window->Size;
                RECT rect = {origin.x + pos.x, origin.y + pos.y, origin.x + (pos.x + size.x),
                             origin.y + (pos.y + size.y)};
                WindowRects.push_back(rect);
            }
        }
        m_Window->set_client_area(WindowRects);
    }

    // Update and Render additional Platform Windows
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        // Restore the OpenGL rendering context to the main window DC, since
        // platform windows might have changed it.
        wglMakeCurrent(g_MainWindow.hDC, g_hRC);
    }

    // Blit
    ::SwapBuffers(g_MainWindow.hDC);
}
void Application::CenterWindow() {
    RECT rect;
    GetWindowRect(m_Window->m_hHWND.get(), &rect);

    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);

    int window_width = rect.right - rect.left;
    int window_height = rect.bottom - rect.top;

    int x = (screen_width - window_width) / 2;
    int y = (screen_height - window_height) / 2;

    SetWindowPos(m_Window->m_hHWND.get(), NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}
} // namespace mcctp
