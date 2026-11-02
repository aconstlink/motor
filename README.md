# motor

[![CMake on Linux with OpenGL](https://github.com/aconstlink/motor/actions/workflows/cmake-lin-gcc-gl.yml/badge.svg)](https://github.com/aconstlink/motor/actions/workflows/cmake-lin-gcc-gl.yml)
[![CMake on Win32 with DirectX 11](https://github.com/aconstlink/motor/actions/workflows/cmake-win32-dx11.yml/badge.svg)](https://github.com/aconstlink/motor/actions/workflows/cmake-win32-dx11.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

motor is an independently developed C++20 real-time framework for graphics,
tooling, runtime systems, and interactive applications.

I build motor for my own demos and game projects, and to explore how the systems
behind them fit together. Applications can use the graphics layer directly or
build on the scene graph.

## History

The motor repository started in 2023. Parts of its code originate from earlier
engines that I have been developing since 2011 and have continued to rework into motor.

## Project Structure

motor is split into libraries:

- `application`, `platform` - application callbacks, windows, and platform integration
- `graphics`, `gfx` - rendering backends, resources, cameras, lights, and rendering helpers
- `scene` - scene graph, components, visitors, and animation
- `msl`, `wire` - motor shading language, shader translation, and typed data-flow connections
- `math`, `geometry`, `noise`, `collide` - mathematics, geometry, and intersection tests
- `concurrent`, `io`, `format` - tasks, file/database access, and asset import
- `audio`, `controls` - audio and input
- `core`, `memory`, `log`, `profiling`, `tool`, `property` - utilities, diagnostics, and tooling

Third-party dependencies are integrated through CMake under `externals/`.
Examples and tests live in [motor_suites](https://github.com/aconstlink/motor_suites)
and [motor_ai_suites](https://github.com/aconstlink/motor_ai_suites).

## Build

motor supports Windows with OpenGL 4 and Direct3D 11, and Linux with X11/GLX OpenGL.

```bash
git clone --recursive https://github.com/aconstlink/motor.git
cd motor
cmake -S . -B build
cmake --build build
```

For an existing checkout, initialize missing submodules with:

```bash
git submodule update --init --recursive
```

## Install Package

```bash
cmake --build build --config Release
cmake --install build --config Release --prefix path/to/motor-package
```

Point `CMAKE_PREFIX_PATH` at the install directory in a consuming project:

```cmake
find_package(motor 0.0.2 EXACT CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE motor::complete)
```

Individual modules, such as `motor::math` or `motor::graphics`, can be linked instead.
The package uses exact version matching. Binary packages must match the consumer's
platform, architecture, compiler/runtime, and build configuration. Shared libraries
must be available to the runtime loader; system graphics dependencies are required
on the consuming machine.

## Status

Active development, currently focused on demos and scene-based integration tests.
APIs and internal systems are still evolving; motor is not a finished SDK.

## AI-Assisted Development

I use AI for test applications, bug investigation, targeted fixes, and technical
discussions. I review the changes I accept and remain responsible for motor's
architecture and direction. I do not use AI to add features I do not understand.

This support has revitalized development. With limited time available, spending
less time tracking down bugs lets me focus on coding, experimenting, and building
my demos and games.

## License

MIT License.
