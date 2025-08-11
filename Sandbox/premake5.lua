local Dependencies = local_require("../Dependencies.lua")
local MacOSVersion = MacOSVersion or "14.5"
local OutputDir = OutputDir or "%{cfg.buildcfg}-%{cfg.system}"

project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++23"
	staticruntime "On"

	debugdir ("%{prj.location}")

	architecture "x86_64"

	warnings "Extra"

	targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. OutputDir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.hpp",
		"src/**.inl",
		"src/**.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS",

		"GLFW_INCLUDE_NONE",

		"NANO_EXPERIMENTAL"
	}

	-- Rendering API specfic selections
	if NANOGRAPHICS_GRAPHICS_API == "vulkan" then
        defines { "NG_API_VULKAN" }
		includedirs(Dependencies.Vulkan.IncludeDir)
    elseif NANOGRAPHICS_GRAPHICS_API == "dx12" then
        defines { "NG_API_DX12" }
		includedirs(Dependencies.DX12.IncludeDir)
		includedirs(Dependencies.D3D12MA.IncludeDir)
		includedirs(Dependencies.DXC.IncludeDir)
	elseif NANOGRAPHICS_GRAPHICS_API == "metal" then
        defines { "NG_API_METAL" }
	elseif NANOGRAPHICS_GRAPHICS_API == "dummy" then
        defines { "NG_API_DUMMY" }
    end

	includedirs
	{
		"src",
		"vendor",

		this_directory() .. "../Graphics/src",
	}

	includedirs(Dependencies.GLFW.IncludeDir)
	includedirs(Dependencies.glm.IncludeDir)
	includedirs(Dependencies.Tracy.IncludeDir)
	includedirs(Dependencies.Nano.IncludeDir)
	includedirs(Dependencies.shaderc.IncludeDir)
	includedirs(Dependencies.SPIRVCross.IncludeDir)

	links
	{
		"Graphics",
	}

	filter "system:windows"
		defines "NG_PLATFORM_DESKTOP"
		defines "NG_PLATFORM_WINDOWS"
		systemversion "latest"
		staticruntime "on"
		editandcontinue "off"

        defines
        {
            "NOMINMAX"
        }

	filter "system:linux"
		defines "NG_PLATFORM_DESKTOP"
		defines "NG_PLATFORM_LINUX"
		defines "NG_PLATFORM_UNIX"
		systemversion "latest"
		staticruntime "on"

		links(Dependencies.GLFW.LibName)
		links(Dependencies.Tracy.LibName)
		links(Dependencies.shaderc.LibName)
		links(Dependencies.SPIRVCross.LibName)

		if NANOGRAPHICS_GRAPHICS_API == "vulkan" then
			links(Dependencies.Vulkan.LibDir .. "/" .. Dependencies.Vulkan.LibName)
		end

    filter "system:macosx"
		defines "NG_PLATFORM_DESKTOP"
		defines "NG_PLATFORM_MACOS"
		defines "NG_PLATFORM_UNIX"
		defines "NG_PLATFORM_APPLE"
		systemversion(MacOSVersion)
		staticruntime "on"

		links
		{
			"AppKit.framework",
			"IOKit.framework",
			"CoreGraphics.framework",
			"CoreFoundation.framework",
			"QuartzCore.framework",
		}

		if gfxapi == "vulkan" then
			libdirs(Dependencies.Vulkan.LibDir)
			links(Dependencies.Vulkan.LibName)

			postbuildcommands
			{
				'{COPYFILE} "' .. Dependencies.Vulkan.LibDir .. '/libvulkan.1.dylib" "%{cfg.targetdir}"',
				'{COPYFILE} "' .. Dependencies.Vulkan.LibDir .. '//lib' .. Dependencies.Vulkan.LibName .. '.dylib" "%{cfg.targetdir}"',
			}
		end

	filter "action:vs*"
    	buildoptions { "/Zc:preprocessor" }

	filter "action:xcode*"
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
		defines "NANO_DEBUG"
		runtime "Release"
		optimize "on"

		defines
		{
			"TRACY_ENABLE"
		}

	filter "configurations:Dist"
		kind "WindowedApp"
		defines "NG_CONFIG_DIST"
		runtime "Release"
		optimize "Full"
		linktimeoptimization "on"
