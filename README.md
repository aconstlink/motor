# motor

[![CMake on Linux with OpenGL](https://github.com/aconstlink/motor/actions/workflows/cmake-lin-gcc-gl.yml/badge.svg)](https://github.com/aconstlink/motor/actions/workflows/cmake-lin-gcc-gl.yml)
[![CMake on Win32 with DirectX 11](https://github.com/aconstlink/motor/actions/workflows/cmake-win32-dx11.yml/badge.svg)](https://github.com/aconstlink/motor/actions/workflows/cmake-win32-dx11.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

motor is my personal C++20 real-time framework for graphics, tooling, runtime systems, and interactive applications.

I build motor for my own demos and game projects, and to explore how the systems behind them fit together. 
Applications can use the graphics layer directly or build on the scene graph.

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
find_package(motor 0.1.0 EXACT CONFIG REQUIRED)
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

## My AI View and Usage

I **DO NOT** use AI for any new features into the engine or let it generate any code. I worked many years on the engine and many many hours, day and night. If I'd use AI to vibe anything into existence now, that would invalidate everything I did so far and I could stop doing this today. The original reason was, and still is, to learn many things and finally release some demos at some demos parties. And also finally make a game with the engine! Those were the original reasons I started coding engines 2009, I think it was.

I think, with an absolute zero Ai tolerance, i.e. no AI at any level, I'd be absolutely lost and left behind. So there are the following Ai use cases for me that I practice in reality:

1. Finding bugs: Just having the AI find the bugs is a tremendous help, and I may let the AI fix it, if it is not too much. 
2. Technical discussions: But I always suggest something. I do have to have my own ideas and opinions.
3. Writing separate [test applications](https://github.com/aconstlink/motor_ai_suites) : I need this. So many thing can go wrong and I(and motor) just need the tests. And non of that code will see any production light of day.
4. Writing the Wiki: I read all of it and I do also do corrections.
5. Let it generate issue texts

## License

MIT License.
