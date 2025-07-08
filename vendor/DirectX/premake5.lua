project "D3D12MA"
	kind "StaticLib"
	language "C++"
	cppdialect "C++14"
	warnings "Off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"D3D12MA/src/D3D12MemAlloc.cpp"
	}

	includedirs
	{
		"D3D12MA/include"
	}

	filter "system:windows"
		staticruntime "on"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

    filter "configurations:Dist"
		runtime "Release"
		optimize "Full"