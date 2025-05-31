MacOSVersion = MacOSVersion or "14.5"

project "Glad"
	kind "StaticLib"
	language "C"
	warnings "Off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"glad/src/glad.c",
	}

	includedirs
	{
		"glad/include"
	}

	filter "system:windows"
		staticruntime "on"
		systemversion "latest"

	filter "system:linux"
		staticruntime "on"
        systemversion "latest"
		pic "On"

	filter "system:macosx"
		staticruntime "on"
		systemversion(MacOSVersion)
		pic "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

    filter "configurations:Dist"
		runtime "Release"
		optimize "Full"