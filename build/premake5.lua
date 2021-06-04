workspace "sdl-iso-project"
	configurations { "Debug", "Release" }

project "Iso"
	local LIBS = { "SDL2", "SDL2_image", "json-c" }

	if os.target() == "windows" then
		-- there is a bug in mingw32-make which sets CC=cc by default. IDK what cc
		-- is, but it's not a compiler
		makesettings "CC=gcc"
	end

	kind "WindowedApp"
	language "C"
	cdialect "C99"

	files { "../src/**.c", "../src/**.h" }
	targetdir ".."

	links { "m", table.unpack(LIBS) }

	floatingpoint "Fast"
	enablewarnings { "all" }

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "RELEASE" }
		optimize "On"

