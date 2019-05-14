cmake_minimum_required(VERSION 3.7)
project(hydra_api_py C CXX)

set(CMAKE_CXX_STANDARD 14)


set(ADDITIONAL_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/../../LIBRARY/include)

set(ADDITIONAL_LIBRARY_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/../../LIBRARY/lib_x64
        ${CMAKE_CURRENT_SOURCE_DIR}/../../bin)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
find_package(FreeImage REQUIRED)
include_directories(${FREEIMAGE_INCLUDE_DIRS})
if(WIN32)
	set(LIBS
					${LIBS}
					${FREEIMAGE_LIBRARIES}
					${CMAKE_SOURCE_DIR}/../../hydra_api/x64/Release/HydraAPI.lib
					${CMAKE_SOURCE_DIR}/../../hydra_api/x64/Release/ies_parser.lib
					${CMAKE_SOURCE_DIR}/../../hydra_api/x64/Release/clew.lib
					${CMAKE_SOURCE_DIR}/../../bin/mikktspace.lib
					${CMAKE_SOURCE_DIR}/../../bin/corto.lib)
else()
	set(LIBS
					${LIBS}
					${FREEIMAGE_LIBRARIES}
					${CMAKE_SOURCE_DIR}/../../bin/libhydra_api.a
					${CMAKE_SOURCE_DIR}/../../bin/libies_parser.a
					${CMAKE_SOURCE_DIR}/../../bin/libmikktspace.a
					${CMAKE_SOURCE_DIR}/../../bin/libcorto.a)

  find_package(glfw3 REQUIRED)
  include_directories(${GLFW_INCLUDE_DIRS})
  set(LIBS ${LIBS} ${GLFW_LIBRARIES})
endif()

find_package(OpenGL REQUIRED)


include_directories(${ADDITIONAL_INCLUDE_DIRS})


#set(PYBIND11_PYTHON_VERSION 3.6)

add_subdirectory(pybind11)
pybind11_add_module(hydra_api_py HydraPy.cpp) #

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")

target_include_directories (hydra_api_py PUBLIC .. ${OPENGL_INCLUDE_DIR})

#build python bindings shared library
if(WIN32)
	add_definitions(-DUNICODE -D_UNICODE)
	target_link_libraries(hydra_api_py LINK_PUBLIC ${LIBS} ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY})
else()

  set(THREADS_PREFER_PTHREAD_FLAG ON)
	target_link_libraries(hydra_api_py LINK_PUBLIC ${LIBS} ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY} stdc++fs rt dl OpenCL glfw)
endif()