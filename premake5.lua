------------------------------------------------------------------------------
-- Graphics API option
------------------------------------------------------------------------------
newoption 
{
    trigger     = "gfxapi",
    value       = "API",
    description = "Choose a graphics API",
    allowed = 
	{
        { "vulkan", "Vulkan graphics API (windows, linux, macosx)" },
        { "d3d12", "DirectX 12 (windows)" },
        { "metal", "Metal (macosx)" },
        { "dummy", "No GraphicsAPI, passthrough function calls. (windows, linux, macosx)" },
    }
}
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Utilities
------------------------------------------------------------------------------
local function GetIOResult(cmd)
	local handle = io.popen(cmd) -- Open a console and execute the command.
	local output = handle:read("*a") -- Read the output.
	handle:close() -- Close the handle.

	return output:match("^%s*(.-)%s*$") -- Trim any trailing whitespace (such as newlines)
end

platform = os.target()
gfxapi = _OPTIONS["gfxapi"] or "vulkan"
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
-- Graphics API
------------------------------------------------------------------------------
VULKAN_SDK = nil
VULKAN_VERSION = nil

-- Vulkan
if gfxapi == "vulkan" then
	VULKAN_SDK = os.getenv("VULKAN_SDK")

	if not VULKAN_SDK or VULKAN_SDK == "" then
		error("VULKAN_SDK environment variable is not set. Please make sure it properly installed.")
	end

	VULKAN_VERSION = VULKAN_SDK:match("(%d+%.%d+%.%d+)") -- Example: 1.3.290 (without the 0)

-- DirectX12
elseif gfxapi == "d3d12" then
	if platform ~= "windows" then
		error("The DirectX12 Graphics API is not supported on the current platform.")
	end
	
	error("D3D12 is currently not supported.")

-- Metal
elseif gfxapi == "metal" then
	if platform ~= "macosx" then
		error("The Metal Graphics API is not supported on the current platform.")
	end
	
	error("Metal is currently not supported.")
end
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Dependencies
------------------------------------------------------------------------------
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
if gfxapi == "vulkan" then
	if platform == "windows" then
		Dependencies.Vulkan =
		{
			LibName = "vulkan-1",
			IncludeDir = "%{VULKAN_SDK}/Include/",
			LibDir = "%{VULKAN_SDK}/Lib/"
		}
		Dependencies.ShaderC = { LibName = "shaderc_shared" }

	elseif platform == "linux" then
		Dependencies.Vulkan =
		{
			LibName = "vulkan",
			IncludeDir = "%{VULKAN_SDK}/include/",
			LibDir = "%{VULKAN_SDK}/lib/"
		}
		Dependencies.ShaderC = { LibName = "shaderc_shared" }

	elseif platform == "macosx" then
		Dependencies.Vulkan = -- Note: Vulkan on MacOS is currently dynamic. (Example: libvulkan1.3.290.dylib)
		{
			LibName = "vulkan.%{VULKAN_VERSION}",
			IncludeDir = "%{VULKAN_SDK}/../macOS/include/",
			LibDir = "%{VULKAN_SDK}/../macOS/lib/",
		}
		Dependencies.ShaderC = { LibName = "shaderc_combined" }
	else
		error("Failed to initialize Vulkan headers for current platform.")
	end
end
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