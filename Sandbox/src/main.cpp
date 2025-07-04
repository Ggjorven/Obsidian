// Note: Specify the test to run here since the Main function
// is defined in the header file.
#include "Tests/TexturedQuad.hpp"

// Note: On windows to remove the terminal on distributions we need a special main function
// on linux and macos a regular main function is fine.
#if defined(NG_CONFIG_DIST) && defined(NG_PLATFORM_WINDOWS) 
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