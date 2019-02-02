workspace "Aberration"
	architecture "x64"
	configurations { "Debug", "Release", "Distrib"}
	platforms { "Windows", "Linux" }
	startproject "Aberration"
	characterset "MBCS"
	exceptionhandling "Off"
	rtti "Off"
	floatingpoint "Fast"
	functionlevellinking "On"
	language "C++"
	cppdialect "C++17"
	systemversion "latest"
	--toolset "clang"


	flags {
		"FatalCompileWarnings", 
		"MultiProcessorCompile"
	}

buildDir = "%{cfg.buildcfg}-%{cfg.system}"

--filter "toolset:clang"
	--toolset "clang"


filter { "configurations:Debug" }
	inlining "Disabled"
	symbols "on"
	optimize "off"
	defines {
		"AB_CONFIG_DEBUG"
	}

filter { "configurations:Release" }
	inlining "Auto"
	optimize "full"
	symbols "off"
	defines {
		"AB_CONFIG_RELEASE"
	}
	flags { "LinkTimeOptimization" }

filter { "configurations:Distrib" }
	inlining "Auto"
	optimize "full"
	symbols "off"
	defines {
		"AB_CONFIG_DISTRIB"
	}
	flags { "LinkTimeOptimization" }

filter { "platforms:Windows" }
	system "windows"
	defines {
		"AB_PLATFORM_WINDOWS"
	}

filter { "platforms:Linux" }
	system "linux"
	defines {
		"AB_PLATFORM_LINUX"
	}

binDir = "build/bin/%{buildDir}"

project "Aberration"
	location "aberration"
	kind "ConsoleApp"
	staticruntime "off"
	--symbolspath "%{binDir}/$(DATE).pdb"
	--linkoptions { "/PDBALTPATH:%DATE%.pdb", "/PDB:%DATE%.pdb" }
	
	targetdir ("%{binDir}")
	objdir ("build/obj/%{binDir}")

	defines {
		"_CRT_SECURE_NO_WARNINGS"
	}

	--pchheader "src/ABHeader.h"
	--pchsource "aberration/src/ABHeader.cpp"
	
	files {
		"aberration/Aberration.h",
		"aberration/src/**.h",
		"aberration/src/**.cpp"
	}

	removefiles { "**/windows/**", "**/unix/**" }

	includedirs {
		"aberration/src",
		"aberration/",
		"hypermath/"
	}

	links {

	}

	filter { "system:windows" }
		defines {
			"WIN32_LEAN_AND_MEAN"
		}

		links {
			"OpenGL32",
			"gdi32"
		}

		files {
			"aberration/src/platform/windows/**.h",
			"aberration/src/platform/windows/**.cpp"
		}

	filter { "system:linux" }
		links {
			"X11",
			"GL",
			"dl"
		}

		files {
			"aberration/src/platform/unix/**.h",
			"aberration/src/platform/unix/**.cpp",
		}

project "Sandbox"
	location "sandbox"
	kind "SharedLib"
	staticruntime "off"

	targetdir ("build/bin/%{buildDir}")
	objdir ("build/obj/%{buildDir}")

	defines {
		
	}

	files {
		"sandbox/src/**.h",
		"sandbox/src/**.cpp",
	}

	includedirs {
		"aberration/",
		"hypermath/"
	}

	flags {
		"NoImportLib",
		"NoImplicitLink"
	}