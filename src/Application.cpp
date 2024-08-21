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

extern void ImGui_ImplWin32_CreateWindow(ImGuiViewport* viewport);


static void Hook_Platform_CreateWindow(ImGuiViewport* viewport) {
    //ImGui_ImplWin32_CreateWindow(viewport);

    //HWND* hwnd = reinterpret_cast<HWND*>(static_cast<char*>(viewport->PlatformUserData));

    assert(viewport->RendererUserData == NULL);

    WGL_WindowData *data = IM_NEW(WGL_WindowData);
    CreateDeviceWGL((HWND)viewport->PlatformHandle, data);
    viewport->RendererUserData = data;
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
    ImGui_ImplWin32_EnableDpiAwareness();
    m_Window = std::make_unique<BorderlessWindow>(applicationSpecification.Width,
                                                  applicationSpecification.Height);

    Init();

    CenterWindow();
    ApplyBorderlessFrame();
    ApplyBgColors();

    PushLayer<StreamedImage>();

    g_Instance = this;
}

Application::~Application() {
}

void Application::Run() {
    m_Running = true;
    bool done = false;

    MSG msg;
    ImVec4 clear_color = ImVec4(.0f, .0f, .0f, .0f);

    static float dt = 0.0f;
    float frame_time = 0.0f;
    static int desired_fr = 120;
    mcctp::utils::Timer clock;
    clock.Reset();

    while (!done) {
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
        //while (::GetMessage(&msg, NULL, 0U, 0U)) {
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
                // Update layers
                {
                    for (auto& layer : m_LayerStack)
                        layer->OnUpdate(frame_time);
                }

                // Dockspace
                {
                    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(),
                                                 ImGuiDockNodeFlags_PassthruCentralNode);
                }

                // Draw Layers
                {
                    for (auto &layer : m_LayerStack)
                        layer->OnUIRender();
                }
            }
            // Rendering
            RenderAppFrame();

            int64_t COEFF{ -100'0 };
            int64_t TIMEOUT{ 6'0 };
            LARGE_INTEGER dueTime;
            desired_fr = 60;
            if (GetActiveWindow() == NULL) {
                desired_fr = 10;
                TIMEOUT = 97'0;
            }

            dueTime.QuadPart = TIMEOUT * COEFF;
            
            if (!m_Window->is_in_size_move()) {
                if (SetWaitableTimerEx(m_hFrameIRTimer, &dueTime, 0, 0, 0, nullptr, 0) == FALSE)
                {
                    DWORD err = GetLastError();
                    std::cerr << "Failed to set timer, error code = " << err << std::endl;
                    CloseHandle(m_hFrameIRTimer);
                    __debugbreak();
                }

                DWORD result = WaitForSingleObjectEx(m_hFrameIRTimer, INFINITE, TRUE);
                if (!(result == WAIT_OBJECT_0)) {
                    __debugbreak();
                }
            }

            dt += clock.Elapsed();
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

    g_ApplicationRunning = false;
}

void Application::Close() {
}

Application *Application::Instance() {
    return (Application *)g_Instance;
}

void Application::Init() {

    ULONG currentRes;
    NtSetTimerResolution(5000, TRUE, &currentRes);

    m_hFrameIRTimer = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
    if (m_hFrameIRTimer == NULL) {
        DWORD err = GetLastError();
        std::cerr << "Failed to create a high resolution waitable timer with a name, error code = " << err << std::endl;
        std::exit(1);
    }

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

    DwmEnableMMCSS(TRUE);
    // Setup Platform/Renderer backends
    ImGui_ImplWin32_EnableAlphaCompositing(m_Window->m_hHWND.get());
    ImGui_ImplWin32_InitForOpenGL(m_Window->m_hHWND.get());
    ImGui_ImplOpenGL3_Init();
    float dpiscale = ImGui_ImplWin32_GetDpiScaleForHwnd(m_Window->m_hHWND.get());
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
    ImFont *myriadPro = io.Fonts->AddFontFromFileTTF("MyriadPro-Light.ttf", 16.0f * dpiscale);
    // io.FontDefault = robotoFont;
    io.FontDefault = myriadPro;
}
void Application::Shutdown() {}
void Application::StartNewAppFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}
void Application::RenderAppFrame() {
    // Render
    ImGui::ShowDemoWindow();
    ImGui::Render();
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, m_Window->get_width(), m_Window->get_height());
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
        //##MainMenuBar
        // Add imgui windows that aren't default rects/dockspaces/etc to client
        // area whitelist, but explicitly include imgui demo
        std::vector<RECT> WindowRects;
        for (ImGuiWindow *window : ImGui::GetCurrentContext()->Windows) {
            if ((!(std::string(window->Name).find("Default") != std::string::npos) &&
                 (!(std::string(window->Name).find("Dock") != std::string::npos)) ||
                (std::string(window->Name).find("Dear ImGui Demo") != std::string::npos))) {
                ImVec2 pos = window->Pos;
                ImVec2 size = window->Size;
                RECT rect = {origin.x + pos.x, origin.y + pos.y, origin.x + (pos.x + size.x),
                             origin.y + (pos.y + size.y)};

                if (window->Active)
                    WindowRects.push_back(rect);
            }
        }
        m_Window->set_client_area(WindowRects);
    }
    // Blit
    ::SwapBuffers(g_MainWindow.hDC);
    ImGui_ImplWin32_EnableAlphaCompositing(m_Window->m_hHWND.get());

    // Update and Render additional Platform Windows
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        // Restore the OpenGL rendering context to the main window DC, since
        // platform windows might have changed it.
        wglMakeCurrent(g_MainWindow.hDC, g_hRC);
    }


}
void Application::FixTimestep()
{
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
void Application::ApplyBorderlessFrame()
{
    static ImVec4 color =
        ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
    ACCENT_POLICY policy = { ACCENT_STATE(ACCENT_ENABLE_BLURBEHIND),
                                1, (((int)(color.w * 255)) << 24) | (((int)(color.z * 255)) << 16) |
        (((int)(color.y * 255)) << 8) | ((int)(color.x * 255)),
                                0 };

    const WINDOWCOMPOSITIONATTRIBDATA data = { WCA_ACCENT_POLICY, &policy,
                                              sizeof(policy) };

    SetWindowCompositionAttribute(m_Window->m_hHWND.get(), &data);
}
void Application::ApplyBgColors()
{
    ImVec4 menu_col = ImGui::GetStyle().Colors[ImGuiCol_MenuBarBg];
    menu_col.w = 150.f / 255.f;
    ImGui::GetStyle().Colors[ImGuiCol_MenuBarBg] = menu_col;

    ImVec4 win_col = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
    win_col.w = 96.f / 255.f;
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = win_col;
}
} // namespace mcctp
