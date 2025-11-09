------------------------------------------------------------------------------
-- Utils
------------------------------------------------------------------------------
local function local_require(path)
	return dofile(path)
end

local function this_directory()
    local str = debug.getinfo(2, "S").source:sub(2)
	local path = str:match("(.*/)")
    return path:gsub("\\", "/") -- Replace \\ with /
end

function append_to_table(dest, value)
	if type(value) == "table" then
		for _, v in ipairs(value) do
        	table.insert(dest, v)
    	end
    else
		table.insert(dest, value)
    end

	return dest
end

function remove_from_table(dest, filter)
    for i = #dest, 1, -1 do  -- Iterate backwards
        local value = dest[i]

		-- Note: Allows lua patterns
        if string.find(value, filter) ~= nil then
            table.remove(dest, i)
        end
    end

	return dest
end

function copy_table(tbl)
    if type(tbl) ~= "table" then 
		return tbl 
	end

    local copy = {}
    for k, v in pairs(tbl) do
        copy[k] = copy_table(v)
    end
	
    return copy
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
	-- Internal Dependencies
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
	},

	-- Export Dependencies (Note: Makes using as submodule easier.)
	Obsidian =
	{
		IncludeDir = {},
		LibName = {},
		LibDir = {},
		PostBuildCommands = {},
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
-- Export Dependencies
------------------------------------------------------------------------------
-- IncludeDirs
append_to_table(Dependencies.Obsidian.IncludeDir, this_directory() .. "Obsidian/src")
append_to_table(Dependencies.Obsidian.IncludeDir, Dependencies.GLFW.IncludeDir)
append_to_table(Dependencies.Obsidian.IncludeDir, Dependencies.glm.IncludeDir)
append_to_table(Dependencies.Obsidian.IncludeDir, Dependencies.Tracy.IncludeDir)
append_to_table(Dependencies.Obsidian.IncludeDir, Dependencies.Nano.IncludeDir)
append_to_table(Dependencies.Obsidian.IncludeDir, Dependencies.shaderc.IncludeDir)
append_to_table(Dependencies.Obsidian.IncludeDir, Dependencies.SPIRVCross.IncludeDir)
append_to_table(Dependencies.Obsidian.IncludeDir, Dependencies.DX12.IncludeDir)
append_to_table(Dependencies.Obsidian.IncludeDir, Dependencies.D3D12MA.IncludeDir)
append_to_table(Dependencies.Obsidian.IncludeDir, Dependencies.DXC.IncludeDir)

if OBSIDIAN_GRAPHICS_API == "vulkan" then
	append_to_table(Dependencies.Obsidian.IncludeDir, Dependencies.Vulkan.IncludeDir)
elseif OBSIDIAN_GRAPHICS_API == "dx12" then
	append_to_table(Dependencies.Obsidian.IncludeDir, Dependencies.DX12.IncludeDir)
	append_to_table(Dependencies.Obsidian.IncludeDir, Dependencies.D3D12MA.IncludeDir)
	append_to_table(Dependencies.Obsidian.IncludeDir, Dependencies.DXC.IncludeDir)
elseif OBSIDIAN_GRAPHICS_API == "metal" then
	-- ...
end

-- LibNames
append_to_table(Dependencies.Obsidian.LibName, "Obsidian")

if os.target() == "linux" then
	append_to_table(Dependencies.Obsidian.LibName, Dependencies.GLFW.LibName)
	append_to_table(Dependencies.Obsidian.LibName, Dependencies.Tracy.LibName)
	append_to_table(Dependencies.Obsidian.LibName, Dependencies.shaderc.LibName)
	append_to_table(Dependencies.Obsidian.LibName, Dependencies.SPIRVCross.LibName)

	if OBSIDIAN_GRAPHICS_API == "vulkan" then
		links(Dependencies.Vulkan.LibName)
	end
end


-- LibDir
if OBSIDIAN_GRAPHICS_API == "vulkan" then
	append_to_table(Dependencies.Obsidian.LibDir, Dependencies.Vulkan.LibDir)
end

-- PostBuildCommands
if os.target() == "macosx" then
	append_to_table(Dependencies.Obsidian.PostBuildCommands, '{COPYFILE} "' .. Dependencies.Vulkan.LibDir .. '/libvulkan.1.dylib" "%{cfg.targetdir}"')
	append_to_table(Dependencies.Obsidian.PostBuildCommands, '{COPYFILE} "' .. Dependencies.Vulkan.LibDir .. '/lib' .. Dependencies.Vulkan.LibName .. '.dylib" "%{cfg.targetdir}"')
end
------------------------------------------------------------------------------

return Dependencies
