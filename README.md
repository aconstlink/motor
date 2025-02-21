# motor

[![CMake on Linux with OpenGL](https://github.com/aconstlink/motor/actions/workflows/cmake-lin-gcc-gl.yml/badge.svg)](https://github.com/aconstlink/motor/actions/workflows/cmake-lin-gcc-gl.yml)
[![CMake on Win32 with DirectX 11](https://github.com/aconstlink/motor/actions/workflows/cmake-win32-dx11.yml/badge.svg)](https://github.com/aconstlink/motor/actions/workflows/cmake-win32-dx11.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

motor is a software framework for real-time interactive audio/visual applications and is a personal endeavour into real-time engine coding. 

## About 
This repository contains my experimental game and animation engine for real-time applications called motor. The philosophy shoud be an not too overloaded experience. It provides the user with some UI but the majority of any project done with motor is to code! Sure, some taks require an UI for simplicity but other just don't and are more easily and quickly achievable using plain old code.

This repository is kept clean from any sample code. It is just the engine.

## Sample Code

The engine repo itself does not contain any sample code! The purpose here is to keep the codebase clean from clutter and rubbish. Sample code also unnecessarily bloats the codebase with unused code if the engine itself is included in another project.

Please see the following repos for samples:

[Base tests](https://github.com/aconstlink/motor_suites) includes sample code and helps me test the engine and remember how this thing actually works, because its already a lot! That repo also shows how to include the engine as a subrepository.

## Platforms 

### Windows
Windows has my main focus.

#### Prerequisits :
 - Windows 10 SDK
 - Visual Studio 2019 (Community) or later
 - CMake
 
 The engine should compile for Windows 8 and Windows 7 still. There are some differences in the Windows SDKs which I might not support anymore.

### Linux:

The engine should compile and sample code should run on Linux but not every feature may work. I try to make that work though. My main distributions are Ubuntu and Fedora.

#### Prerequisits :

*For development:*
```bash
libX11-devel libGLES-devel libGL-devel libEGL-devel alsa-lib-devel make cmake gcc gcc-c++
```
*For execution only*
```bash
libX11 mesa-libGLES mesa-libGL mesa-libEGL alsa-lib
```

I regularly test on a Fedora 30+ machine with OpenGL 4. OpenGL ES is currently not working.


## Clone and Build

This project onyl supports CMake.

**Windows Visual Studio:**  
1. Open CMake Gui
2. Choose 64 bit Visual Studio 2019(or higher)
3. Generate (out of source)
4. Open Visual Studio Solution
   
**Linux CMake:**  
```
cd repos
git clone https://github.com/aconstlink/motor.git
mkdir _build/motor.platform
cd _build/motor.platform
cmake ../../motor
```


