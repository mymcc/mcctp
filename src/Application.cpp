#include "mcctppch.h"
#include "Application.h"
#include "Layer/StreamedImage.h"

// App Instance
extern void* g_Instance;
extern bool  g_ApplicationRunning;
static int   g_Width;
static int   g_Height;

// Data
HGLRC            g_hRC;
WGL_WindowData   g_MainWindow;

bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data);
void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data);
//void ResetDeviceWGL();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Helper functions
bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
	HDC hDc = ::GetDC(hWnd);
	PIXELFORMATDESCRIPTOR pfd = { 0 };
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;

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

void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
	wglMakeCurrent(NULL, NULL);
	::ReleaseDC(hWnd, data->hDC);
}

static void Hook_Renderer_CreateWindow(ImGuiViewport* viewport)
{
	assert(viewport->RendererUserData == NULL);

	WGL_WindowData* data = IM_NEW(WGL_WindowData);
	CreateDeviceWGL((HWND)viewport->PlatformHandle, data);
	viewport->RendererUserData = data;
}

static void Hook_Renderer_DestroyWindow(ImGuiViewport* viewport)
{
	if (viewport->RendererUserData != NULL)
	{
		WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData;
		CleanupDeviceWGL((HWND)viewport->PlatformHandle, data);
		IM_DELETE(data);
		viewport->RendererUserData = NULL;
	}
}

static void Hook_Platform_RenderWindow(ImGuiViewport* viewport, void*)
{
	// Activate the platform window DC in the OpenGL rendering context
	if (WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData)
	{
		wglMakeCurrent(data->hDC, g_hRC);
	}
}

static void Hook_Renderer_SwapBuffers(ImGuiViewport* viewport, void*)
{
	if (WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData)
		::SwapBuffers(data->hDC);
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			//mcctp::Application().Instance()->GetWidth() = LOWORD(lParam);
			//mcctp::Application().Instance()->GetHeight() = HIWORD(lParam);
			g_Width = LOWORD(lParam);
			g_Height = HIWORD(lParam);
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		g_ApplicationRunning = false;
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

extern bool g_ApplicationRunning;

namespace mcctp
{
	Application::Application(const ApplicationSpecification& applicationSpecification)
	{
		Init();
		CenterWindow();

		PushLayer<StreamedImage>();

		g_Instance = this;
	}

	Application::~Application()
	{

	}

	void Application::Run()
	{
		m_Running = true;
		bool done = false;

		MSG msg;
		ImVec4 clear_color = ImVec4(.0f, .0f, .0f, 0.f);

		while (!done)
		{
			//MCCTT_PROFILE_FUNCTION();
			while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
				if (msg.message == WM_QUIT)
					done = true;
			}
			if (done) break;

			StartNewAppFrame();
			{
				for (auto& layer : m_LayerStack) layer->OnUpdate(ImGui::GetIO().DeltaTime);

				// Dockspace
				{
					ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
				}

				// ImGui Demo
				{
					ImGui::ShowDemoWindow();
				}

				// App Layers
				{
					for (auto& layer : m_LayerStack) layer->OnUIRender();
				}
			}
			// Rendering
			RenderAppFrame();
		}
	}

	void Application::Close()
	{

	}

	Application* Application::Instance() 
	{
		return (Application*)g_Instance;
	}

    void Application::Init()
    {
		ImGui_ImplWin32_EnableDpiAwareness();
		static std::wstring title = converter().from_bytes(m_Specification.Name);
		m_hWC = { sizeof(m_hWC), CS_OWNDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, title.c_str(), NULL};
		::RegisterClassExW(&m_hWC);
		m_hHWND = ::CreateWindowW(m_hWC.lpszClassName, L"Dear ImGui Win32+OpenGL3 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, m_hWC.hInstance, NULL);

		if (!::CreateDeviceWGL(m_hHWND, &g_MainWindow))
		{
			::CleanupDeviceWGL(m_hHWND, &g_MainWindow);
			::DestroyWindow(m_hHWND);
			::UnregisterClassW(m_hWC.lpszClassName, GetModuleHandle(NULL));
			std::exit(1);
		}
		
		wglMakeCurrent(g_MainWindow.hDC, g_hRC);
		
		::ShowWindow(m_hHWND, SW_SHOWDEFAULT);
		::UpdateWindow(m_hHWND);
		
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Enable Multi-Viewport / Platform Windows

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();
		
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		
		// Setup Platform/Renderer backends
		ImGui_ImplWin32_InitForOpenGL(m_hHWND);
		ImGui_ImplOpenGL3_Init();
		
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
			IM_ASSERT(platform_io.Renderer_CreateWindow == NULL);
			IM_ASSERT(platform_io.Renderer_DestroyWindow == NULL);
			IM_ASSERT(platform_io.Renderer_SwapBuffers == NULL);
			IM_ASSERT(platform_io.Platform_RenderWindow == NULL);
			platform_io.Renderer_CreateWindow = Hook_Renderer_CreateWindow;
			platform_io.Renderer_DestroyWindow = Hook_Renderer_DestroyWindow;
			platform_io.Renderer_SwapBuffers = Hook_Renderer_SwapBuffers;
			platform_io.Platform_RenderWindow = Hook_Platform_RenderWindow;
		}
		//ImGui::GetIO().ConfigViewportsNoDecoration = false;

		//ImFontConfig fontConfig;
		//fontConfig.FontDataOwnedByAtlas = false;
		//ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular, sizeof(g_RobotoRegular), 20.0f, &fontConfig);
		//io.FontDefault = robotoFont;
    }
	void Application::Shutdown()
	{
	}
	void Application::StartNewAppFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}
	void Application::RenderAppFrame()
	{
		// Render
		ImGui::Render();
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();

			// Restore the OpenGL rendering context to the main window DC, since platform windows might have changed it.
			wglMakeCurrent(g_MainWindow.hDC, g_hRC);
		}

		// Blit
		::SwapBuffers(g_MainWindow.hDC);
	}
	void Application::CenterWindow() 
	{
		RECT rect;
		GetWindowRect(m_hHWND, &rect);

		int screen_width = GetSystemMetrics(SM_CXSCREEN);
		int screen_height = GetSystemMetrics(SM_CYSCREEN);

		int window_width = rect.right - rect.left;
		int window_height = rect.bottom - rect.top;

		int x = (screen_width - window_width) / 2;
		int y = (screen_height - window_height) / 2;

		SetWindowPos(m_hHWND, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}
}
