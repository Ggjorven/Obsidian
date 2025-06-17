------------------------------------------------------------------------------
-- Rendering API selection
------------------------------------------------------------------------------
newoption 
{
    trigger     = "gfxapi",
    value       = "API",
    description = "Choose a graphics API",
    allowed = 
	{
        { "vulkan", "Vulkan" },
        { "d3d12", "D3D12", "D3d12", "dx12", "DX12", "Dx12" },
        { "metal", "Metal", "mtl" },
        { "dummy", "Dummy", "headless", "Headless" },
    }
}

------------------------------------------------------------------------------
-- Utilities
------------------------------------------------------------------------------
local function GetIOResult(cmd)
	local handle = io.popen(cmd) -- Open a console and execute the command.
	local output = handle:read("*a") -- Read the output.
	handle:close() -- Close the handle.

	return output:match("^%s*(.-)%s*$") -- Trim any trailing whitespace (such as newlines)
end

function GetOS()
	local osName = os.getenv("OS")

	if osName == "Windows_NT" then
		return "windows"
	else
		local uname = io.popen("uname"):read("*l")
		if uname == "Linux" then
			return "linux"
		elseif uname == "Darwin" then
			return "macosx"
		end
	end

	return "unknown-os"
end
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Bug fixes
------------------------------------------------------------------------------
-- Visual Studio: Bugfix for C++ Modules (same module file name per project)
-- https://github.com/premake/premake-core/issues/2177
require("vstudio")
premake.override(premake.vstudio.vc2010.elements, "clCompile", function(base, prj)
    local m = premake.vstudio.vc2010
    local calls = base(prj)

    if premake.project.iscpp(prj) then
		table.insertafter(calls, premake.xmlDeclaration,  function()
			premake.w('<ModuleDependenciesFile>$(IntDir)\\%%(RelativeDir)</ModuleDependenciesFile>')
			premake.w('<ModuleOutputFile>$(IntDir)\\%%(RelativeDir)</ModuleOutputFile>')
			premake.w('<ObjectFileName>$(IntDir)\\%%(RelativeDir)</ObjectFileName>')
		end)
    end

    return calls
end)
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Dependencies
------------------------------------------------------------------------------
gfxapi = _OPTIONS["gfxapi"] or "vulkan"
VULKAN_SDK = nil
VULKAN_VERSION = nil

if gfxapi == "vulkan" then
	VULKAN_SDK = os.getenv("VULKAN_SDK")

	if not VULKAN_SDK or VULKAN_SDK == "" then
		error("VULKAN_SDK environment variable is not set. Please make sure it properly installed.")
	end

	VULKAN_VERSION = VULKAN_SDK:match("(%d+%.%d+%.%d+)") -- Example: 1.3.290 (without the 0)
elseif gfx == "d3d12" then
	-- error("D3D12 is currently not supported.")
elseif gfx == "metal" then
	-- error("Metal is currently not supported.")
end

MacOSVersion = "14.5"

Dependencies =
{
	GLFW =
	{
		LibName = "GLFW",
		IncludeDir = "%{wks.location}/vendor/GLFW/GLFW/include"
	},
	glm =
	{
		IncludeDir = "%{wks.location}/vendor/glm/glm"
	},
	stb = 
	{
		IncludeDir = "%{wks.location}/vendor/stb/include"
	},
	Tracy = 
	{
		LibName = "Tracy",
		IncludeDir = "%{wks.location}/vendor/tracy/tracy/public"
	},
	Nano = 
	{
		IncludeDir = "%{wks.location}/vendor/Nano/Nano/Nano/include"
	}
}

------------------------------------------------------------------------------
-- Platform specific
------------------------------------------------------------------------------
if gfxapi == "vulkan" then -- TODO: Use Vulkan-Headers
	if GetOS() == "windows" then
		Dependencies.Vulkan =
		{
			LibName = "vulkan-1",
			IncludeDir = "%{VULKAN_SDK}/Include/",
			LibDir = "%{VULKAN_SDK}/Lib/"
		}
		Dependencies.ShaderC = { LibName = "shaderc_shared" }

	elseif GetOS() == "linux" then
		Dependencies.Vulkan =
		{
			LibName = "vulkan",
			IncludeDir = "%{VULKAN_SDK}/include/",
			LibDir = "%{VULKAN_SDK}/lib/"
		}
		Dependencies.ShaderC = { LibName = "shaderc_shared" }

	elseif GetOS() == "macosx" then
		Dependencies.Vulkan = -- Note: Vulkan on MacOS is currently dynamic. (Example: libvulkan1.3.290.dylib)
		{
			LibName = "vulkan.%{VULKAN_VERSION}",
			IncludeDir = "%{VULKAN_SDK}/../macOS/include/",
			LibDir = "%{VULKAN_SDK}/../macOS/lib/",
		}
		Dependencies.ShaderC = { LibName = "shaderc_combined" }
	end
end
------------------------------------------------------------------------------
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Solution
------------------------------------------------------------------------------
outputdir = "%{cfg.buildcfg}-%{cfg.system}"

workspace "NanoGraphics"
	architecture "x86_64"
	startproject "Sandbox"

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
	include "vendor/tracy"
group ""

group "NanoGraphics"
	include "Graphics"
group ""

include "Sandbox"
------------------------------------------------------------------------------