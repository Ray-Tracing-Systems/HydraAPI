cmake_minimum_required(VERSION 3.7)
project(hydra_api_py)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
        ../alloc16.h
        ../HR_HDRImage.h
        ../HR_HDRImage4f.cpp
        ../HRMeshCommon.cpp
        ../HRTextureCommon.cpp
        ../HydraAPI.cpp
        ../HydraAPI.h
        ../HydraAPI_Camera.cpp
        ../HydraAPI_FrameBuffer.cpp
        ../HydraAPI_Geom.cpp
        ../HydraAPI_Light.cpp
        ../HydraAPI_LoadExistingLibrary.cpp
        ../HydraAPI_Material.cpp
        ../HydraAPI_Texture.cpp
        ../HydraDriverUpdate.cpp
        ../HydraInternal.h
        ../HydraInternalCommon.h
        ../HydraObjectManager.cpp
        ../HydraObjectManager.h
        ../HydraRenderDriverAPI.h
        ../HydraVSGFExport.cpp
        ../HydraVSGFExport.h
        ../HydraXMLHelpers.h
        ../HydraXMLVerify.cpp
        ../HydraXMLVerify.h
        ../LiteMath.h
        ../pugiconfig.hpp
        ../pugixml.cpp
        ../pugixml.hpp
        ../RenderDriverDebugPrint.cpp
        ../RenderDriverOpenGL1.cpp
        ../RenderDriverOpenGL1.h
        ../RenderDriverOpenGL1_DelayedLoad.cpp
        ../RenderDriverOpenGL1_DrawBvhLevels.cpp
        ../RenderDriverOpenGL1_TestSplit.cpp
        ../RenderDriverOpenGL1Debug.cpp
        ../RenderDriverOpenGL1DebugDrawRays.cpp
        ../SystemUnix.cpp
        ../VirtualBuffer.cpp
        ../xxhash.c
        ../xxhash.h
	    ../SystemUnix.cpp)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

find_package(FreeImage REQUIRED)
include_directories(${FREEIMAGE_INCLUDE_DIRS})
set(LIBS ${LIBS} ${FREEIMAGE_LIBRARIES})

find_package(OpenGL REQUIRED)

#build python bindings shared library

add_subdirectory(pybind11)
pybind11_add_module(hydra_api_py ${SOURCE_FILES} ../hydraPy.cpp)

target_include_directories (hydra_api_py PUBLIC .. ${OPENGL_INCLUDE_DIR})
target_link_libraries(hydra_api_py LINK_PUBLIC ${LIBS} ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY} stdc++fs rt)
