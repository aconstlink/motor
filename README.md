# motor

[![CMake on Linux with OpenGL](https://github.com/aconstlink/motor/actions/workflows/cmake-lin-gcc-gl.yml/badge.svg)](https://github.com/aconstlink/motor/actions/workflows/cmake-lin-gcc-gl.yml)
[![CMake on Win32 with DirectX 11](https://github.com/aconstlink/motor/actions/workflows/cmake-win32-dx11.yml/badge.svg)](https://github.com/aconstlink/motor/actions/workflows/cmake-win32-dx11.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

motor is a software framework for real-time interactive audio/visual applications and is a personal endeavour into real-time engine coding. 

Sample code using the engine can be found in the [sample suites](https://github.com/aconstlink/motor_suites) repository. The engine itself does not contain any sample code! 

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


