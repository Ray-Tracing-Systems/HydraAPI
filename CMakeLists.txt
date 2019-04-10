cmake_minimum_required(VERSION 3.7)
project(HydraAPI_Ex)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(ADDITIONAL_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/LIBRARY/include)
        
set(ADDITIONAL_LIBRARY_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/LIBRARY/lib_x64)   

add_subdirectory (ies_parser)
add_subdirectory (mikktspace)
add_subdirectory (corto)
add_subdirectory (hydra_api)
#add_subdirectory (hydra_api/hydra_api_py)


if(WIN32)
  add_definitions(-DUNICODE -D_UNICODE)
#  if(COMPILER_MSVC)
#    set_target_properties(main PROPERTIES LINK_FLAGS "/LTCG")
#    set_target_properties(hydra_api PROPERTIES LINK_FLAGS "/LTCG")
#    set_target_properties(ies_parser PROPERTIES LINK_FLAGS "/LTCG")
#  endif(COMPILER_MSVC)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)
endif(WIN32)
