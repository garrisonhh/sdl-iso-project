workspace "sdl-iso-project"
	configurations { "debug", "release" }

project "iso"
	if os.target() == "windows" then
		-- there is a bug in mingw32-make which sets CC=cc by default. IDK what cc
		-- is, but it's not a compiler
		makesettings "CC=gcc"
	end

	kind "ConsoleApp"
	language "C"
	cdialect "gnu99"
	targetdir ".."

	enablewarnings { "all" }
	floatingpoint "Fast"

	links { "m", "SDL2", "SDL2_image", "json-c", "gcc" }

	files {
		"../src/**.c",
		"../src/**.h"
	}

	defines {
		"BASE_DIRECTORY=\".\""
	}

	filter "configurations:debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:release"
		defines { "RELEASE" }
		optimize "On"
