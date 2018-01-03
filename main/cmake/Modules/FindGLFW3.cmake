include(FindPackageHandleStandardArgs)


find_path( GLFW_INCLUDE_DIR 
    NAMES
        GLFW/glfw3.h
    HINTS
        "${GLFW_LOCATION}/include"
        "$ENV{GLFW_LOCATION}/include"
    PATHS
        "$ENV{PROGRAMFILES}/GLFW/include"
        "${OPENGL_INCLUDE_DIR}"
        ${CMAKE_SOURCE_DIR}/LIBRARY/include
    DOC 
        "The directory where GLFW/glfw3.h resides"
)


if (WIN32)
	find_library(
		GLFW_LIBRARY
		NAMES FreeImage
		PATHS
			$ENV{PROGRAMFILES}/lib
			${FREEIMAGE_ROOT_DIR}/lib
			${CMAKE_SOURCE_DIR}/LIBRARY/lib_x64)
    
endif (WIN32)


# Handle REQUIRD argument, define *_FOUND variable
find_package_handle_standard_args(glfw3 DEFAULT_MSG GLFW_INCLUDE_DIR GLFW_LIBRARY)

# Define FREEIMAGE_LIBRARIES and FREEIMAGE_INCLUDE_DIRS
if (GLFW3_FOUND)
	set(GLFW_LIBRARIES ${GLFW_LIBRARY})
	set(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIR})
endif()

# Hide some variables
mark_as_advanced(GLFW_INCLUDE_DIR GLFW_LIBRARY)