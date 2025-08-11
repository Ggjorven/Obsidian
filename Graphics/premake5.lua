local Dependencies = local_require("../Dependencies.lua")
local MacOSVersion = MacOSVersion or "14.5"
local OutputDir = OutputDir or "%{cfg.buildcfg}-%{cfg.system}"

project "Graphics"
	kind "StaticLib"
	language "C++"
	cppdialect "C++23"
	staticruntime "On"

	architecture "x86_64"

	warnings "Extra"

	targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. OutputDir .. "/%{prj.name}")

	-- Note: VS2022/Make only need the pchheader filename
	pchheader "ngpch.h"
	pchsource "src/NanoGraphics/ngpch.cpp"

	files
	{
		"src/NanoGraphics/**.h",
		"src/NanoGraphics/**.hpp",
		"src/NanoGraphics/**.inl",
		"src/NanoGraphics/**.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS",

		"GLFW_INCLUDE_NONE",

		"NANO_EXPERIMENTAL",
	}

	-- Rendering API specfic selections
	if NANOGRAPHICS_GRAPHICS_API == "vulkan" then
        defines { "NG_API_VULKAN" }
		removefiles { "src/NanoGraphics/Platform/DX12/**", "src/NanoGraphics/Platform/Metal/**", "src/NanoGraphics/Platform/Dummy/**" }
		includedirs(Dependencies.Vulkan.IncludeDir)
    elseif NANOGRAPHICS_GRAPHICS_API == "dx12" then
        defines { "NG_API_DX12" }
		removefiles { "src/NanoGraphics/Platform/Vulkan/**", "src/NanoGraphics/Platform/Metal/**", "src/NanoGraphics/Platform/Dummy/**" }
		includedirs(Dependencies.DX12.IncludeDir)
		includedirs(Dependencies.D3D12MA.IncludeDir)
		includedirs(Dependencies.DXC.IncludeDir)
	elseif NANOGRAPHICS_GRAPHICS_API == "metal" then
        defines { "NG_API_METAL" }
		removefiles { "src/NanoGraphics/Platform/Vulkan/**", "src/NanoGraphics/Platform/DX12/**", "src/NanoGraphics/Platform/Dummy/**" }
	elseif NANOGRAPHICS_GRAPHICS_API == "dummy" then
        defines { "NG_API_DUMMY" }
		removefiles { "src/NanoGraphics/Platform/Vulkan/**", "src/NanoGraphics/Platform/DX12/**", "src/NanoGraphics/Platform/Metal/**" }
    end

	includedirs
	{
		"src",
		"src/NanoGraphics",
	}

	includedirs(Dependencies.GLFW.IncludeDir)
	includedirs(Dependencies.glm.IncludeDir)
	includedirs(Dependencies.Tracy.IncludeDir)
	includedirs(Dependencies.Nano.IncludeDir)
	includedirs(Dependencies.shaderc.IncludeDir)
	includedirs(Dependencies.SPIRVCross.IncludeDir)

	links(Dependencies.GLFW.LibName)
	links(Dependencies.Tracy.LibName)
	links(Dependencies.shaderc.LibName)
	links(Dependencies.SPIRVCross.LibName)
 
	filter "system:windows"
		defines "NG_PLATFORM_DESKTOP"
		defines "NG_PLATFORM_WINDOWS"
		defines "NG_PLATFORM_UNIX"
		systemversion "latest"
		staticruntime "on"
		editandcontinue "off"

        defines
        {
            "NOMINMAX"
        }

		if NANOGRAPHICS_GRAPHICS_API == "vulkan" then
			links(Dependencies.Vulkan.LibDir .. "/" .. Dependencies.Vulkan.LibName)
		elseif NANOGRAPHICS_GRAPHICS_API == "dx12" then
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
		defines "NG_PLATFORM_DESKTOP"
		defines "NG_PLATFORM_LINUX"
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
		defines "NG_PLATFORM_DESKTOP"
		defines "NG_PLATFORM_MACOS"
		defines "NG_PLATFORM_UNIX"
		defines "NG_PLATFORM_APPLE"
		systemversion(MacOSVersion)
		staticruntime "on"

	filter "action:vs*"
		buildoptions { "/Zc:preprocessor" }

	filter "action:xcode*"
		-- Note: XCode only needs the full pchheader path
		pchheader "src/NanoGraphics/ngpch.h"

		-- Note: If we don't add the header files to the externalincludedirs
		-- we can't use <angled> brackets to include files.
		externalincludedirs(includedirs())

	filter "configurations:Debug"
		defines "NG_CONFIG_DEBUG"
		defines "NANO_DEBUG"
		runtime "Debug"
		symbols "on"
		
        defines
        {
			"TRACY_ENABLE"
        }
		
	filter "configurations:Release"
		defines "NG_CONFIG_RELEASE"
		runtime "Release"
		optimize "on"

        defines
        {
            "TRACY_ENABLE"
        }

	filter "configurations:Dist"
		defines "NG_CONFIG_DIST"
		runtime "Release"
		optimize "Full"
		linktimeoptimization "on"
