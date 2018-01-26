cmake_minimum_required(VERSION 3.7)
project(hydra_api_py C CXX)

set(CMAKE_CXX_STANDARD 14)


set(ADDITIONAL_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/../../LIBRARY/include)

set(ADDITIONAL_LIBRARY_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/../../LIBRARY/lib_x64
        ${CMAKE_CURRENT_SOURCE_DIR}/../../bin)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Modules")
find_package(FreeImage REQUIRED)
include_directories(${FREEIMAGE_INCLUDE_DIRS})
set(LIBS ${LIBS} ${FREEIMAGE_LIBRARIES} ${CMAKE_SOURCE_DIR}/../../bin/libhydra_api.a ${CMAKE_SOURCE_DIR}/../../bin/libies_parser.a)

find_package(OpenGL REQUIRED)
find_package (Threads REQUIRED)

include_directories(${ADDITIONAL_INCLUDE_DIRS})


#set(PYBIND11_PYTHON_VERSION 3.6)

#build python bindings shared library
if(WIN32)

else()
    find_package (Threads REQUIRED)
    set(THREADS_PREFER_PTHREAD_FLAG ON)

	add_subdirectory(pybind11)
	pybind11_add_module(hydra_api_py HydraPy.cpp) #

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")

	target_include_directories (hydra_api_py PUBLIC .. ${OPENGL_INCLUDE_DIR})
	target_link_libraries(hydra_api_py LINK_PUBLIC ${LIBS} ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY} stdc++fs rt dl OpenCL)
endif()