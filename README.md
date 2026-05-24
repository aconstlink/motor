# motor

[![CMake on Linux with OpenGL](https://github.com/aconstlink/motor/actions/workflows/cmake-lin-gcc-gl.yml/badge.svg)](https://github.com/aconstlink/motor/actions/workflows/cmake-lin-gcc-gl.yml)
[![CMake on Win32 with DirectX 11](https://github.com/aconstlink/motor/actions/workflows/cmake-win32-dx11.yml/badge.svg)](https://github.com/aconstlink/motor/actions/workflows/cmake-win32-dx11.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

`motor` is a custom C++ engine for real-time audio/visual applications. It is designed around explicit control, predictable ownership, modular systems, and fast iteration.

The project is primarily a learning and research engine: it explores rendering abstraction, runtime systems, memory/resource management, and tooling workflows without hiding the low-level details.

## Features

### Rendering

- Multi-backend rendering architecture
  - OpenGL 4
  - DirectX 11
  - OpenGL ES 3
- Cross-backend shader and material flow through the engine's MSL layer
- Runtime shader compilation and reload-oriented workflows
- Support for running multiple rendering backends/windows for debugging and comparison

### Runtime Systems

- Modular engine libraries built with CMake targets
- Application lifecycle hooks for update, render, audio, input, logic, physics, profiling, networking, and tooling
- Scene graph with components and visitors
- Custom task/concurrency utilities
- Explicit resource and memory management

### Memory

- Central memory manager
- Allocation tracking with purpose tags
- Managed and borrowed pointer conventions
- Runtime memory visibility and leak-dump support

### Tooling

- Dear ImGui integration
- Built-in tooling hooks through `on_tool()`
- Engine profiling UI support
- Asset- and shader-iteration workflows intended for fast development loops

## Repository Scope

This repository contains the engine code itself. It intentionally does not contain the main sample and demo applications.

Related repositories:

- [`motor_suites`](https://github.com/aconstlink/motor_suites)  
  Sample applications, manual/visual test scenarios, and integration checks for engine layers.

- [`motor_exdep`](https://github.com/aconstlink/motor_exdep)  
  Work-in-progress external dependency and package integration setup.

- [`motor_demos`](https://github.com/aconstlink/motor_demos)  
  Demo-focused applications built with the engine.

Keeping samples and demos outside the engine repository helps keep the engine code focused and makes the engine easier to include in other projects.

## Dependencies

Third-party code is pulled in through git submodules under `externals/`. The engine provides local CMake files that expose those dependencies as targets used by the rest of the build.

## Build

```bash
git clone --recursive https://github.com/aconstlink/motor.git
cd motor
cmake -S . -B build
cmake --build build
```

If the repository was cloned without submodules:

```bash
git submodule update --init --recursive
```

## Platforms

Windows is the main development platform.

Regularly used targets include:

- Windows 10/11 with Visual Studio 2019 or newer
- Linux with GCC and OpenGL development packages
  ```libX11 mesa-libGLES mesa-libGL mesa-libEGL alsa-lib```  
OpenGL ES support exists in the codebase, but is not currently the primary tested path.

## Status

Active development. The current focus is on building toward polished real-time demos that exercise scene loading, rendering, runtime resources, and iteration workflows.

## License

MIT License
