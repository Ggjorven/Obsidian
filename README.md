# Obsidian

Obsidian is an extensive abstraction layer around multiple graphics APIs allowing for performance-based rendering.
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

## Programming Guide

// TODO: ...

## Samples

// TODO: ...

## TODO List
- [x] Pushconstants
- [x] Bindless support
- [x] Tesselation control
- [ ] Dynamic buffers
- [ ] **Vulkan**
  - [x] Pushconstants
  - [x] Bindless support
  - [x] Tesselation control
  - [ ] Dynamic buffers
  - [x] Profiling 
- [ ] **Dx12**
  - [x] Bindless support
  - [x] Pushconstants
  - [x] Tesselation control
  - [ ] Dynamic buffers
  - [ ] Profiling
- [ ] **Metal**
  - [ ] Window
  - [ ] Device
  - [ ] Other...
     
## Tested Platforms (API, OS, GPU & Display Manager)

This is a list of platforms of where I have personally ran all tests. <br> Unless explicitly stated all of these platforms should work even when untested.

- [ ] **Dx12**
  - [ ] Windows
    - [x] NVIDIA [![Last tested](https://img.shields.io/github/commits-difference/Ggjorven/Obsidian?base=4ca6e3c53a968432467b875f105f237d93862480&head=main&label=Commits%20since%20last%20test&color=#32CD32)](https://github.com/Ggjorven/Obsidian/commit/4ca6e3c53a968432467b875f105f237d93862480)
    - [ ] AMD [![Last tested](https://img.shields.io/badge/Commits%20since%20last%20test-Never%20tested-lightgray)](https://github.com/Ggjorven/Obsidian/commit/main)
    - [x] Intel [![Last tested](https://img.shields.io/github/commits-difference/Ggjorven/Obsidian?base=4ca6e3c53a968432467b875f105f237d93862480&head=main&label=Commits%20since%20last%20test&color=#32CD32)](https://github.com/Ggjorven/Obsidian/commit/4ca6e3c53a968432467b875f105f237d93862480)
- [ ] **Vulkan**
  - [ ] Windows
    - [x] NVIDIA [![Last tested](https://img.shields.io/github/commits-difference/Ggjorven/Obsidian?base=4ca6e3c53a968432467b875f105f237d93862480&head=main&label=Commits%20since%20last%20test&color=#32CD32)](https://github.com/Ggjorven/Obsidian/commit/4ca6e3c53a968432467b875f105f237d93862480)
    - [ ] AMD [![Last tested](https://img.shields.io/badge/Commits%20since%20last%20test-Never%20tested-lightgray)](https://github.com/Ggjorven/Obsidian/commit/main)
    - [x] Intel [![Last tested](https://img.shields.io/github/commits-difference/Ggjorven/Obsidian?base=4ca6e3c53a968432467b875f105f237d93862480&head=main&label=Commits%20since%20last%20test&color=#32CD32)](https://github.com/Ggjorven/Obsidian/commit/4ca6e3c53a968432467b875f105f237d93862480)
  - [ ] Linux
    - [ ] X11
      - [x] NVIDIA [![Last tested](https://img.shields.io/github/commits-difference/Ggjorven/Obsidian?base=d792a20a1041967d96a9d13cea37661bf9748de8&head=main&label=Commits%20since%20last%20test&color=#32CD32)](https://github.com/Ggjorven/Obsidian/commit/d792a20a1041967d96a9d13cea37661bf9748de8)
      - [ ] AMD [![Last tested](https://img.shields.io/badge/Commits%20since%20last%20test-Never%20tested-lightgray)](https://github.com/Ggjorven/Obsidian/commit/main)
      - [x] Intel [![Last tested](https://img.shields.io/github/commits-difference/Ggjorven/Obsidian?base=4ca6e3c53a968432467b875f105f237d93862480&head=main&label=Commits%20since%20last%20test&color=#32CD32)](https://github.com/Ggjorven/Obsidian/commit/4ca6e3c53a968432467b875f105f237d93862480)
    - [ ] Wayland
      - [ ] NVIDIA [![Last tested](https://img.shields.io/github/commits-difference/Ggjorven/Obsidian?base=main&head=main&label=Commits%20since%20last%20test&color=orangered)](https://github.com/Ggjorven/Obsidian/commit/main)
      - [ ] AMD [![Last tested](https://img.shields.io/badge/Commits%20since%20last%20test-Never%20tested-lightgray)](https://github.com/Ggjorven/Obsidian/commit/main)
      - [x] Intel [![Last tested](https://img.shields.io/github/commits-difference/Ggjorven/Obsidian?base=main&head=main&label=Commits%20since%20last%20test&color=#32CD32)](https://github.com/Ggjorven/Obsidian/commit/main)
  - [ ] MacOS
    - [x] Intel [![Last tested](https://img.shields.io/github/commits-difference/Ggjorven/Obsidian?base=ab5da6bb2fd531449e51be5096deb58be1174052&head=main&label=Commits%20since%20last%20test&color=#32CD32)](https://github.com/Ggjorven/Obsidian/commit/ab5da6bb2fd531449e51be5096deb58be1174052)
    - [ ] M(X) [![Last tested](https://img.shields.io/badge/Commits%20since%20last%20test-Never%20tested-lightgray)](https://github.com/Ggjorven/Obsidian/commit/main)

## Additional information

Apart from the main development happening in the `main` (or `dev`) branch, there's an [`imgui`](https://github.com/your-repo/Obsidian/tree/imgui) branch that includes **Dear ImGui** integration for Obsidian. This branch is designed to make it easy to plug in GUI support without adding clutter to the core abstraction layers in the main branch.


Inside the `Sandbox` project, you’ll also find several example applications demonstrating usage of Obsidian.
- **TexturedQuad Example**: Loads and renders an image using [`stb_image`](https://github.com/nothings/stb/blob/master/stb_image.h), working for all supported backends.
- **TinyObjLoader Example**: Loads and renders 3D models using [`tinyobjloader`](https://github.com/tinyobjloader/tinyobjloader), again working across all supported backends.

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
