# motor

[![CMake on Linux with OpenGL](https://github.com/aconstlink/motor/actions/workflows/cmake-lin-gcc-gl.yml/badge.svg)](https://github.com/aconstlink/motor/actions/workflows/cmake-lin-gcc-gl.yml)
[![CMake on Win32 with DirectX 11](https://github.com/aconstlink/motor/actions/workflows/cmake-win32-dx11.yml/badge.svg)](https://github.com/aconstlink/motor/actions/workflows/cmake-win32-dx11.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
`motor` is a custom C++ real-time framework for graphics, tooling, runtime systems, and interactive applications.

The project is developed as a personal engine/runtime environment. It is not meant to compete with large commercial engines. Its purpose is to explore and build the systems behind real-time applications directly: rendering abstraction, resource lifetimes, scene composition, shader workflows, data flow, tooling, and live iteration.

## Focus

`motor` is currently focused on demo and visual application workflows:

- multi-window rendering for tool and production views
- OpenGL and DirectX rendering backends
- explicit graphics resource configuration and release
- asynchronous render-resource workflows
- shader compilation and live iteration through MSL
- scene graph with components and visitors
- typed data-flow through the `wire` slot system
- glTF based scene import from Blender
- camera sequencing and animation playback
- HDR/post-processing and render-pass experiments

The engine is built around explicit control. Objects are configured before they are used, ownership is visible, and higher-level systems such as the scene graph can automate that lifecycle when needed.

## Project Structure

The repository is split into small libraries. Important modules include:

- `application` - application and window integration
- `graphics` - backend-facing graphics objects, frontend command layer, render state and resource handling
- `gfx` - higher-level rendering helpers and camera utilities
- `scene` - scene graph, components, visitors, animation and graphics integration
- `msl` - motor shading language and shader translation infrastructure
- `wire` - typed slots and data-flow connections
- `math`, `geometry`, `noise` - math and geometry support
- `concurrent` - task and scheduling utilities
- `tool`, `property` - runtime tooling and editable data interfaces
- `format` - asset and file format integration

Third-party dependencies live under `externals/` and are pulled in as git submodules. The engine integrates them through local CMake targets instead of modifying the external source code.

## Related Repositories

The engine repository intentionally does not contain all demos and visual test applications.

- [`motor_suites`](https://github.com/aconstlink/motor_suites) contains sample applications, integration checks, and manual/visual test scenarios.
- `motor_demos` is used for demo-focused applications built on top of the engine.

This keeps the engine repository focused while still allowing larger examples and experiments to evolve separately.

## Build

Clone with submodules:

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

Windows is the primary development platform. The engine is also built and tested on Linux for OpenGL-oriented configurations.

## Status

Active development.

The current work is centered on building polished real-time demos and using them as production-style tests for the engine: Blender scene import, camera sequencing, scene streaming, render passes, shadows, HDR, bloom, and post-processing.

APIs and internal systems are still evolving. The repository is best understood as an engine development project and research runtime, not as a finished SDK.

## License

MIT License.
