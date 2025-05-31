#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

#include <GLFW/glfw3.h>
#undef APIENTRY

#include <glm/glm.hpp>

#include <Tracy/Tracy.hpp>

#include <glad/glad.h>
#undef APIENTRY

#include <stb/stb_image.h>

#define NANO_IMPLEMENTATION
#include <Nano.hpp>

int Main(int argc, char* argv[])
{
	Nano::Log::PrintLn("Hello, world!");
	return 0;
}

#if !defined(NG_CONFIG_DIST)
	int main(int argc, char* argv[])
	{
		return Main(argc, argv);
	}
#elif defined(NG_PLATFORM_WINDOWS) // WindowedApp on windows
	#include <Windows.h>
	int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ PSTR, _In_ int)
	{
		return Main(__argc, __argv);
	}
#else
	int main(int argc, char* argv[])
	{
		return Main(argc, argv);
	}
#endif