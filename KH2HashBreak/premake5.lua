workspace "KH2HashBreak" 
    configurations {"Debug", "Release"} 
    platforms {"Win32"}

    project "KH2HashBreak"
        kind "ConsoleApp"
        language "C++"
		cppdialect "C++17"  -- Needed for proper <filesystem> support.
        system "Windows" 
        architecture "x32"
		vpaths {
			["Source"] = { "**.h", "**.c", "**.hpp", "**.cpp"},
		}
		files { "HashBreak/*.c", "HashBreak/*.cpp", "HashBreak/*.h", "HashBreak/*.hpp" }
        flags {"NoPCH"}  -- TODO: Check to see if more flags are needed.
        filter "configurations:Debug"
            defines {"DEBUG"} 
            symbols "on" 
            optimize "off" 

        filter "configurations:Release"
            defines {"NDEBUG"} 
            optimize "on" 
			
	project "KH2HashGather"
        kind "ConsoleApp"
        language "C++"
		cppdialect "C++17"  -- Needed for proper <filesystem> support.
        system "Windows" 
        architecture "x32"
		vpaths {
			["Source"] = { "**.h", "**.c", "**.hpp", "**.cpp"},
		}
		files { "HashGather/*.c", "HashGather/*.cpp", "HashGather/*.h", "HashGather/*.hpp" }
        flags {"NoPCH"}  -- TODO: Check to see if more flags are needed.
        filter "configurations:Debug"
            defines {"DEBUG"} 
            symbols "on" 
            optimize "off" 

        filter "configurations:Release"
            defines {"NDEBUG"} 
            optimize "on" 