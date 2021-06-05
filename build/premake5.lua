workspace "sdl-iso-project"
	configurations { "Debug", "Release" }

project "Iso"
	local BASEDIR = ".."

	if os.target() == "windows" then
		-- there is a bug in mingw32-make which sets CC=cc by default. IDK what cc
		-- is, but it's not a compiler
		makesettings "CC=gcc"
	end

	kind "WindowedApp"
	language "C"
	cdialect "C99"
	targetdir ".."

	enablewarnings { "all" }
	floatingpoint "Fast"

	links { "m", "SDL2", "SDL2_image", "json-c" }

	files {
		BASEDIR .. "/src/**.c",
		BASEDIR .. "/src/**.h"
	}

	defines {
		"BASE_DIRECTORY=\"" .. BASEDIR .. "\""
	}

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "RELEASE" }
		optimize "On"

