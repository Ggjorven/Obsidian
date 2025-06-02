MacOSVersion = MacOSVersion or "14.5"

project "Graphics"
	kind "StaticLib"
	language "C++"
	cppdialect "C++23"
	staticruntime "On"

	architecture "x86_64"

	warnings "Extra"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

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
	if gfxapi == "vulkan" then
        defines { "NG_API_VULKAN" }
		removefiles { "src/NanoGraphics/Platform/D3D12/**", "src/NanoGraphics/Platform/D3D11/**", "src/NanoGraphics/Platform/OpenGL/**" }
		includedirs { "%{Dependencies.Vulkan.IncludeDir}" }
    elseif gfxapi == "d3d12" then
        defines { "NG_API_D3D12" }
		removefiles { "src/NanoGraphics/Platform/Vulkan/**", "src/NanoGraphics/Platform/D3D11/**", "src/NanoGraphics/Platform/OpenGL/**" }
	elseif gfxapi == "d3d11" then
        defines { "NG_API_D3D11" }
		removefiles { "src/NanoGraphics/Platform/Vulkan/**", "src/NanoGraphics/Platform/D3D12/**", "src/NanoGraphics/Platform/OpenGL/**" }
	elseif gfxapi == "opengl" then
        defines { "NG_API_OPENGL" }
		removefiles { "src/NanoGraphics/Platform/Vulkan/**", "src/NanoGraphics/Platform/D3D12/**", "src/NanoGraphics/Platform/D3D11/**" }
    end

	includedirs
	{
		"src",
		"src/NanoGraphics",

		"%{Dependencies.GLFW.IncludeDir}",
		"%{Dependencies.glad.IncludeDir}",
		"%{Dependencies.glm.IncludeDir}",
		"%{Dependencies.stb.IncludeDir}",
		"%{Dependencies.Tracy.IncludeDir}",
		"%{Dependencies.Nano.IncludeDir}",
	}

	links
	{
		"%{Dependencies.GLFW.LibName}",
		"%{Dependencies.Tracy.LibName}",
		"%{Dependencies.glad.LibName}",
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

		if gfxapi == "vulkan" then
			links
			{
				"%{Dependencies.Vulkan.LibDir}/%{Dependencies.Vulkan.LibName}",
				"%{Dependencies.Vulkan.LibDir}/%{Dependencies.ShaderC.LibName}",
			}
		end

	filter "system:linux"
		defines "NG_PLATFORM_DESKTOP"
		defines "NG_PLATFORM_LINUX"
		systemversion "latest"
		staticruntime "on"

		if gfxapi == "vulkan" then
			links
			{
				"%{Dependencies.Vulkan.LibDir}/%{Dependencies.Vulkan.LibName}",
				"%{Dependencies.Vulkan.LibDir}/%{Dependencies.ShaderC.LibName}",
			}
		end
		
		links
		{
			"Xrandr", "Xi", "GLU", "GL", "GLX", "X11", "dl", "pthread", "stdc++fs"
		}

    filter "system:macosx"
		defines "NG_PLATFORM_DESKTOP"
		defines "NG_PLATFORM_MACOS"
		systemversion(MacOSVersion)
		staticruntime "on"

	filter "action:vs*"
		buildoptions { "/Zc:preprocessor" }

	filter "action:xcode*"
		-- Note: XCode only needs the full pchheader path
		pchheader "src/NanoGraphics/ngpch.h"

		-- Note: If we don't add the header files to the externalincludedirs
		-- we can't use <angled> brackets to include files.
		externalincludedirs
		{
			"src",
			"src/NanoGraphics",

			"%{Dependencies.GLFW.IncludeDir}",
			"%{Dependencies.glad.IncludeDir}",
			"%{Dependencies.glm.IncludeDir}",
			"%{Dependencies.stb.IncludeDir}",
			"%{Dependencies.Tracy.IncludeDir}",
			"%{Dependencies.Nano.IncludeDir}",
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
		defines "NG_CONFIG_DIST"
		runtime "Release"
		optimize "Full"
		linktimeoptimization "on"
