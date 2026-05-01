# motor

[![CMake on Linux with OpenGL](https://github.com/aconstlink/motor/actions/workflows/cmake-lin-gcc-gl.yml/badge.svg)](https://github.com/aconstlink/motor/actions/workflows/cmake-lin-gcc-gl.yml)
[![CMake on Win32 with DirectX 11](https://github.com/aconstlink/motor/actions/workflows/cmake-win32-dx11.yml/badge.svg)](https://github.com/aconstlink/motor/actions/workflows/cmake-win32-dx11.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

motor is a custom C++ engine for real-time audio/visual applications, designed for performance, explicit control, and fast iteration.

## ✨ Key Features

### 🎮 Rendering

* Multi-backend rendering:

  * OpenGL 4
  * DirectX 11
  * GLES 3
* Cross-backend shader system (GLSL / HLSL mapping)
* Runtime shader compilation & hot-reload
* Multiple renderers active simultaneously (debug & comparison)

### ⚡ Runtime & Systems

* Custom job system (dependency-based, parallel execution)
* Explicit resource & memory management
* Deterministic ownership model (managed vs borrowed pointers)
* Scene graph & runtime object management

### 🔁 Iteration Speed

* Live shader hot-reload (async compilation)
* Asset reload at runtime (file watcher integration)
* No restart required for most changes
* Immediate-mode tooling via Dear ImGui integration

### 🧠 Memory System

* Central memory manager with:

  * allocation tracking
  * purpose tagging
  * full runtime visibility
* Explicit ownership control (`*_mtr_t` vs `*_ptr_t`)
* Leak detection via memory dump

### 🧱 Architecture Highlights

* Layered system design (top → down dependencies only)
* Clean separation between:

  * runtime systems
  * rendering backends
  * tooling
* CMake-based build with target-driven dependencies
* CI-enabled, builds out-of-the-box

### 🛠 Tooling

* Built-in tooling layer:

  * implement `on_tool()` to instantly add UI
* Dear ImGui fully integrated across backends
* Multi-window debug setup (tooling, debug, production)

--- 

## 🎯 Design Philosophy

motor is built around a few core principles:

* **Control over abstraction**
* **Explicit ownership and lifetime management**
* **Fast iteration loops**
* **Minimal hidden behavior**

The goal is not to replace existing engines, but to deeply understand and control the systems behind them.

--- 

## 🚀 Why this project

This project explores:

* rendering abstraction across APIs
* runtime system design in C++
* memory & resource management strategies
* real-time iteration workflows

It serves as both a **learning platform** and a **proof of system-level engineering capability**.

---

## 📌 Status

Active development.
The current focus is on completing a polished real-time demo showcasing the engine’s capabilities.

--- 

## 📚 Sample Code & Related Repositories

This repository intentionally **does not include sample code**.

The goal is to keep the engine codebase focused, clean, and free from unrelated or experimental code. Embedding samples directly into the engine would introduce unnecessary clutter and make integration into other projects more difficult.

Instead, examples and usage are maintained in separate repositories:

### 🧪 Base Tests

https://github.com/aconstlink/motor_suites

* Contains sample applications and test scenarios
* Used to validate engine features and workflows
* Demonstrates how to include the engine as a Git subrepository

### 📦 External Dependency Setup (WIP)

https://github.com/aconstlink/motor_exdep

* Shows how to use the engine as an external dependency
* Uses CMake package configurations
* Supports integration via build tree or install tree

### 🎬 Demo Focus (Work in Progress)

The engine is currently used to build a **real-time demo** showcasing:

* seamless scene streaming (no visible hitches)
* async shader pipeline
* runtime resource loading
* multi-backend rendering consistency

https://github.com/aconstlink/motor_demos

Collection of real-time demos built with the engine
Showcases rendering, runtime systems, and workflows in practice

This separation keeps the engine modular and allows each repository to focus on a specific purpose:
**core engine**, **testing**, **integration**, and **demonstration**.

--- 

## 🖥 Platforms

### Windows

Windows has the main development focus.

### Prerequisites

* Windows 10/11 SDK
* Visual Studio 2019 (Community) or later
* CMake

In newer Version of VS, the SDK can just be installed along with the VS setup.
You should not have to install anything other then whats included in VS for this project.

### Linux

The engine is also supported and regularly tested on Fedora/WSL/Ubuntu.

---

## 🧩 Prerequisites (Linux)

### 🔧 Development Dependencies

```bash
libX11-devel libGLES-devel libGL-devel libEGL-devel alsa-lib-devel make cmake gcc gcc-c++
```

### ▶️ Runtime Dependencies

```bash
libX11 mesa-libGLES mesa-libGL mesa-libEGL alsa-lib
```

### 🖥 Tested Environment

* Fedora 30+ (regularly tested)
* OpenGL 4.x

> Note: OpenGL ES support is currently not working.
> 
--- 

## 📦 Build

```bash
git clone https://github.com/aconstlink/motor.git
cd motor
mkdir build && cd build
cmake ..
cmake --build .
```
## 📄 License

MIT License
