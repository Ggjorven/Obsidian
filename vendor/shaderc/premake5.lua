MacOSVersion = MacOSVersion or "14.5"

project "shaderc"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "On"

	architecture "x86_64"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	warnings "Off"

	files
	{
		"ShaderCompiler/ShaderCompiler/src/**.h",
		"ShaderCompiler/ShaderCompiler/src/**.hpp",
		"ShaderCompiler/ShaderCompiler/src/**.inl",
		"ShaderCompiler/ShaderCompiler/src/**.cpp",
		"ShaderCompiler/ShaderCompiler/src/**.cc",

		"ShaderCompiler/ShaderCompiler/include/glslang/**.c",
		"ShaderCompiler/ShaderCompiler/include/glslang/**.h",
		"ShaderCompiler/ShaderCompiler/include/glslang/**.hpp",
		"ShaderCompiler/ShaderCompiler/include/glslang/**.cpp",
		"ShaderCompiler/ShaderCompiler/include/glslang/**.cc",

		"ShaderCompiler/ShaderCompiler/include/glslangSPIRV/**.c",
		"ShaderCompiler/ShaderCompiler/include/glslangSPIRV/**.h",
		"ShaderCompiler/ShaderCompiler/include/glslangSPIRV/**.hpp",
		"ShaderCompiler/ShaderCompiler/include/glslangSPIRV/**.cpp",
		"ShaderCompiler/ShaderCompiler/include/glslangSPIRV/**.cc",
	}

	removefiles
	{
		"ShaderCompiler/ShaderCompiler/include/glslang/OSDependent/**",

		"ShaderCompiler/ShaderCompiler/src/spirv-tools/fuzz/**",
		"ShaderCompiler/ShaderCompiler/src/spirv-tools/reduce/**",
		"ShaderCompiler/ShaderCompiler/src/spirv-tools/wasm/**",
	}

	defines
	{
		"ENABLE_HLSL",

		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"ShaderCompiler/ShaderCompiler/src",
		"ShaderCompiler/ShaderCompiler/include",
	}

	filter "system:windows"
		defines "SPIRV_WINDOWS"
		systemversion "latest"
		staticruntime "on"
		
		files
		{
			"ShaderCompiler/ShaderCompiler/include/glslang/OSDependent/Windows/**.c",
			"ShaderCompiler/ShaderCompiler/include/glslang/OSDependent/Windows/**.h",
			"ShaderCompiler/ShaderCompiler/include/glslang/OSDependent/Windows/**.hpp",
			"ShaderCompiler/ShaderCompiler/include/glslang/OSDependent/Windows/**.cpp",
			"ShaderCompiler/ShaderCompiler/include/glslang/OSDependent/Windows/**.cc",
		}
		
	filter "system:linux"
		defines "SPIRV_UNIX"
		systemversion "latest"
		staticruntime "on"
		
		files
		{
			"ShaderCompiler/ShaderCompiler/include/glslang/OSDependent/Unix/**.c",
			"ShaderCompiler/ShaderCompiler/include/glslang/OSDependent/Unix/**.h",
			"ShaderCompiler/ShaderCompiler/include/glslang/OSDependent/Unix/**.hpp",
			"ShaderCompiler/ShaderCompiler/include/glslang/OSDependent/Unix/**.cpp",
			"ShaderCompiler/ShaderCompiler/include/glslang/OSDependent/Unix/**.cc",
		}
		
	filter "system:macosx"
		defines "SPIRV_UNIX"
		systemversion(MacOSVersion)
		staticruntime "on"

		files
		{
			"ShaderCompiler/ShaderCompiler/include/glslang/OSDependent/Unix/**.c",
			"ShaderCompiler/ShaderCompiler/include/glslang/OSDependent/Unix/**.h",
			"ShaderCompiler/ShaderCompiler/include/glslang/OSDependent/Unix/**.hpp",
			"ShaderCompiler/ShaderCompiler/include/glslang/OSDependent/Unix/**.cpp",
			"ShaderCompiler/ShaderCompiler/include/glslang/OSDependent/Unix/**.cc",
		}

	filter "action:xcode*"
		-- Note: If we don't add the header files to the externalincludedirs
		-- we can't use <angled> brackets to include files.
		externalincludedirs
		{
			"ShaderCompiler/ShaderCompiler/src",
			"ShaderCompiler/ShaderCompiler/include",
		}

	filter "configurations:Debug"
		defines "NDEBUG"
		runtime "Debug"
		symbols "on"
	
	filter "configurations:Release"
		defines "NDEBUG"
		runtime "Release"
		optimize "on"
		
	filter "configurations:Dist"
		defines "NDEBUG"
		runtime "Release"
		optimize "Full"
