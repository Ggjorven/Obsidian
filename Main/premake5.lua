local Dependencies = local_require("../Dependencies.lua")
local MacOSVersion = MacOSVersion or "14.5"
local OutputDir = OutputDir or "%{cfg.buildcfg}-%{cfg.system}"

project "Main"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++23"
	staticruntime "On"

	architecture "x86_64"

	warnings "Extra"

	targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. OutputDir .. "/%{prj.name}")

	files
	{
		"src/**.cpp"
	}

	defines
	{
		"GLFW_INCLUDE_NONE",
	}

	defines { "OB_API_VULKAN" }
	removefiles { "src/Obsidian/Platform/DX12/**", "src/Obsidian/Platform/Metal/**", "src/Obsidian/Platform/Dummy/**" }

	includedirs
	{
		"src",
		"src/Obsidian",
	}

	includedirs(Dependencies.Obsidian.IncludeDir)

	links(Dependencies.GLFW.LibName)
	links(Dependencies.shaderc.LibName)

	libdirs(Dependencies.Obsidian.LibDir)
 
	filter "system:windows"
		systemversion "latest"
		staticruntime "on"
		editandcontinue "off"

        defines
        {
            "NOMINMAX"
        }

		links(Dependencies.Vulkan.LibDir .. "/" .. Dependencies.Vulkan.LibName)

	filter "system:linux"
		systemversion "latest"
		staticruntime "on"

		libdirs(Dependencies.Vulkan.LibDir)
		links(Dependencies.Vulkan.LibName)
		
		if OBSIDIAN_DISPLAY_MANAGER == "x11" then
			defines("OB_DISPLAY_MANAGER_X11")
			links
			{
				"Xrandr", "Xi", "GLU", "GL", "GLX", "X11"
			}
		elseif OBSIDIAN_DISPLAY_MANAGER == "wayland" then
			defines("OB_DISPLAY_MANAGER_WAYLAND")
		end

		links
		{
			"dl", "pthread", "stdc++fs"
		}

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

		libdirs(Dependencies.Vulkan.LibDir)
		links(Dependencies.Vulkan.LibName)

		postbuildcommands(Dependencies.Obsidian.PostBuildCommands)

	filter "action:vs*"
		buildoptions { "/Zc:preprocessor" }

	filter "action:xcode*"
		-- Note: If we don't add the header files to the externalincludedirs
		-- we can't use <angled> brackets to include files.
		externalincludedirs(includedirs())

	filter "configurations:Debug"
		defines "OB_CONFIG_DEBUG"
		runtime "Debug"
		symbols "on"
		
	filter "configurations:Release"
		defines "OB_CONFIG_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "OB_CONFIG_DIST"
		runtime "Release"
		optimize "Full"
		linktimeoptimization "on"
