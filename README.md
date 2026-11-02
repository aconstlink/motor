# motor

[![CMake on Linux with OpenGL](https://github.com/aconstlink/motor/actions/workflows/cmake-lin-gcc-gl.yml/badge.svg)](https://github.com/aconstlink/motor/actions/workflows/cmake-lin-gcc-gl.yml)
[![CMake on Win32 with DirectX 11](https://github.com/aconstlink/motor/actions/workflows/cmake-win32-dx11.yml/badge.svg)](https://github.com/aconstlink/motor/actions/workflows/cmake-win32-dx11.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

motor is an independently developed C++20 real-time framework for graphics,
tooling, runtime systems, and interactive applications.

I build motor for my own demos and game projects, and to explore how the systems
behind them fit together. Its scope extends beyond rendering: application
callbacks, asset loading, animation, typed data flow, audio, task scheduling,
memory tracking, and debugging tools are part of the same framework.

Applications can use the graphics layer directly or build on the scene graph.
The aim is explicit control with reusable infrastructure, rather than a complete
editor-driven game production environment.

## History

The motor repository started in 2023. Parts of its code originate from earlier
engines that I have been developing since 2011 and have continued to
rework and evolve into motor.

## Focus

motor is currently focused on demo and visual application workflows:

- multi-window rendering for tool and production views
- OpenGL 4 and Direct3D 11 rendering backends
- explicit graphics resource configuration and release
- asynchronous frame preparation and rendering
- background shader compilation and live iteration through msl
- scene graph with components and visitors
- typed data-flow through the `wire` slot system
- glTF scene import from Blender, including animation and punctual lights
- camera sequencing and animation playback
- multi-pass lighting, shadow mapping, HDR, bloom, and FXAA
- debug primitives, text rendering, and bounding-volume intersection tests

The engine is built around explicit control. Objects are configured before they are used, ownership is visible, and higher-level systems such as the scene graph can automate that lifecycle when needed.

## Application and Rendering Workflow

The application layer separates shared graphics preparation in `on_graphics`
from per-window rendering in `on_render`. `on_first_frame` and `on_last_frame`
provide per-window setup and cleanup points. Input, logic, audio, and physics
have their own update callbacks rather than being defined by the render rate.

Each window has a graphics frontend. The same engine-side graphics objects can
be configured and rendered through multiple frontends, including OpenGL 4 and
Direct3D 11 windows in one application. Each backend manages its own native
resources; this does not depend on sharing GPU handles between APIs.

The frame pipeline separates resource preparation and variable capture from
draw execution. After the snapshot handoff, the application can prepare the
next frame while the render thread executes the current one. The backends also
compile shaders on support threads and apply the results on the render thread.
This is not a promise that arbitrary concurrent changes are safe: borrowed
resources and mutable data still follow the frame and ownership contracts.

The graphics layer includes vertex/index buffers, textures and render targets,
variable sets, render-state push/pop, geometry shaders, buffer-based vertex
pulling, and transform feedback / stream output. The latter supports GPU-side
update and filtering passes whose output can be reused for rendering.

## motor Shading Language

msl means **motor shading language**, not Metal Shading Language. It translates
shader configurations into GLSL and HLSL for the active rendering backends.
Configurations can contain vertex, geometry, and pixel stages, with reusable
library functions and built-ins such as procedural noise.

Bindings such as `world`, `view`, `projection`, `light_color`, and
`light_intensity` let scene-side code supply values without application-specific
uniform lookup code at every draw. Shader variables are grouped in variable sets;
the backends handle their API-specific layout and upload.

Recompilation is integrated with graphics objects and scene components.
Compilation listeners let the scene refresh its variable bindings when a shader
changes. Names and resource lifetimes remain part of the application's contract;
shader sources are not automatically deduplicated across unrelated projects.

An [Emacs major mode](tools/emacs/README.md) provides highlighting for msl types,
bindings, and built-ins.

## Scene Graph and Data Flow

The scene graph combines nodes, components, and visitors for transformations,
animation, cameras, lights, variable updates, and rendering. It is an application
interface to the runtime, not a requirement for using the lower-level libraries.

An `msl_component` keeps a base `render_data_set` and creates additional sets for
individual render IDs. Each set holds a graphics variable set, resolved bindings,
and a wire input bridge. Output-to-input bridge connections propagate base values
to the additional sets, while visitors provide the camera and light values for
each render. This allows the same geometry to participate in multiple views and
passes without repeatedly overwriting one shared set of draw variables.

The `wire` system provides typed input/output slots and explicit data propagation.
Its bridges connect that data flow to shader variables. Transform and light
collection visitors also make imported, animated lights usable by lighting passes.

## Assets and Debugging

- Asset imports return futures and use the `io` database and location system.
  Blender-authored glTF scenes can supply geometry, material data, cameras,
  animation, and lights. Loading an asset and configuring its GPU resources are
  separate steps.
- The `collide` library provides sphere, AABB, OBB, ray, point, and frustum tests.
  Frustum/volume classification uses conservative plane tests: it can retain an
  object near a frustum corner even when the volumes do not actually overlap.
- Primitive renderers can draw debugging geometry and frusta. The font renderer
  provides text independently of ImGui; sample applications use it for projected
  labels on 3D objects.
- The memory manager tracks allocations and can report outstanding allocations.
  Optional memory observation and CPU profiling are enabled through
  `MOTOR_MEMORY_OBSERVER` and `MOTOR_PROFILING`. These diagnostics help investigate
  behavior but do not replace ownership and synchronization discipline.
- ImGui integration provides application controls and runtime inspection tools.

## Project Structure

The repository is split into small libraries. Important modules include:

- `application`, `platform` - callbacks, windows, and native platform/backend integration
- `graphics` - backend-facing graphics objects, frontend command layer, render state and resource handling
- `gfx` - cameras, lights, primitive/text/sprite renderers, and post-processing
- `scene` - scene graph, components, visitors, animation and graphics integration
- `msl` - motor shading language and shader translation infrastructure
- `wire` - typed slots and data-flow connections
- `math`, `geometry`, `noise`, `collide` - math, geometry generation, noise, and intersection tests
- `concurrent` - task and scheduling utilities
- `io`, `format` - file/database access and asynchronous asset import
- `audio`, `controls` - audio and input integration
- `memory`, `log`, `profiling` - allocation tracking and runtime diagnostics
- `core` - shared utilities, including token-oriented document processing
- `tool`, `property` - runtime tooling and editable data interfaces

Third-party dependencies live under `externals/` and are pulled in as git submodules. The engine integrates them through local CMake targets instead of modifying the external source code.

## Related Repositories

The engine repository intentionally does not contain all demos and visual test applications.

- [`motor_suites`](https://github.com/aconstlink/motor_suites) contains sample applications, integration checks, and manual/visual test scenarios.
- [`motor_ai_suites`](https://github.com/aconstlink/motor_ai_suites) contains AI-assisted regression tests, benchmarks, and visual applications, with verification notes alongside the tests.
- `motor_demos` is used for demo-focused applications built on top of the engine.

This keeps the engine repository focused while still allowing larger examples and experiments to evolve separately.

The ai suites include a procedural village, GPU particle/sculpture tests,
asynchronous asset loading, render-state comparisons, font/debug labels, and
two-camera frustum culling. Windowless checks and interactive graphics tests
cover different behavior; passing a CPU test alone does not validate rendered
pixels. Benchmark results are documented with their workload and configuration,
not treated as a general ranking of engines.

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

Windows is the primary development platform. OpenGL 4 and Direct3D 11 are used
there; Linux uses the X11/GLX OpenGL path. Older OpenGL ES code exists in the tree
but is not currently validated to the same level as these desktop paths.

## Install Package

Build and install a selected configuration into a separate directory:

```bash
cmake --build build --config Release
cmake --install build --config Release --prefix path/to/motor-package
```

Point `CMAKE_PREFIX_PATH` at that directory when configuring a consuming project.
The installed package provides individual module targets and `motor::complete`:

```cmake
find_package(motor 0.0.2 EXACT CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE motor::complete)
```

For a smaller dependency set, link the required modules, such as `motor::math`
or `motor::graphics`, instead. The targets propagate their include directories,
compile definitions, dependencies, and the C++20 requirement.

The package currently uses exact version matching; compatibility between different
motor versions is not promised. Binary packages must match the consumer's platform,
architecture, compiler/runtime, and configuration. Shared libraries must also be
available to the runtime loader (`bin` on Windows, `lib` on Linux). System graphics
development dependencies, such as OpenGL, are resolved on the consuming machine.

## Status

Active development.

The current work is centered on building real-time demos and using them as
integration tests for the engine: Blender scene import, camera sequencing,
animation, scene streaming, lighting, shadows, and post-processing. Recent work
has also focused on frame overlap, targeted thread wakeups, backend state caching,
resource reconfiguration, and debug visualization.

APIs and internal systems are still evolving. The repository is best understood as an engine development project and research runtime, not as a finished SDK.

## AI-Assisted Development

motor is a long-running, independently developed C++ engine.

I use AI to support development, not to pile on features I don’t understand or hand over control of motor. The engine’s architecture, direction, and the changes I accept remain my responsibility.

I mainly use AI to write tests, investigate bugs, and help implement targeted fixes. Understanding and fixing problems matters, but spending an entire evening tracking down a bug isn’t inherently more valuable than finding it sooner and moving forward.

Using AI has genuinely revitalized the development of motor. With limited time available, it lets me spend more of that time coding, experimenting, and building the things I actually want to create.

## License

MIT License.
