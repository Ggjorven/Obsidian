# NanoGraphics

NanoGraphics is an extensive abstraction layer around multiple graphics APIs allowing for performance-based rendering.
Heavily inspired by NVRHI, but with a more modern API and more abstraction.

## Features

- **Low-Level Explicit Control**: Maintain full control over rendering through CommandLists, Buffers, and Image resources without hiding important GPU concepts.

- **Backend Abstraction (Vulkan & DX12)**: Abstracts both Vulkan and Direct3D 12 with a unified API while staying close to the metal, offering predictable performance and behavior.

- **Stack-Allocated, No-VTable Architecture**: API-agnostic types like Image, Buffer, and CommandList are lightweight and stack-allocated, eliminating runtime overhead from virtual tables or dynamic memory allocation.

- **Compile-Time Backend Selection**: Backend (Vulkan or DX12) is selected at compile-time, removing runtime branching and maximizing performance and binary simplicity.

- **Staging Resources**: Built-in support for staging images and buffers to assist with resource uploads and readbacks.

- **Automatic barrier insertion**: Maintain low-level control over GPU operations while leveraging automatic barrier tracking

- **Minimal Runtime Dependencies**: Focused on keeping the runtime lightweight, with no mandatory heap allocations or reflection systems.

## Getting Started

### Prerequisites

Ensure you have the following installed on your system:
- Vulkan SDK (Required for Vulkan API, windows, linux & macos)
- Windows SDK (Required for DX12 API, windows only)

- A C++23 or later compiler (e.g., MSVC, GCC, or Clang)
- Build tools: Make (Linux/Windows) or Visual Studio 2022 (Windows) or XCode (MacOS)

### Building

Build instructions for this project can be found in the [BUILDING.md](BUILDING.md) file. Supported platforms are:
- **Windows**: Visual Studio 2022, Make
- **Linux**: Make
- **MacOS**: XCode

### Programming Guide

// TODO: ...

### Samples

// TODO: ...

## License

This project is licensed under the Apache 2.0 License. See [LICENSE](LICENSE.txt) for details.

## Contributing

Contributions are welcome! Please fork the repository and create a pull request with your changes.

## Third-Party Libraries
- [GLFW](https://github.com/glfw/glfw) - Windowing and input handling
- [glm](https://github.com/g-truc/glm) - Mathematics library for graphics
- [tracy](https://github.com/wolfpld/tracy) - Realtime sampling profiler
- [Nano](https://github.com/ggjorven/Nano) - Additional utilities
- [VMA](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) - Vulkan Memory Allocator
- [D3D12MA](https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator) - D3D12 Memory Allocator
- [shaderc](https://github.com/google/shaderc) - Shader compilation for GLSL and HLSL
- [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) - Transpiling shader code