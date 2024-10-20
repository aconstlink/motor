# motor

[![CMake on Linux with OpenGL](https://github.com/aconstlink/motor/actions/workflows/cmake-lin-gcc-gl.yml/badge.svg)](https://github.com/aconstlink/motor/actions/workflows/cmake-lin-gcc-gl.yml)
[![CMake on Win32 with DirectX 11](https://github.com/aconstlink/motor/actions/workflows/cmake-win32-dx11.yml/badge.svg)](https://github.com/aconstlink/motor/actions/workflows/cmake-win32-dx11.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

motor is a software framework for real-time interactive audio/visual applications and is a personal endeavour into real-time engine coding. 

## Sample Code

The engine itself does not contain any sample code! The purpose of this is to keep the codebase clean from clutter and rubbish. It also unnecessarily bloats the codebase with unused code if the engine itself is included in another project.

Please see the following repos for samples:
[Base tests](https://github.com/aconstlink/motor_suites) includes sample code and helps me test the engine. This repo also shows how to include the engine as a subrepository.

## Platform Prerequisites

### Windows:
 - Windows 10 SDK
 - Visual Studio 2019 (Community) or later
 - CMake
 
 The engine should compile for Windows 8 and Windows 7 still. There are some differences in the Windows SDKs which I might not support anymore.

### Linux:

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


