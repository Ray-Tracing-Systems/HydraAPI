cmake_minimum_required(VERSION 3.7)
project(hydraPy C CXX)

set(CMAKE_CXX_STANDARD 17)


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
if(WIN32)
	set(LIBS
					${LIBS}
					${FREEIMAGE_LIBRARIES}
					${HYDRA_API_SRC_DIR}/x64/Release/HydraAPI.lib
					${HYDRA_API_SRC_DIR}/x64/Release/ies_parser.lib
					${HYDRA_API_SRC_DIR}/x64/Release/clew.lib
					${HYDRA_API_LIB_DIR}/mikktspace.lib
					${HYDRA_API_LIB_DIR}/corto.lib)
else()
	set(LIBS ${LIBS} ${FREEIMAGE_LIBRARIES})

	find_library(HYDRA_API_LIB hydra_api HINTS ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
	find_library(HYDRA_API_IES_PARSER_LIB ies_parser HINTS ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
	find_library(HYDRA_API_MIKKTSPACE mikktspace HINTS ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
	find_library(HYDRA_API_CORTO corto HINTS ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
	find_package(glfw3 REQUIRED)
	include_directories(${GLFW_INCLUDE_DIRS})
	set(LIBS ${LIBS} ${GLFW_LIBRARIES})
endif()

message(LIBS = [${LIBS}])

if(POLICY CMP0072)
	cmake_policy(SET CMP0072 NEW)
endif()
find_package(OpenGL REQUIRED)


include_directories(${ADDITIONAL_INCLUDE_DIRS})

set(SOURCE_FILES
		${HYDRA_API_SRC_DIR}/HydraAPI.cpp
		${HYDRA_API_SRC_DIR}/cube2sphere.cpp
		${HYDRA_API_SRC_DIR}/HydraLegacyUtils.cpp
		${HYDRA_API_SRC_DIR}/HR_HDRImage4f.cpp
		${HYDRA_API_SRC_DIR}/HR_HDRImageTool.cpp
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
		${HYDRA_API_SRC_DIR}/HydraDriverUpdate.cpp
		${HYDRA_API_SRC_DIR}/HydraObjectManager.cpp
		${HYDRA_API_SRC_DIR}/HydraRenderDriverAPI.cpp
		${HYDRA_API_SRC_DIR}/HydraVSGFExport.cpp
		${HYDRA_API_SRC_DIR}/HydraVSGFCompress.cpp
		${HYDRA_API_SRC_DIR}/HydraXMLVerify.cpp
		${HYDRA_API_SRC_DIR}/RenderDriverHydraConnection.cpp
		${HYDRA_API_SRC_DIR}/RenderDriverDebugPrint.cpp
		${HYDRA_API_SRC_DIR}/ssemath.cpp
		${HYDRA_API_SRC_DIR}/VirtualBuffer.cpp
		${HYDRA_API_SRC_DIR}/xxhash.c
		${HYDRA_API_SRC_DIR}/HydraXMLHelpers.cpp
		${HYDRA_API_SRC_DIR}/HydraTextureUtils.cpp
		${HYDRA_API_SRC_DIR}/HydraAPI_GeomProcessing.cpp
		${HYDRA_API_SRC_DIR}/HydraRngUtils.cpp
		${HYDRA_API_SRC_DIR}/HydraVSGFCompress.cpp
		${HYDRA_API_SRC_DIR}/cmesh.cpp
		${HYDRA_API_SRC_DIR}/cmesh_mikey_connect.cpp
		${HYDRA_API_SRC_DIR}/cmesh_processing_weld.cpp
		${HYDRA_API_SRC_DIR}/cmesh_processing.cpp
		${HYDRA_API_SRC_DIR}/cmesh_vsgf.cpp

		${HYDRA_API_SRC_DIR}/pugixml.cpp

		${HYDRA_API_SRC_DIR}/SystemUnix.cpp
		${HYDRA_API_SRC_DIR}/HR_AccumImageLinux.cpp
		${HYDRA_API_SRC_DIR}/HydraLaunchProcessLinux.cpp

		${HYDRA_API_SRC_DIR}/glad.c
		${HYDRA_API_SRC_DIR}/OpenGLCoreProfileUtils.cpp
		${HYDRA_API_SRC_DIR}/RenderDriverOpenGL3_Utility.cpp
		${HYDRA_API_SRC_DIR}/HydraFsUtils.cpp
		${HYDRA_API_SRC_DIR}/HRExtensions.cpp
		)
#set(PYBIND11_PYTHON_VERSION 3.6)

add_subdirectory(pybind11)
pybind11_add_module(hydraPy HydraPy.cpp ${SOURCE_FILES}) #${HYDRA_API_SRC_DIR}/pugixml.cpp
set(CMAKE_CXX_FLAGS "-g -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")

target_include_directories (hydraPy PUBLIC .. ${OPENGL_INCLUDE_DIR})

#build python bindings shared library
if(WIN32)
	add_definitions(-DUNICODE -D_UNICODE)
	target_link_libraries(hydraPy LINK_PUBLIC ${LIBS} ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY})
else()

  set(THREADS_PREFER_PTHREAD_FLAG ON)
	target_link_libraries(hydraPy LINK_PUBLIC
#			${HYDRA_API_LIB}
#					${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY}
			${LIBS} ${HYDRA_API_IES_PARSER_LIB} ${HYDRA_API_MIKKTSPACE} ${HYDRA_API_CORTO}
			stdc++fs rt dl OpenCL glfw
			)
endif()
