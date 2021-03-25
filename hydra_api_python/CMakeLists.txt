cmake_minimum_required(VERSION 3.7)
project(hydra_api_python C CXX)

set(CMAKE_CXX_STANDARD 14)


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

find_package(OpenGL REQUIRED)


include_directories(${ADDITIONAL_INCLUDE_DIRS})


#set(PYBIND11_PYTHON_VERSION 3.6)

add_subdirectory(pybind11)
pybind11_add_module(hydra_api_python HydraPy.cpp) #

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")

target_include_directories (hydra_api_python PUBLIC .. ${OPENGL_INCLUDE_DIR})

#build python bindings shared library
if(WIN32)
	add_definitions(-DUNICODE -D_UNICODE)
	target_link_libraries(hydra_api_python LINK_PUBLIC ${LIBS} ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY})
else()

  set(THREADS_PREFER_PTHREAD_FLAG ON)
	target_link_libraries(hydra_api_python LINK_PUBLIC ${LIBS}
					${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY}
					${HYDRA_API_LIB} ${HYDRA_API_IES_PARSER_LIB} ${HYDRA_API_MIKKTSPACE} ${HYDRA_API_CORTO}
					stdc++fs rt dl OpenCL glfw)
endif()
