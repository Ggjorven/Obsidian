local MacOSVersion = MacOSVersion or "14.5"
local OutputDir = OutputDir or "%{cfg.buildcfg}-%{cfg.system}"

project "SPIRVCross"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	warnings "Off"

	targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. OutputDir .. "/%{prj.name}")

	files
	{
		"SPIRV-Cross/spirv_cfg.cpp",
		"SPIRV-Cross/spirv_cpp.cpp",
		"SPIRV-Cross/spirv_cross.cpp",
		"SPIRV-Cross/spirv_cross_util.cpp",
		"SPIRV-Cross/spirv_glsl.cpp",
		"SPIRV-Cross/spirv_hlsl.cpp",
		"SPIRV-Cross/spirv_msl.cpp",
		"SPIRV-Cross/spirv_parser.cpp",
		"SPIRV-Cross/spirv_cross_parsed_ir.cpp",
		"SPIRV-Cross/spirv_reflect.cpp",
	}

	includedirs
	{
		"SPIRV-Cross",
		"SPIRV-Cross/include",
	}

	filter "system:windows"
		staticruntime "on"
		systemversion "latest"

	filter "system:linux"
		staticruntime "on"
        systemversion "latest"

	filter "system:macosx"
		staticruntime "on"
		systemversion(MacOSVersion)

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

    filter "configurations:Dist"
		runtime "Release"
		optimize "Full"