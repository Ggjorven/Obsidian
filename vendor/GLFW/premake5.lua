local MacOSVersion = MacOSVersion or "14.5"
local OutputDir = OutputDir or "%{cfg.buildcfg}-%{cfg.system}"

project "GLFW"
	kind "StaticLib"
	language "C"
	warnings "Off"

	targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. OutputDir .. "/%{prj.name}")

	files
	{
		"GLFW/include/GLFW/glfw3.h",
		"GLFW/include/GLFW/glfw3native.h",
		"GLFW/src/glfw_config.h",
		"GLFW/src/context.c",
		"GLFW/src/init.c",
		"GLFW/src/input.c",
		"GLFW/src/monitor.c",

		"GLFW/src/null_init.c",
		"GLFW/src/null_joystick.c",
		"GLFW/src/null_monitor.c",
		"GLFW/src/null_window.c",

		"GLFW/src/platform.c",
		"GLFW/src/vulkan.c",
		"GLFW/src/window.c",
	}

	filter "system:windows"
		staticruntime "on"
		systemversion "latest"

		files
		{
			"GLFW/src/win32_init.c",
			"GLFW/src/win32_joystick.c",
			"GLFW/src/win32_module.c",
			"GLFW/src/win32_monitor.c",
			"GLFW/src/win32_time.c",
			"GLFW/src/win32_thread.c",
			"GLFW/src/win32_window.c",
			"GLFW/src/wgl_context.c",
			"GLFW/src/egl_context.c",
			"GLFW/src/osmesa_context.c"
		}

		defines
		{
			"_GLFW_WIN32",
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "system:linux"
		staticruntime "on"
        systemversion "latest"
		pic "On"

        files
        {
            "GLFW/src/xkb_unicode.c",
            "GLFW/src/posix_module.c",
			"GLFW/src/posix_time.c",
			"GLFW/src/posix_thread.c",
			"GLFW/src/posix_module.c",
			"GLFW/src/posix_poll.c",
			"GLFW/src/egl_context.c",
			"GLFW/src/glx_context.c",
			"GLFW/src/osmesa_context.c",
			"GLFW/src/linux_joystick.c"
        }

        -- Check if system is running wayland
        if os.getenv("WAYLAND_DISPLAY") then -- Note: Wayland has never been tested.
            defines { "_GLFW_WAYLAND" }

            files
            {
               "GLFW/src/wl_init.c",
               "GLFW/src/wl_monitor.c",
               "GLFW/src/wl_window.c",
            }
        else -- Default to X11 -- os.getenv("DISPLAY")
            defines { "_GLFW_X11" }

            files
            {
                "GLFW/src/x11_init.c",
                "GLFW/src/x11_monitor.c",
                "GLFW/src/x11_window.c",
                "GLFW/src/xkb_unicode.c",
            }
        end

	filter "system:macosx"
		staticruntime "on"
		systemversion(MacOSVersion)
		pic "On"

		defines
		{
			"_GLFW_COCOA"
		}

		files
		{
			"GLFW/src/cocoa_init.m",
			"GLFW/src/cocoa_monitor.m",
			"GLFW/src/cocoa_window.m",
			"GLFW/src/cocoa_joystick.m",
			"GLFW/src/cocoa_time.c",
			"GLFW/src/nsgl_context.m",
			"GLFW/src/posix_thread.c",
			"GLFW/src/posix_module.c",
			"GLFW/src/osmesa_context.c",
			"GLFW/src/egl_context.c"
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

    filter "configurations:Dist"
		runtime "Release"
		optimize "Full"