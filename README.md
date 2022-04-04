# HydraAPI
The Hydra Renderer consists of 3 heads:

- End User Plugin (3ds max or else)
- HydraAPI (infrastructure)
- HydraCore (render engine, compute core)

This repo contains the second one.

![](image.jpg)
<p align="center">An example of our renderer in 3ds max</p>

## Building HydraAPI

1. Clone HydraAPI repo
2. (Linux only) Install OpenCL loader, for example:
```shell
sudo apt install ocl-icd-opencl-dev; 
```
[check this guide if you have problems with OpenCL](doc/opencl_setup_linux.md)
3. To work with images HydraAPI uses FreeImage (ver. 3.1.80) library, we provide prebuilt binaries in 'dependencies/lib_x64_*' directory.
   - You can always try to substitute the prebuilt binaries with your own.
4. If you want to build demos ("main" target) you will need OpenGL. 
   - Set CMake "USE_GL" option to "ON".
   - HydraAPI uses [GLFW](https://github.com/glfw/glfw) (ver. 3.3.6), we provide prebuilt binaries in 'dependencies/lib_x64_*' directory.
     - Set ADDITIONAL_LIBRARY_DIRS CMake variable to other directory if you want.
     - You can always try to substitute the prebuilt binaries with your own.
   - (Linux only) GLFW requires OS-dependent windowing libraries to be installed:
     - on Debian/Ubuntu and derivatives: xorg-dev
     - on Fedora and derivatives: libXcursor-devel libXi-devel libXinerama-devel libXrandr-devel
     - for details check [GLFW website](https://www.glfw.org/docs/latest/compile.html).
5. Use CMake to generate project, for example:
```shell
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_GL=ON ..
```
6. Build HydraAPI using appropriate tools (make, MSVC, etc.)


Alternatively under Windows you can use provided MSVC solution - **"hydra_api/HydraNewAPI1.sln"**.


Optionally, to build and use python bindings library (tested only under Linux):
1. Get pybind11 as submodule (HydraAPI root dir):
```shell
git submodule init
git submodule update
```
2. Install Python headers.
```shell
sudo apt-get install python3-dev
```
3. Build hydra bindings with Cmake (hydra_api/hydra_api_py)
 ```shell
mkdir build
cd build
cmake .. -DPYTHON_EXECUTABLE=/path/to/your/python3
```
4. Import resulting library in your python project

Some examples can be found in hydra_api/hydra_api_py/hydraPyTests.py

# License and dependency

Hydra API uses MIT licence itself, however it depends on the other software as follows (see doc/licence directory):

* 02 - FreeImage Public License - Version 1.0 (FreeImage is used in the form of binaries)
* 04 - xxhash BSD 3-clause "New" or "Revised" (xxhash is used in the form of sources)
* 05 - pugixml MIT licence (pugixml is used in the form of sources)
* 06 - clew Boost Software License - Version 1.0 - August 17th, 2003 (clew is used in the form of sources)
* 07 - IESNA MIT-like licence (IESNA used in the form of sources)
* 08 - glad MIT licence (glad is used in form of generated source code).
* 09 - glfw BSD-like license (glfw is used in form of binaries only for demonstration purposes).
* 10 - pybind11 BSD-style license (used in the form of sources) 
* 11 - corto LGPL3 (used in the form of sources, mesh compression library).

Most of them are simple MIT-like-licences without any serious restrictions. 
So in general there should be no problem to use HydraAPI in your open source or commercial projects. 

However, if you find that for some reason you can't use one of these components, please let us know!
Most of these components can be replaced.

# Acknowledgments
This project is supported by RFBR 16-31-60048 "mol_a_dk" and 18-31-20032 "mol_a_ved".
