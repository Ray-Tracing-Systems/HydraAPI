cmake_minimum_required(VERSION 3.7)
project(main)

set(CMAKE_CXX_STANDARD 11)

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
        testsA.cpp
        tests_geo.cpp
        tests_gl.cpp
        tests_lights.cpp
        tests_lights2.cpp
        tests_mtl.cpp
        tests_mtl2.cpp
        tests_mtl3.cpp
        Timer.cpp
        Timer.h
        utils.cpp
        #../hydra_api/glad.c
        )

set(ADDITIONAL_INCLUDE_DIRS
        ../LIBRARY/include)

set (CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG}")

if(WIN32)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

	include_directories(${GLFW_INCLUDE_DIRS})
	set(LIBS ${LIBS} ${GLFW_LIBRARIES})
else()
	find_package(glfw3 REQUIRED)
endif()

find_package(OpenGL REQUIRED)

add_executable(main ${SOURCE_FILES})

include_directories(${ADDITIONAL_INCLUDE_DIRS})

target_compile_options(main PRIVATE -fpermissive -Wnarrowing)
target_include_directories(main PRIVATE ${OPENGL_INCLUDE_DIR})
target_link_libraries(main LINK_PUBLIC ${OPENGL_gl_LIBRARY} hydra_api glfw)
