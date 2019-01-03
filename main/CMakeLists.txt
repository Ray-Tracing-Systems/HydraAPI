cmake_minimum_required(VERSION 3.7)
project(main)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
        Camera.h
        input.cpp
        input.h
        linmath.h
        main.cpp
        mesh_utils.cpp
        mesh_utils.h
        test_camera_free_look_gl.cpp
        test_mtl_adv.cpp
        test_image_processing.cpp
        tests.h
        tests1.cpp
        tests2.cpp
        tests3.cpp
        tests4.cpp
        tests5.cpp
        tests6.cpp
        tests7.cpp
        tests8.cpp
        tests9.cpp
        tests10.cpp
        testsA.cpp
        testsB.cpp
        tests_geo.cpp
        tests_gl.cpp
        tests_lights.cpp
        tests_lights2.cpp
        test_lights3.cpp
        test_lights4.cpp
        tests_mtl.cpp
        tests_mtl2.cpp
        tests_mtl3.cpp
        tests_mtl4.cpp
        tests_mtl5.cpp
        tests_pp.cpp
        Timer.cpp
        Timer.h
        utils.cpp
        simplerandom.h
        tests_external.cpp demo_01.cpp)

#set (CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG}")

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

add_executable(main ${SOURCE_FILES})
include_directories(${ADDITIONAL_INCLUDE_DIRS})

target_include_directories(main PUBLIC ${OPENGL_INCLUDE_DIR})

if(WIN32)
  add_definitions(-DUNICODE -D_UNICODE)
  add_definitions(-DNEED_DIR_CHANGE)
  target_link_libraries(main LINK_PUBLIC ${OPENGL_gl_LIBRARY} hydra_api glfw3dll )
else()
  target_compile_options(main PRIVATE -fpermissive -Wnarrowing)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1 -fopenmp")

  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      MESSAGE( STATUS "WE USE CLANG(main);")
  else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=bounds -fsanitize=address -fsanitize=leak -fsanitize=undefined")
      MESSAGE( STATUS "WE USE GCC  (main);")
  endif()

  MESSAGE( STATUS "CMAKE_CXX_FLAGS(main): " ${CMAKE_CXX_FLAGS} )
  target_link_libraries(main LINK_PUBLIC Threads::Threads  ${OPENGL_gl_LIBRARY} ${LIBS} hydra_api glfw)

endif()



