#include <string>
#include <memory>
#include <vector>
#include <stdint.h>
#include <filesystem>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

// Data stored per platform window
struct WGL_WindowData { HDC hDC; };

// Dependencies
#include "mcctp.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_win32.h"

#include <locale>
#include <codecvt>
#pragma warning( disable : 4996 )
using converter = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>;