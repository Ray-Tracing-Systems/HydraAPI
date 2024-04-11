cmake_minimum_required(VERSION 3.7)
project(HydraAPI_Ex)

set(CMAKE_CXX_STANDARD 17)

set(HYDRA_API_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin CACHE PATH "Output directory for all artifacts")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${HYDRA_API_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${HYDRA_API_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${HYDRA_API_OUTPUT_DIRECTORY})

set(HYDRA_API_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(ADDITIONAL_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include)

set(USE_GL OFF CACHE BOOL "Build OpenGL render drivers")
set(VECTOR_TEX_SUPPORT ON CACHE BOOL "Build with vector textures support extension")

set(TINYEXR_IMPL ON CACHE BOOL "TinyEXR implementation in HydraAPI")
if(TINYEXR_IMPL)
  add_compile_definitions(TINYEXR_IMPL)
endif()

if(WIN32)
  add_compile_definitions(UNICODE _UNICODE)
  set(ADDITIONAL_LIBRARY_DIRS
          ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib_x64_win CACHE STRING "Path to dependencies (freeimage and, if USE_GL is ON, glfw)")
else()
  set(ADDITIONAL_LIBRARY_DIRS
          ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib_x64_linux CACHE STRING "Path to dependencies (freeimage and, if USE_GL is ON, glfw)")
endif()

add_compile_definitions(HYDRA_API_CMAKE)
if(USE_GL)
  add_compile_definitions(USE_GL)
endif()

if(VECTOR_TEX_SUPPORT)
  add_compile_definitions(VECTOR_TEX_SUPPORT)
endif()

add_subdirectory (utils/ies_parser)
add_subdirectory (utils/mikktspace)
add_subdirectory (utils/corto)
add_subdirectory (hydra_api)
#add_subdirectory (hydra_api_python)


if(USE_GL)
    add_subdirectory (main)
endif()

if(WIN32)
  add_subdirectory (utils/clew)
  if(USE_GL)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)
  else()
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT hydra_api)
  endif()
endif(WIN32)
