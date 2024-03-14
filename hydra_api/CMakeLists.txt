cmake_minimum_required(VERSION 3.7)
project(hydra_api C CXX)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        HydraAPI.cpp
        HRMeshCommon.cpp
        HRTextureCommon.cpp
        HydraAPI_Camera.cpp
        HydraAPI_FrameBuffer.cpp
        HydraAPI_Geom.cpp
        HydraAPI_GBuffer.cpp
        HydraAPI_Light.cpp
        HydraAPI_LoadExistingLibrary.cpp
        HydraAPI_Material.cpp
        HydraAPI_Texture.cpp
        HydraAPI_TextureProcLex.cpp
        HydraAPI_EXR.cpp
        ../dependencies/include/miniz.c
        HydraTextureUtils.cpp
        HydraLegacyUtils.cpp
        HydraDriverUpdate.cpp
        HydraObjectManager.cpp
        HydraRenderDriverAPI.cpp
        HydraRngUtils.cpp
        VirtualBuffer.cpp
        xxhash.c
        cube2sphere.cpp

        HydraFsUtils.cpp

        RenderDriverHydraConnection.cpp
        RenderDriverDebugPrint.cpp
        RenderDriverHydraCore3.cpp

        HydraXMLHelpers.cpp
        HydraXMLVerify.cpp
        pugixml.cpp

        ssemath.cpp
        HR_HDRImage4f.cpp
        HR_HDRImageTool.cpp

        HydraAPI_GeomProcessing.cpp
        HydraVSGFExport.cpp
        HydraVSGFCompress.cpp
        cmesh.cpp
        cmesh_mikey_connect.cpp
        cmesh_processing_weld.cpp
        cmesh_processing.cpp)

set(GL_DEPENDENT
        glad.c
        OpenGLCoreProfileUtils.h OpenGLCoreProfileUtils.cpp
        RenderDriverOpenGL3_Utility.h RenderDriverOpenGL3_Utility.cpp)

if (WIN32)
    set(SOURCE_FILES ${SOURCE_FILES} SystemWin.cpp
                                     HR_AccumImageWin.cpp
                                     HydraLaunchProcessWin32.cpp)
    set(GL_DEPENDENT ${GL_DEPENDENT} OpenGLContextWin.cpp)
else()
	set(SOURCE_FILES ${SOURCE_FILES} SystemUnix.cpp
                                     HR_AccumImageLinux.cpp
                                     HydraLaunchProcessLinux.cpp)
endif()


set(HR_EXTENSIONS HRExtensions.cpp HRExtensions_Spectral.cpp)

set(VEC_TEX_EXTENSION HRExtensions_VectorTex.cpp)

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
#find_package(FreeImage REQUIRED)
#include_directories(${FREEIMAGE_INCLUDE_DIRS})
#set(LIBS ${LIBS} ${FREEIMAGE_LIBRARIES})

message(STATUS "HAPI USE_GL = ${USE_GL}")
if(USE_GL)
    add_compile_definitions(USE_GL)
    cmake_policy(SET CMP0072 OLD)
    find_package(OpenGL REQUIRED)
    set(SOURCE_FILES ${SOURCE_FILES} ${GL_DEPENDENT})
    include_directories (${OPENGL_INCLUDE_DIR})
endif()

if(VECTOR_TEX_SUPPORT)
	include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/msdfgen/include)
	include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/nanosvg)
	include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/skia/headers)
	include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/skia/headers/include)
	include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/skia/headers/modules)
	include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/skia/headers/src)
	include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/skia/headers/third_party)
	
    set(SOURCE_FILES ${SOURCE_FILES} ${VEC_TEX_EXTENSION})
endif()

find_package (Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)


#build hydra API static library
add_library(hydra_api STATIC ${SOURCE_FILES} ${HR_EXTENSIONS})
target_include_directories (hydra_api PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${ADDITIONAL_INCLUDE_DIRS})

#target_link_libraries(hydra_api freeimage)
#add_dependencies(hydra_api freeimage)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DIN_DEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1 -mavx")

if(WIN32)
  add_definitions(-DUNICODE -D_UNICODE)
#  target_link_libraries(hydra_api LINK_PUBLIC ${LIBS} ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY} corto ies_parser mikttspace)
else()
  set_property(TARGET hydra_api PROPERTY POSITION_INDEPENDENT_CODE ON)

  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang") # enable sanitize youself if needed
      #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory -fno-omit-frame-pointer -fsanitize=leak)
      #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fsanitize=leak -fsanitize=undefined")
      MESSAGE( STATUS "WE USE CLANG(HydraAPI);")
  else()                                     # enable sanitize youself if needed
      #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fsanitize=leak -fsanitize=undefined -fsanitize=bounds-strict")
      MESSAGE( STATUS "WE USE GCC  (HydraAPI);")
  endif()

endif()

MESSAGE( STATUS "CMAKE_CXX_FLAGS(HydraAPI): " ${CMAKE_CXX_FLAGS})