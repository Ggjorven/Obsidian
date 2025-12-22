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
