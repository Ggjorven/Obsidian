------------------------------------------------------------------------------
-- Utils
------------------------------------------------------------------------------
function local_require(path)
	return dofile(path)
end

function this_directory()
    local str = debug.getinfo(2, "S").source:sub(2)
	local path = str:match("(.*/)")
    return path:gsub("\\", "/") -- Replace \\ with /
end
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Graphics API
------------------------------------------------------------------------------
OBSIDIAN_GRAPHICS_API = OBSIDIAN_GRAPHICS_API or "vulkan" -- This is global
local VULKAN_SDK = nil
local VULKAN_VERSION = nil

-- Vulkan
if OBSIDIAN_GRAPHICS_API == "vulkan" then
	VULKAN_SDK = os.getenv("VULKAN_SDK")

	if not VULKAN_SDK or VULKAN_SDK == "" then
		error("VULKAN_SDK environment variable is not set. Please make sure it properly installed.")
	end

	VULKAN_VERSION = VULKAN_SDK:match("(%d+%.%d+%.%d+)") -- Example: 1.3.290 (without the .0)

-- DirectX12
elseif OBSIDIAN_GRAPHICS_API == "dx12" then
	if os.target() ~= "windows" then
		error("The DirectX12 Graphics API is not supported on the current platform.")
	end

-- Metal
elseif OBSIDIAN_GRAPHICS_API == "metal" then
	if os.target() ~= "macosx" then
		error("The Metal Graphics API is not supported on the current platform.")
	end
	
	error("Metal is currently not supported.")
end
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Dependencies
------------------------------------------------------------------------------
local Dependencies =
{
	GLFW =
	{
		LibName = "GLFW",
		IncludeDir = this_directory() .. "/vendor/GLFW/GLFW/include"
	},
	glm =
	{
		IncludeDir = this_directory() .. "/vendor/glm/glm"
	},
	Tracy = 
	{
		LibName = "Tracy",
		IncludeDir = this_directory() .. "/vendor/tracy/tracy/public"
	},
	Nano = 
	{
		IncludeDir = this_directory() .. "/vendor/Nano/Nano/Nano/include"
	},

	shaderc = 
	{
		LibName = "shaderc",
		IncludeDir = this_directory() .. "/vendor/shaderc/ShaderCompiler/ShaderCompiler/include"
	},
	SPIRVCross = 
	{
		LibName = "SPIRVCross",
		IncludeDir = this_directory() .. "/vendor/SPIRV-Cross/SPIRV-Cross" 
	},

	DX12 = 
	{
		IncludeDir = this_directory() .. "/vendor/DirectX/DirectX-Headers/include"
	},
	D3D12MA = 
	{
		LibName = "D3D12MA",
		IncludeDir = this_directory() .. "/vendor/DirectX/D3D12MA/include"
	},
	DXC = 
	{
		LibName = this_directory() .. "/vendor/DirectX/DXC/lib/dxcompiler",
		IncludeDir = this_directory() .. "/vendor/DirectX/DXC/include",
	}
}
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Platform specific
------------------------------------------------------------------------------
if OBSIDIAN_GRAPHICS_API == "vulkan" then
	if os.target() == "windows" then
		Dependencies.Vulkan =
		{
			LibName = "vulkan-1",
			IncludeDir = VULKAN_SDK .. "/Include/",
			LibDir = VULKAN_SDK .. "/Lib/"
		}

	elseif os.target() == "linux" then
		Dependencies.Vulkan =
		{
			LibName = "vulkan",
			IncludeDir = VULKAN_SDK .. "/include/",
			LibDir = VULKAN_SDK .. "/lib/"
		}

	elseif os.target() == "macosx" then
		Dependencies.Vulkan = -- Note: Vulkan on MacOS is currently dynamic. (Example: libvulkan1.3.290.dylib)
		{
			LibName = "vulkan." .. VULKAN_VERSION,
			IncludeDir = VULKAN_SDK .. "/../macOS/include/",
			LibDir = VULKAN_SDK .. "/../macOS/lib/",
		}
	else
		error("Failed to initialize Vulkan headers for current platform.")
	end
end
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Merge
------------------------------------------------------------------------------
Dependencies.Combined =
{
    IncludeDir = {},
    LibName = {},
    LibDir = {}
}

for name, dep in pairs(Dependencies) do
    if name ~= "Combined" then
        -- IncludeDirs
        if dep.IncludeDir then
            table.insert(Dependencies.Combined.IncludeDir, dep.IncludeDir)
        end
        
        -- LibNames
        if dep.LibName then
            table.insert(Dependencies.Combined.LibName, dep.LibName)
        end

        -- LibDirs
        if dep.LibDir then
            table.insert(Dependencies.Combined.LibDir, dep.LibDir)
        end
    end
end
------------------------------------------------------------------------------

return Dependencies