workspace "WordleSolver"
	location "build"
	platforms { "Win64" }
	configurations { "Debug", "Release" }
	filter { "platforms:Win64" }
		system "Windows"
		architecture "x86_64"
	filter{}

	filter { "configurations:Debug"}

    filter { "configurations:Release" }
   		optimize "Full"
		flags "NoRuntimeChecks"
		inlining "Auto"
		intrinsics "on"
	filter{}

function describeWordle()
	language "C++"
	editandcontinue "On"
	debugdir "$(TargetDir)"
	files { 
		"src/**.h"
	}
	postbuildcommands {
		'robocopy "' .. path.getabsolute("data") .. '" "$(TargetDir)data" /E',
		'exit /b 0'
	}
	removefiles {
		"src/Platform/SDL/**",
	}
	includedirs {
		"src",
		"src/**",
		"third_party/EASTL/include",
		"third_party/EASTL/test/packages/EABase/include/Common",
	}
	links {
		"EASTL",
	}
	cppdialect "C++latest"
	vpaths
	{
		["*"] = "src",
		["data/*"] = "data/*",
		["data/*"] = "data/common/**",
		["data/platform/*"] = "data/console/**",
	}
	flags { "MultiProcessorCompile" }
end

project "WordleSolver"
	kind "ConsoleApp"
	location "build/WordleSolver"
	files { 
		"src/main.cpp",
		"src/wordleSolver.cpp",
	}
	describeWordle()
	
project "WordleSolverDLL"
	kind "SharedLib"
	location "build/WordleSolver"
	removefiles { "src/main.cpp" }
	files { 
		"src/dllmain.cpp",
		"src/wordleSolver.cpp",
	}
	describeWordle()

dofile("third_party/eastl.lua")
