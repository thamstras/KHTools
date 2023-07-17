workspace "BBSHash"
    configurations { "Debug", "Release" }
    platforms { "x64" }
    filter "configurations:Debug"
        defines {"DEBUG"} 
        symbols "on" 
        optimize "off"
        runtime "debug"
    filter "configurations:Release"
        defines {"NDEBUG"} 
        optimize "on"
        runtime "release"
        flags {"NoRuntimeChecks", "LinkTimeOptimization"}
    filter({})
    staticruntime "off"
    targetdir "%{prj.location}/bin/%{cfg.buildcfg}/%{cfg.platform}"
    objdir "%{prj.location}/obj/%{cfg.buildcfg}/%{cfg.platform}"

    project "BBSHash"
        location "src"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"
        system "Windows"
        architecture "x86_64"
        files { "src/**.h", "src/**.c", "src/**.cpp", "src/**.hpp" }
        includedirs {"./src"}
        flags {"NoPCH"}