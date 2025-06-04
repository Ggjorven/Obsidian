#include "Main.hpp"

#if 1

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

#else

NANO_BENCHMARK_INIT("Main")
{
	NANO_BENCHMARK_AUTO("Init-Destroy")
	{
		Main(__argc, __argv);
	};
}

NANO_BENCHMARK_MAIN()

#endif