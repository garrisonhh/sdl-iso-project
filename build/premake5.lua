workspace "sdl-iso-project"
	configurations { "debug", "release" }

include "../external/ghh-lib"

project "iso"
	if os.target() == "windows" then
		-- there is a bug in mingw32-make which sets CC=cc by default
		makesettings "CC=gcc"
	end

	kind "ConsoleApp"
	language "C"
	cdialect "c99"
	targetdir ".."

	enablewarnings { "all" }
	floatingpoint "Fast"

	links {
		"m", "SDL2", "SDL2_image", "json-c", "ghh"
	}

	includedirs {
		"../external/**/include/"
	}

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
		defines { "NDEBUG" }
		optimize "On"
