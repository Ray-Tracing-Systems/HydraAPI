# HydraAPI
The Hydra Renderer consists of 3 heads:

- End User Plugin (3ds max or else)
- HydraAPI (infrastructure)
- HydraCore (render engine, compute core)

This repo contain the second one.

![](image.jpg)
<p align="center">An example of our renderer in 3ds max</p>

# Usage

Windows:

1. Clone HydraAPI repo
2. Copy 'bin2' folder content to 'C:/[Hydra]/bin2' under windows;
3. Open 'hydra_api/HydraNewAPI1.sln' under windows with visual studio 2015 or later; If use 2015 you will need to downgrade platform toolset to v140 (2015).
4. **Select x64 configuration**
5. Set 'main' as startup project.
6. Now you can run some demo and tests

Linux:
1. Clone HydraAPI repo
2. sudo apt-get install libfreeimage-dev
3. sudo apt-get install mesa-common-dev libglu1-mesa-dev libglfw3-dev libglfw3
4. sudo apt install ocl-icd-opencl-dev
5. use Cmake;
6. build and install HydraCore using Cmake and approptiate instruction

Optionally, to build and use python bindings library (tested only under Linux):
1. Get pybind11 as submodule (HydraAPI root dir): 
- git submodule init
- git submodule update
2. sudo apt-get install python3-dev (install Python headers)
3. Build hydra bindings with Cmake (hydra_api/hydra_api_py)
 
 * mkdir build (to create "hydra_api/hydra_api_py/build")
 * cd build
 * select specific python version specify CMake variable: **cmake .. -DPYTHON_EXECUTABLE=/path/to/your/python3**
 
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
So in general there should be no problem to use HydraAPI in your open source or commertial projects. 

However if you find that for some reason you can't use one of these components, please let us know!
Most of these components can be replaced.

# Acknowlegments
This project is supported by RFBR 16-31-60048 "mol_a_dk".
