project "test_patch"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    targetdir "bin/%{cfg.buildcfg}"
    staticruntime "on"

    files 
    {
        "premake5.lua",
        "**.cpp",
    }

    includedirs
    {
        ".",
        "../%{IncludeDir.mcctp}",
        "../%{IncludeDir.imgui}",
        "../%{IncludeDir.fpng}",
        "../%{IncludeDir.GLEW}",
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

    defines
    {
        "GLEW_STATIC"
    }

    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"
        defines { "MCCTP_TEST_PATCH_PLATFORM_WINDOWS" }

    filter "configurations:Debug"
        defines { "MCCTP_TEST_PATCH_DEBUG" }
        runtime "Debug"
        optimize "Off"
        symbols "On"

    filter "configurations:Release"
        defines { "MCCTP_TEST_PATCH_RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "Off"