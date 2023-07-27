#include <filesystem>
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <GL/gl.h>
#include <GL/glu.h>
#include <windows.h>

// Data stored per platform window
struct WGL_WindowData {
  HDC hDC;
};

// Dependencies
#include "mcctp.h"

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_win32.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <codecvt>
#include <locale>
#pragma warning(disable : 4996)
using converter = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>;
