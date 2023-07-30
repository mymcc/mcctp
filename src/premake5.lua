project "mcctp-example"
    language "C++"
    cppdialect "C++20"
    targetdir "bin/%{cfg.buildcfg}"
    staticruntime "on"

    pchheader "mcctppch.h"
    pchsource "mcctppch.cpp"
    
    files 
    {
        "premake5.lua",
        "**.h",
        "**.hpp",
        "**.cpp",
    }

    includedirs
    {
        ".",
        "%{IncludeDir.mcctp}",
        "%{IncludeDir.imgui}",
        "%{IncludeDir.fpng}",
    }

    links
    {
        "ImGui",
        "mcctp",
        "opengl32.lib",
    }

    libdirs
    {

    }

    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"
        defines { "MCCTP_EXAMPLE_PLATFORM_WINDOWS" }

    filter "configurations:Debug"
        kind "ConsoleApp"
        defines { "MCCTP_EXAMPLE_DEBUG" }
        runtime "Debug"
        optimize "Off"
        symbols "On"

    filter "configurations:Release"
        kind "WindowedApp"
        defines { "MCCTP_EXAMPLE_RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "Off"