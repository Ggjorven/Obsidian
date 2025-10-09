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
	if OBSIDIAN_GRAPHICS_API == "vulkan" then
        defines { "OB_API_VULKAN" }
    elseif OBSIDIAN_GRAPHICS_API == "dx12" then
        defines { "OB_API_DX12" }
	elseif OBSIDIAN_GRAPHICS_API == "metal" then
        defines { "OB_API_METAL" }
	elseif OBSIDIAN_GRAPHICS_API == "dummy" then
        defines { "OB_API_DUMMY" }
    end

	includedirs
	{
		"src",
	}

	includedirs(Dependencies.Obsidian.IncludeDir)
	
	links(Dependencies.Obsidian.LibName)

	filter "system:windows"
		systemversion "latest"
		staticruntime "on"
		editandcontinue "off"

        defines
        {
            "NOMINMAX"
        }

	filter "system:linux"
		systemversion "latest"
		staticruntime "on"

    filter "system:macosx"
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

			postbuildcommands(Dependencies.Obsidian.PostBuildCommands)
		end

	filter "action:vs*"
    	buildoptions { "/Zc:preprocessor" }

	filter "action:xcode*"
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
		defines "NANO_DEBUG"
		runtime "Release"
		optimize "on"

		defines
		{
			"TRACY_ENABLE"
		}

	filter "configurations:Dist"
		kind "WindowedApp"
		defines "OB_CONFIG_DIST"
		runtime "Release"
		optimize "Full"
		linktimeoptimization "on"
