# HydraAPI
The Hydra Renderer consists of 3 heads:

- End User Plugin (3ds max or else)
- HydraAPI (infrastructure)
- HydraCore (render engine, compute core)

This repo contain the second one.

# Usage

Windows:
1. Clone HydraAPI repo
2. Copy 'bin2' folder content to 'C:/[Hydra]/bin2' under windows;
3. Open 'hydra_api/HydraNewAPI1.sln' under windows with visual studio 2015 or later; If use 2015 you will need to downgrade platform toolset to v140 (2015).
4. Select x64 configuration
5. Set 'main' as startup project.
6. Now you can try 'HydraModern' with some tests. Try 'test39_mesh_from_vsgf' for example. It is simple cornell box test.

Linux:
1. Clone HydraAPI repo
2. Currently main engine is not yet ported to Linux, so do nothing.
3. use Cmake;
4. use OpenGL1 or OpenGL3 render drivers. try some interactive 'main_window' tests.

# License and dependency

Hydra API uses MIT licence itself, however it depends on the other software as follows (see doc/licence directory):

* 02 - FreeImage Public License - Version 1.0 (FreeImage is used in the form of binaries)
* 03 - Embree Apache License 2.0 (Embree is used in the form of binaries)
* 04 - xxhash BSD 3-clause "New" or "Revised" (xxhash is used in the form of sources)
* 05 - pugixml MIT licence (pugixml is used in the form of sources)
* 06 - clew Boost Software License - Version 1.0 - August 17th, 2003 (clew is used in the form of sources)
* 07 - IESNA MIT-like licence (IESNA used in the form of sources)
* 08 - glad MIT licence (glad is used in form of generated source code).
* 09 - glfw BSD-like license (glfw is used in form of binaries only for demonstration purposes).

Most of them are simple MIT-like-licences without any serious restrictions. 
So in general there should be no problem to use HydraAPI in your open source or commertial projects. 

However if you find that for some reason you can't use one of these components, please let us know!
Most of these components can be replaced.
