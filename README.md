# Obsidian

Obsidian is an extensive abstraction layer around multiple graphics APIs allowing for performance-based rendering.
Heavily inspired by NVRHI, but with a more modern API and more abstraction.

## Information

This is a minimal example for a vulkan issue being experienced on Wayland + NVIDIA.
Issue [#20](https://github.com/Ggjorven/Obsidian/issues/20) on github.

## Getting Started

### Prerequisites

Ensure you have the following installed on your system:
- Vulkan SDK (Required for Vulkan API, windows, linux & macos)
- A C++23 or later compiler (e.g., MSVC, GCC, or Clang)
- Build tools: Make (Linux/Windows) or Visual Studio 2022 (Windows) or XCode (MacOS)

### Building

Build instructions for this project can be found in the [BUILDING.md](BUILDING.md) file. Supported platforms are:
- **Windows**: Visual Studio 2022, Make
- **Linux**: Make
- **MacOS**: XCode

## License

This project is licensed under the Apache 2.0 License. See [LICENSE](LICENSE.txt) for details.

## Contributing

Contributions are welcome! Please fork the repository and create a pull request with your changes.

## Third-Party Libraries
- [GLFW](https://github.com/glfw/glfw) - Windowing and input handling
- [VMA](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) - Vulkan Memory Allocator
- [shaderc](https://github.com/google/shaderc) - Shader compilation for GLSL and HLSL
