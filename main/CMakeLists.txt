cmake_minimum_required(VERSION 3.7)
project(main)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
        main.cpp
        demo_01.cpp
        demo_02.cpp
        demo_03.cpp
        demo_04.cpp
        demo_05.cpp
        ../utils/mesh_utils.h
        ../utils/mesh_utils.cpp init_gl.cpp)

set(HYDRA_API_ROOT_DIR      ${CMAKE_SOURCE_DIR}/../HydraAPI CACHE PATH "Path to HydraAPI") #
set(HYDRA_API_SRC_DIR       ${HYDRA_API_ROOT_DIR}/hydra_api)
set(HYDRA_API_LIB_DIR       ${HYDRA_API_ROOT_DIR}/bin)
set(HYDRA_API_UTILS_SRC_DIR ${HYDRA_API_ROOT_DIR}/utils)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

if(WIN32)
  link_directories(${ADDITIONAL_LIBRARY_DIRS})
else()
  find_package(glfw3 REQUIRED)
  include_directories(${GLFW_INCLUDE_DIRS})
  set(LIBS ${LIBS} ${GLFW_LIBRARIES})

  find_package (Threads REQUIRED)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()


find_package(OpenGL REQUIRED)
add_executable(main ${SOURCE_FILES} ${SOURCE_FILES_GL})
include_directories(${ADDITIONAL_INCLUDE_DIRS} ${HYDRA_API_SRC_DIR} ${HYDRA_API_UTILS_SRC_DIR})

find_package(FreeImage REQUIRED)
include_directories(${FREEIMAGE_INCLUDE_DIRS})
set(LIBS ${LIBS} ${FREEIMAGE_LIBRARIES})

add_dependencies(main hydra_api corto ies_parser mikktspace)

target_include_directories(main PUBLIC ${OPENGL_INCLUDE_DIR})

if(WIN32)
  add_definitions(-DUNICODE -D_UNICODE)
  add_definitions(-DNEED_DIR_CHANGE)
  target_link_libraries(main LINK_PUBLIC ${OPENGL_gl_LIBRARY} hydra_api glfw3dll )
else()
  target_compile_options(main PRIVATE -fpermissive -Wnarrowing)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")

  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory -fno-omit-frame-pointer -fsanitize=leak)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fsanitize=leak -fsanitize=undefined")
    MESSAGE( STATUS "WE USE CLANG(main);")
  else()
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fsanitize=leak -fsanitize=undefined -fsanitize=bounds-strict")
    MESSAGE( STATUS "WE USE GCC  (main);")
  endif()

  MESSAGE( STATUS "CMAKE_CXX_FLAGS(main): " ${CMAKE_CXX_FLAGS} )
  target_link_libraries(main LINK_PUBLIC
          ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY}
          hydra_api corto ies_parser mikktspace
          ${LIBS} Threads::Threads rt dl OpenCL glfw)

endif()

