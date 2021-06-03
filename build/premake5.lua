workspace "sdl-iso-project"
	configurations { "Debug", "Release" }

project "Iso"
	kind "WindowedApp"
	language "C"
	cdialect "gnu99"
	targetdir ".."

	files { "../src/**.c", "../src/**.h" }

	floatingpoint "Fast"
	enablewarnings { "all" }

	-- libdirs {}
	links { "m", "SDL2", "SDL2_image", "json-c" }

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "RELEASE" }
		optimize "On"

