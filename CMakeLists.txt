cmake_minimum_required(VERSION 3.7)
project(HydraAPI_Ex)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(ADDITIONAL_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include)

SET(USE_GL ON CACHE BOOL "Build OpenGL render drivers")

if(WIN32)
  set(ADDITIONAL_LIBRARY_DIRS
          ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib_x64_win)
else()
  SET(USE_FIND_PACKAGE OFF CACHE BOOL "Use find_package to find system installation of dependencies")
  set(ADDITIONAL_LIBRARY_DIRS
          ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib_x64_linux)
endif()

add_subdirectory (utils/ies_parser)
add_subdirectory (utils/mikktspace)
add_subdirectory (utils/corto)
add_subdirectory (hydra_api)
add_subdirectory (main)
#add_subdirectory (hydra_api_python)


if(WIN32)
  add_subdirectory (utils/clew)
  add_definitions(-DUNICODE -D_UNICODE)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)
endif(WIN32)
