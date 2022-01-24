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

project "WordleSolver"
	kind "ConsoleApp"
	location "build/WordleSolver"
	language "C++"
	editandcontinue "On"
	debugdir "$(TargetDir)"
	files { 
		".editorconfig",
		"data/**",
		"src/**"
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

dofile("third_party/eastl.lua")