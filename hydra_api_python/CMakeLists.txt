cmake_minimum_required(VERSION 3.7)
project(hydraPy C CXX)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(HYDRA_API_CMAKE)
set(USE_GL OFF CACHE BOOL "Build OpenGL render drivers")

set(ADDITIONAL_INCLUDE_DIRS
		${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/include)

set(ADDITIONAL_LIBRARY_DIRS
		${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/lib_x64
		${CMAKE_CURRENT_SOURCE_DIR}/../bin)

set(HYDRA_API_ROOT_DIR ${CMAKE_SOURCE_DIR}/..)
set(HYDRA_API_SRC_DIR ${HYDRA_API_ROOT_DIR}/hydra_api)
set(HYDRA_API_LIB_DIR ${HYDRA_API_ROOT_DIR}/bin)
set(HYDRA_API_UTILS_SRC_DIR ${HYDRA_API_ROOT_DIR}/utils)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
find_package(FreeImage REQUIRED)
include_directories(${FREEIMAGE_INCLUDE_DIRS} ${HYDRA_API_SRC_DIR})

set(GL_DEPENDENT
	${HYDRA_API_SRC_DIR}/glad.c
	${HYDRA_API_SRC_DIR}/OpenGLCoreProfileUtils.h 
	${HYDRA_API_SRC_DIR}/OpenGLCoreProfileUtils.cpp
	${HYDRA_API_SRC_DIR}/RenderDriverOpenGL3_Utility.cpp)

set(HAPI_SOURCE_FILES
	${HYDRA_API_SRC_DIR}/HydraAPI.cpp
	${HYDRA_API_SRC_DIR}/HRMeshCommon.cpp
	${HYDRA_API_SRC_DIR}/HRTextureCommon.cpp
	${HYDRA_API_SRC_DIR}/HydraAPI_Camera.cpp
	${HYDRA_API_SRC_DIR}/HydraAPI_FrameBuffer.cpp
	${HYDRA_API_SRC_DIR}/HydraAPI_Geom.cpp
	${HYDRA_API_SRC_DIR}/HydraAPI_GBuffer.cpp
	${HYDRA_API_SRC_DIR}/HydraAPI_Light.cpp
	${HYDRA_API_SRC_DIR}/HydraAPI_LoadExistingLibrary.cpp
	${HYDRA_API_SRC_DIR}/HydraAPI_Material.cpp
	${HYDRA_API_SRC_DIR}/HydraAPI_Texture.cpp
	${HYDRA_API_SRC_DIR}/HydraAPI_TextureProcLex.cpp
	${HYDRA_API_SRC_DIR}/HydraAPI_EXR.cpp
	${HYDRA_API_SRC_DIR}/../dependencies/include/miniz.c
	${HYDRA_API_SRC_DIR}/HydraTextureUtils.cpp
	${HYDRA_API_SRC_DIR}/HydraLegacyUtils.cpp
	${HYDRA_API_SRC_DIR}/HydraDriverUpdate.cpp
	${HYDRA_API_SRC_DIR}/HydraObjectManager.cpp
	${HYDRA_API_SRC_DIR}/HydraRenderDriverAPI.cpp
	${HYDRA_API_SRC_DIR}/HydraRngUtils.cpp
	${HYDRA_API_SRC_DIR}/VirtualBuffer.cpp
	${HYDRA_API_SRC_DIR}/xxhash.c
	${HYDRA_API_SRC_DIR}/cube2sphere.cpp

	${HYDRA_API_SRC_DIR}/HydraFsUtils.cpp

	${HYDRA_API_SRC_DIR}/RenderDriverHydraConnection.cpp
	${HYDRA_API_SRC_DIR}/RenderDriverDebugPrint.cpp

	${HYDRA_API_SRC_DIR}/HydraXMLHelpers.cpp
	${HYDRA_API_SRC_DIR}/HydraXMLVerify.cpp
	${HYDRA_API_SRC_DIR}/pugixml.cpp

	${HYDRA_API_SRC_DIR}/ssemath.cpp
	${HYDRA_API_SRC_DIR}/HR_HDRImage4f.cpp
	${HYDRA_API_SRC_DIR}/HR_HDRImageTool.cpp

	${HYDRA_API_SRC_DIR}/HydraAPI_GeomProcessing.cpp
	${HYDRA_API_SRC_DIR}/HydraVSGFExport.cpp
	${HYDRA_API_SRC_DIR}/HydraVSGFCompress.cpp
	${HYDRA_API_SRC_DIR}/cmesh.cpp
	${HYDRA_API_SRC_DIR}/cmesh_mikey_connect.cpp
	${HYDRA_API_SRC_DIR}/cmesh_processing_weld.cpp
	${HYDRA_API_SRC_DIR}/cmesh_processing.cpp)

if(WIN32)
	set(HAPI_SOURCE_FILES ${HAPI_SOURCE_FILES}
			${HYDRA_API_SRC_DIR}/SystemWin.cpp
			${HYDRA_API_SRC_DIR}/HR_AccumImageWin.cpp)

	set(HAPI_SOURCE_FILES ${HAPI_SOURCE_FILES}
			${HYDRA_API_SRC_DIR}/HydraLaunchProcessWin32.cpp)
	
	set(GL_DEPENDENT ${GL_DEPENDENT} OpenGLContextWin.cpp)

	SET(LIB_SOURCES
			${HYDRA_API_UTILS_SRC_DIR}/corto/bitstream.cpp
			${HYDRA_API_UTILS_SRC_DIR}/corto/color_attribute.cpp
			${HYDRA_API_UTILS_SRC_DIR}/corto/cstream.cpp
			${HYDRA_API_UTILS_SRC_DIR}/corto/decoder.cpp
			${HYDRA_API_UTILS_SRC_DIR}/corto/encoder.cpp
			${HYDRA_API_UTILS_SRC_DIR}/corto/normal_attribute.cpp
			${HYDRA_API_UTILS_SRC_DIR}/corto/tunstall.cpp

			${HYDRA_API_UTILS_SRC_DIR}/clew/clew.c

			${HYDRA_API_UTILS_SRC_DIR}/ies_parser/IESNA.C
			${HYDRA_API_UTILS_SRC_DIR}/ies_parser/IES_READ.C

			${HYDRA_API_UTILS_SRC_DIR}/mikktspace/mikktspace.c)
	include_directories("c:/Python310/include")
	set(PYTHON_EXECUTABLE "c:/Python310/Python")
else()

set(HAPI_SOURCE_FILES ${HAPI_SOURCE_FILES}
			${HYDRA_API_SRC_DIR}/SystemUnix.cpp
			${HYDRA_API_SRC_DIR}/HR_AccumImageLinux.cpp
			${HYDRA_API_SRC_DIR}/HydraLaunchProcessLinux.cpp)

	set(LIBS ${LIBS} ${FREEIMAGE_LIBRARIES})
	find_library(HYDRA_API_IES_PARSER_LIB ies_parser HINTS ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
	find_library(HYDRA_API_MIKKTSPACE mikktspace HINTS ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
	find_library(HYDRA_API_CORTO corto HINTS ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
	find_package(glfw3 REQUIRED)
	include_directories(${GLFW_INCLUDE_DIRS})

#	set(PYTHON_EXECUTABLE /home/vs/Software/Python-3.9.7/Python)
	include_directories(/home/vs/Software/Python-3.9.7/Include)
	set(LIBS ${LIBS} ${GLFW_LIBRARIES})
endif()

set(HR_EXTENSIONS HRExtensions.cpp HRExtensions_Spectral.cpp)

set(VEC_TEX_EXTENSION HRExtensions_VectorTex.cpp)

message(LIBS = [${LIBS}])

message(STATUS "HAPI USE_GL = ${USE_GL}")
if(USE_GL)
    add_compile_definitions(USE_GL)
    cmake_policy(SET CMP0072 OLD)
    find_package(OpenGL REQUIRED)
    set(HAPI_SOURCE_FILES ${HAPI_SOURCE_FILES} ${GL_DEPENDENT})
    include_directories (${OPENGL_INCLUDE_DIR})
endif()

include_directories(${ADDITIONAL_INCLUDE_DIRS})

# set(PYBIND11_PYTHON_VERSION 3.7)

add_subdirectory(pybind11)

if(WIN32)
	add_definitions(-DUNICODE -D_UNICODE)
	pybind11_add_module(hydraPy ${HAPI_SOURCE_FILES} ${LIB_SOURCES} HydraPy.cpp)
else()

	pybind11_add_module(hydraPy HydraPy.cpp ${HAPI_SOURCE_FILES})
endif()


#${HYDRA_API_SRC_DIR}/pugixml.cpp
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")

target_include_directories (hydraPy PUBLIC .. ${OPENGL_INCLUDE_DIR})

#build python bindings shared library
if(WIN32)
	add_definitions(-DUNICODE -D_UNICODE)
	target_link_libraries(hydraPy LINK_PUBLIC ${LIBS} ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY})
else()

	set(THREADS_PREFER_PTHREAD_FLAG ON)
	target_link_libraries(hydraPy LINK_PUBLIC
			${LIBS} ${HYDRA_API_IES_PARSER_LIB} ${HYDRA_API_MIKKTSPACE} ${HYDRA_API_CORTO}
			stdc++fs rt dl OpenCL glfw
			)
endif()