project "mcctp"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir "bin/%{cfg.buildcfg}"
    staticruntime "on"

    files 
    {
        "premake5.lua",
        "**.h",
        "**.hpp",
        "**.cpp"
    }

    includedirs
    {
        ".",
        "%{IncludeDir.fpng}",
        "%{IncludeDir.GLEW}",
    }

    links
    {
        "fpng",
        "GLEW"
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
        defines { "MCCTP_PLATFORM_WINDOWS" }

    filter "configurations:Debug"
        defines { "MCCTP_DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "MCCTP_RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"