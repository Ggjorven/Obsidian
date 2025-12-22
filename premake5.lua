------------------------------------------------------------------------------
-- Custom options
------------------------------------------------------------------------------
newoption 
{
    trigger     = "dm",
    value       = "Display Manager",
    description = "Choose the display manager for linux (wayland or x11)",
    allowed = 
	{
        { "wayland", "Wayland" },
        { "x11", "X11" },
    }
}

if os.target() == "linux" then
	if _OPTIONS["dm"] then
		OBSIDIAN_DISPLAY_MANAGER = _OPTIONS["dm"]
	else
		if os.getenv("WAYLAND_DISPLAY") then
			OBSIDIAN_DISPLAY_MANAGER = "wayland"
		else
			OBSIDIAN_DISPLAY_MANAGER = "x11"
		end
	end
end
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Utils
------------------------------------------------------------------------------
function local_require(path)
	return dofile(path)
end
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Solution
------------------------------------------------------------------------------
MacOSVersion = "14.5"
OutputDir = "%{cfg.buildcfg}-%{cfg.system}"

workspace "Obsidian"
	architecture "x86_64"
	startproject "Main"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile"
	}

group "Dependencies"
	include "vendor/GLFW"
	include "vendor/shaderc"
group ""

include "Main"
------------------------------------------------------------------------------
