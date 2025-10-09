local MacOSVersion = MacOSVersion or "14.5"
local OutputDir = OutputDir or "%{cfg.buildcfg}-%{cfg.system}"

project "Tracy"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	-- staticruntime "Off"
	warnings "Off"

	targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. OutputDir .. "/%{prj.name}")

	files
	{
		"tracy/public/client/**.h",
		"tracy/public/client/**.hpp",
		"tracy/public/client/**.cpp",

		"tracy/public/common/**.h",
		"tracy/public/common/**.hpp",
		"tracy/public/common/**.cpp",

		"tracy/public/tracy/**.h",
		"tracy/public/tracy/**.hpp",
		"tracy/public/tracy/**.cpp",

		"tracy/public/libbacktrace/alloc.cpp",
		"tracy/public/libbacktrace/sort.cpp",
		"tracy/public/libbacktrace/state.cpp"
	}

	includedirs
    {
        "tracy/public/"
    }

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

		links
		{
			"Ws2_32.lib",
			"DbgHelp.lib"
		}

	filter "system:linux"
		systemversion "latest"
		staticruntime "On"

		files
		{
			"tracy/public/libbacktrace/posix.cpp",
			"tracy/public/libbacktrace/mmapio.cpp",
			"tracy/public/libbacktrace/macho.cpp",
			"tracy/public/libbacktrace/fileline.cpp",
			"tracy/public/libbacktrace/elf.cpp",
			"tracy/public/libbacktrace/dwarf.cpp",
		}

	filter "system:macosx"
		systemversion(MacOSVersion)
		staticruntime "On"

		files
		{
			"tracy/public/libbacktrace/posix.cpp",
			"tracy/public/libbacktrace/mmapio.cpp",
			"tracy/public/libbacktrace/macho.cpp",
			"tracy/public/libbacktrace/fileline.cpp",
			"tracy/public/libbacktrace/dwarf.cpp",
		}

	filter "action:xcode*"
		-- Note: If we don't add the header files to the externalincludedirs
		-- we can't use <angled> brackets to include files.
		externalincludedirs(includedirs())

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"
		conformancemode "On"

		defines
		{
			"TRACY_ENABLE",
			"TRACY_ON_DEMAND"
		}

	filter "configurations:Release"
		runtime "Release"
		optimize "On"
		conformancemode "On"

		defines
		{
			"TRACY_ENABLE",
			"TRACY_ON_DEMAND"
		}

	filter "configurations:Dist"
		runtime "Release"
		optimize "Full"
		conformancemode "On"