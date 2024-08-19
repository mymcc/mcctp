#include <algorithm>
#include <string>
#include <memory>
#include <vector>
#include <stdint.h>
#include <filesystem>
#include <deque>
#include <atomic>
#include <chrono>
#include <thread>
#include <windows.h>
#include <psapi.h>
#include <tchar.h>
#include <mmsystem.h>
#include <avrt.h>
#include <dwmapi.h>
#define STATUS_SUCCESS 0
#define STATUS_TIMER_RESOLUTION_NOT_SET 0xC0000245
extern "C" NTSYSAPI NTSTATUS NTAPI NtSetTimerResolution(ULONG DesiredResolution, BOOLEAN SetResolution, PULONG CurrentResolution);
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "dwmapi.lib")
// Data stored per platform window
struct WGL_WindowData {
    HDC hDC;
};

// Dependencies
#include "mcctp.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_win32.h"

#include "stb_image.h"

#include <locale>
#include <codecvt>
#pragma warning(disable : 4996)
using converter = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>;

#include "BorderlessWindow/swcadef.h"
#include "BorderlessWindow/BorderlessWindow.hpp"

// Courtesy of https://gist.github.com/sylveon/9c199bb6684fe7dffcba1e3d383fb609
static const auto SetWindowCompositionAttribute =
    reinterpret_cast<PFN_SET_WINDOW_COMPOSITION_ATTRIBUTE>(
        GetProcAddress(GetModuleHandle(L"user32.dll"), "SetWindowCompositionAttribute"));

#include "Utils/Timer.h"
#include "Utils/Perf.h"