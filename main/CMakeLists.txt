cmake_minimum_required(VERSION 3.7)
project(main)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        main.cpp
        demo_01.cpp
        demo_02.cpp
        demo_03.cpp
        demo_04.cpp
        demo_05.cpp
        demo_06.cpp
        ../utils/mesh_utils.h
        ../utils/mesh_utils.cpp init_gl.cpp)

set(HYDRA_API_SRC_DIR       ${HYDRA_API_ROOT_DIR}/hydra_api)
set(HYDRA_API_LIB_DIR       ${HYDRA_API_ROOT_DIR}/bin)
set(HYDRA_API_UTILS_SRC_DIR ${HYDRA_API_ROOT_DIR}/utils)


find_package (Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)

include_directories(${ADDITIONAL_INCLUDE_DIRS})
link_directories(${ADDITIONAL_LIBRARY_DIRS})
    
if(USE_GL)
  cmake_policy(SET CMP0072 OLD)
  find_package(OpenGL REQUIRED)
  include_directories(${OPENGL_INCLUDE_DIR})
else()
  message(WARNING " HydraAPI demos require OpenGL. Set USE_GL to ON if you want to build them")
endif()


if(VECTOR_TEX_SUPPORT)
	include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/msdfgen/include)
	include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/nanosvg)
	include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/skia/headers)
	include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/skia/headers/include)
	include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/skia/headers/modules)
	include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/skia/headers/src)
	include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/skia/headers/third_party)
	
	if(WIN32)
		set(MSDF_LIB_DIR_DEBUG   ${HYDRA_API_ROOT_DIR}/dependencies/optional/msdfgen/lib/win_x64_debug)
		set(MSDF_LIB_DIR_RELEASE ${HYDRA_API_ROOT_DIR}/dependencies/optional/msdfgen/lib/win_x64_release)
		set(SKIA_LIB_DIR         ${HYDRA_API_ROOT_DIR}/dependencies/optional/skia/win_x64)

        find_library(MSDF_LIB_DEBUG   NAMES msdfgen REQUIRED HINTS ${MSDF_LIB_DIR_DEBUG})
        find_library(MSDF_LIB_RELEASE NAMES msdfgen REQUIRED HINTS ${MSDF_LIB_DIR_RELEASE})
	else()
		set(MSDF_LIB_DIR_DEBUG   ${HYDRA_API_ROOT_DIR}/dependencies/optional/msdfgen/lib/linux_x64_debug)
		set(MSDF_LIB_DIR_RELEASE ${HYDRA_API_ROOT_DIR}/dependencies/optional/msdfgen/lib/linux_x64_release)
		set(SKIA_LIB_DIR         ${HYDRA_API_ROOT_DIR}/dependencies/optional/skia/linux_x64)

        find_library(MSDF_LIB_CORE_DEBUG   NAMES msdfgen-core REQUIRED HINTS ${MSDF_LIB_DIR_DEBUG})
        find_library(MSDF_LIB_EXT_DEBUG    NAMES msdfgen-ext  REQUIRED HINTS ${MSDF_LIB_DIR_DEBUG})
        find_library(MSDF_LIB_CORE_RELEASE NAMES msdfgen-core REQUIRED HINTS ${MSDF_LIB_DIR_RELEASE})
        find_library(MSDF_LIB_EXT_RELEASE  NAMES msdfgen-ext  REQUIRED HINTS ${MSDF_LIB_DIR_RELEASE})
        set(MSDF_LIB_DEBUG   ${MSDF_LIB_CORE_DEBUG}   ${MSDF_LIB_EXT_DEBUG})
        set(MSDF_LIB_RELEASE ${MSDF_LIB_CORE_RELEASE} ${MSDF_LIB_EXT_RELEASE})
	endif()

	find_library(SKIA_LIB NAMES skia.dll skia REQUIRED HINTS ${SKIA_LIB_DIR})
endif()

add_executable(main ${SOURCE_FILES})
include_directories(${HYDRA_API_SRC_DIR} ${HYDRA_API_UTILS_SRC_DIR})


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1 -mavx")

add_dependencies(main hydra_api corto ies_parser mikktspace)
target_link_libraries(main LINK_PUBLIC hydra_api corto ies_parser mikktspace freeimage)

if(VECTOR_TEX_SUPPORT)
	target_link_libraries(main LINK_PUBLIC debug     ${MSDF_LIB_DEBUG})  
	target_link_libraries(main LINK_PUBLIC optimized ${MSDF_LIB_RELEASE})  
	target_link_libraries(main LINK_PUBLIC ${SKIA_LIB})  
endif()

if(WIN32)
  add_dependencies(main clew)
  add_definitions(-DUNICODE -D_UNICODE)
  add_definitions(-DNEED_DIR_CHANGE)
  if(USE_GL)
    target_link_libraries(main LINK_PUBLIC ${OPENGL_gl_LIBRARY} glfw3dll)
  endif()
  
  target_link_libraries(main LINK_PUBLIC clew)

else()
  find_package(OpenCL REQUIRED)
  target_compile_options(main PRIVATE -fpermissive -Wnarrowing)
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory -fno-omit-frame-pointer -fsanitize=leak)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fsanitize=leak -fsanitize=undefined")
    MESSAGE( STATUS "WE USE CLANG(main);")
  else()
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fsanitize=leak -fsanitize=undefined -fsanitize=bounds-strict")
    MESSAGE( STATUS "WE USE GCC  (main);")
  endif()

  if(USE_GL)
    target_link_libraries(main LINK_PUBLIC ${OPENGL_gl_LIBRARY} ${GLFW_LIBRARIES} glfw)
  endif()

  target_link_libraries(main LINK_PUBLIC Threads::Threads rt dl OpenCL::OpenCL )

endif()

MESSAGE( STATUS "CMAKE_CXX_FLAGS(main): " ${CMAKE_CXX_FLAGS} )

