MacOSVersion = MacOSVersion or "14.5"

project "ImGui"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	architecture "x86_64"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"ImGui/imconfig.h",
		"ImGui/imgui.h",
		"ImGui/imgui.cpp",
		"ImGui/imgui_draw.cpp",
		"ImGui/imgui_internal.h",
		"ImGui/imgui_tables.cpp",
		"ImGui/imgui_widgets.cpp",
		"ImGui/imstb_rectpack.h",
		"ImGui/imstb_textedit.h",
		"ImGui/imstb_truetype.h",
		"ImGui/imgui_demo.cpp",

		"ImGui/backends/imgui_impl_glfw.h",
		"ImGui/backends/imgui_impl_glfw.cpp",
	}

	includedirs
	{
		"ImGui",

		"%{Dependencies.GLFW.IncludeDir}" 
	}

	if gfxapi == "vulkan" then
		files
		{
			"ImGui/backends/imgui_impl_vulkan.h",
			"ImGui/backends/imgui_impl_vulkan.cpp",
		}
		includedirs 
		{ 
			"%{Dependencies.Vulkan.IncludeDir}" 
		}
    elseif gfxapi == "dx12" then
		files
		{
			"ImGui/backends/imgui_impl_dx12.h",
			"ImGui/backends/imgui_impl_dx12.cpp",
		}
	elseif gfxapi == "metal" then -- TODO: Implement
		-- files
		-- {
		-- 	"ImGui/backends/imgui_impl_metal.h",
		-- 	"ImGui/backends/imgui_impl_metal.mm",
		-- }
    end

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

	filter "system:linux"
		systemversion "latest"
		staticruntime "On"
		pic "On"

	filter "system:macosx"
		systemversion(MacOSVersion)
		staticruntime "on"
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
