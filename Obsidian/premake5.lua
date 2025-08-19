local Dependencies = local_require("../Dependencies.lua")
local MacOSVersion = MacOSVersion or "14.5"
local OutputDir = OutputDir or "%{cfg.buildcfg}-%{cfg.system}"

project "Obsidian"
	kind "StaticLib"
	language "C++"
	cppdialect "C++23"
	staticruntime "On"

	architecture "x86_64"

	warnings "Extra"

	targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. OutputDir .. "/%{prj.name}")

	-- Note: VS2022/Make only need the pchheader filename
	pchheader "obpch.h"
	pchsource "src/Obsidian/obpch.cpp"

	files
	{
		"src/Obsidian/**.h",
		"src/Obsidian/**.hpp",
		"src/Obsidian/**.inl",
		"src/Obsidian/**.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS",

		"GLFW_INCLUDE_NONE",

		"NANO_EXPERIMENTAL",
	}

	-- Rendering API specfic selections
	if OBSIDIAN_GRAPHICS_API == "vulkan" then
        defines { "OB_API_VULKAN" }
		removefiles { "src/Obsidian/Platform/DX12/**", "src/Obsidian/Platform/Metal/**", "src/Obsidian/Platform/Dummy/**" }
    elseif OBSIDIAN_GRAPHICS_API == "dx12" then
        defines { "OB_API_DX12" }
		removefiles { "src/Obsidian/Platform/Vulkan/**", "src/Obsidian/Platform/Metal/**", "src/Obsidian/Platform/Dummy/**" }
	elseif OBSIDIAN_GRAPHICS_API == "metal" then
        defines { "OB_API_METAL" }
		removefiles { "src/Obsidian/Platform/Vulkan/**", "src/Obsidian/Platform/DX12/**", "src/Obsidian/Platform/Dummy/**" }
	elseif OBSIDIAN_GRAPHICS_API == "dummy" then
        defines { "OB_API_DUMMY" }
		removefiles { "src/Obsidian/Platform/Vulkan/**", "src/Obsidian/Platform/DX12/**", "src/Obsidian/Platform/Metal/**" }
    end

	includedirs
	{
		"src",
		"src/Obsidian",
	}

	includedirs(Dependencies.Obsidian.IncludeDir)

	links(Dependencies.GLFW.LibName)
	links(Dependencies.Tracy.LibName)
	links(Dependencies.shaderc.LibName)
	links(Dependencies.SPIRVCross.LibName)
 
	filter "system:windows"
		systemversion "latest"
		staticruntime "on"
		editandcontinue "off"

        defines
        {
            "NOMINMAX"
        }

		if OBSIDIAN_GRAPHICS_API == "vulkan" then
			links(Dependencies.Vulkan.LibDir .. "/" .. Dependencies.Vulkan.LibName)
		elseif OBSIDIAN_GRAPHICS_API == "dx12" then
			links
			{
				"d3d12",
				"dxgi",
				"dxguid",
				"dxcompiler",
			}

			links(Dependencies.D3D12MA.LibName)
			links(Dependencies.DXC.LibName)
		end

	filter "system:linux"
		systemversion "latest"
		staticruntime "on"

		if gfxapi == "vulkan" then
			links(Dependencies.Vulkan.LibDir .. "/" .. Dependencies.Vulkan.LibName)
		end
		
		links
		{
			"Xrandr", "Xi", "GLU", "GL", "GLX", "X11", "dl", "pthread", "stdc++fs"
		}

    filter "system:macosx"
		systemversion(MacOSVersion)
		staticruntime "on"

	filter "action:vs*"
		buildoptions { "/Zc:preprocessor" }

	filter "action:xcode*"
		-- Note: XCode only needs the full pchheader path
		pchheader "src/Obsidian/obpch.h"

		-- Note: If we don't add the header files to the externalincludedirs
		-- we can't use <angled> brackets to include files.
		externalincludedirs(includedirs())

	filter "configurations:Debug"
		defines "OB_CONFIG_DEBUG"
		defines "NANO_DEBUG"
		runtime "Debug"
		symbols "on"
		
        defines
        {
			"TRACY_ENABLE"
        }
		
	filter "configurations:Release"
		defines "OB_CONFIG_RELEASE"
		runtime "Release"
		optimize "on"

        defines
        {
            "TRACY_ENABLE"
        }

	filter "configurations:Dist"
		defines "OB_CONFIG_DIST"
		runtime "Release"
		optimize "Full"
		linktimeoptimization "on"
