MacOSVersion = MacOSVersion or "14.5"

project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++23"
	staticruntime "On"

	debugdir ("%{prj.location}")

	architecture "x86_64"

	warnings "Extra"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

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
	if gfxapi == "vulkan" then
        defines { "NG_API_VULKAN" }
		includedirs { "%{Dependencies.Vulkan.IncludeDir}" }
    elseif gfxapi == "dx12" then
        defines { "NG_API_DX12" }
		includedirs { "%{Dependencies.DX12.IncludeDir}", "%{Dependencies.D3D12MA.IncludeDir}" }
    elseif gfxapi == "metal" then
        defines { "NG_API_METAL" }
    elseif gfxapi == "dummy" then
        defines { "NG_API_DUMMY" }
    end

	includedirs
	{
		"src",

		"%{wks.location}/Graphics/src",

		"%{Dependencies.GLFW.IncludeDir}",
		"%{Dependencies.glm.IncludeDir}",
		"%{Dependencies.Tracy.IncludeDir}",
		"%{Dependencies.Nano.IncludeDir}",
		"%{Dependencies.SPIRVCross.IncludeDir}",
	}

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

		links
		{
			"%{Dependencies.GLFW.LibName}",
			"%{Dependencies.Tracy.LibName}",
		}

		if gfxapi == "vulkan" then
			links
			{
				"%{Dependencies.Vulkan.LibDir}/%{Dependencies.Vulkan.LibName}",
				"%{Dependencies.Vulkan.LibDir}/%{Dependencies.ShaderC.LibName}",
			}
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
			libdirs
			{
				"%{Dependencies.Vulkan.LibDir}"
			}

			links
			{
				"%{Dependencies.Vulkan.LibName}",
				"%{Dependencies.ShaderC.LibName}",
			}

			postbuildcommands
			{
				'{COPYFILE} "%{Dependencies.Vulkan.LibDir}/libvulkan.1.dylib" "%{cfg.targetdir}"',
				'{COPYFILE} "%{Dependencies.Vulkan.LibDir}/lib%{Dependencies.Vulkan.LibName}.dylib" "%{cfg.targetdir}"',
			}
		end

	filter "action:vs*"
    	buildoptions { "/Zc:preprocessor" }

	filter "action:xcode*"
		-- Note: If we don't add the header files to the externalincludedirs
		-- we can't use <angled> brackets to include files.
		externalincludedirs
		{
			"src",

			"%{wks.location}/Lumen/src",

			"%{Dependencies.GLFW.IncludeDir}",
			"%{Dependencies.glm.IncludeDir}",
			"%{Dependencies.Tracy.IncludeDir}",
			"%{Dependencies.Nano.IncludeDir}",
			"%{Dependencies.SPIRVCross.IncludeDir}",
		}

		if gfxapi == "vulkan" then
			externalincludedirs
			{
				"%{Dependencies.Vulkan.IncludeDir}",
			}
		end

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
