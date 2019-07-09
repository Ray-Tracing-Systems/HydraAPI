#
# Find GLFW3
#
# Try to find GLFW3.
# This module defines the following variables:
# - GLFW_INCLUDE_DIRS
# - GLFW_LIBRARIES
# - GLFW_FOUND
#
# The following variables can be set as arguments for the module.
# - GLFW_ROOT_DIR : Root library directory of FreeImage
#

# Additional modules

include(FindPackageHandleStandardArgs)


if (WIN32)
	# Find include files
	find_path(
		GLFW_INCLUDE_DIR 
    NAMES
        GLFW/glfw3.h
    HINTS
        ${GLFW_LOCATION}/include
        $ENV{GLFW_LOCATION}/include
    PATHS
        $ENV{PROGRAMFILES}/GLFW/include
        ${OPENGL_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/LIBRARY/include
		DOC "The directory where GLFW/glfw3.h resides")

	# Find library files
	find_library(
		GLFW_LIBRARY
		NAMES 
      glfw3dll
		PATHS
			$ENV{PROGRAMFILES}/lib
			${GLFW_ROOT_DIR}/lib
			${CMAKE_SOURCE_DIR}/LIBRARY/lib_x64
      DOC "The glfw3dll static library")
else()
	# Find include files
	find_path(
		GLFW_INCLUDE_DIR
		NAMES  
      GLFW/glfw3.h
		PATHS
			/usr/include
			/usr/local/include
			/sw/include
			/opt/local/include
		DOC "The directory where GLFW/glfw3.h resides")

	# Find library files
	find_library(
		GLFW_LIBRARY
		NAMES glfw3
		PATHS
			/usr/lib64
			/usr/lib
			/usr/local/lib64
			/usr/local/lib
			/sw/lib
			/opt/local/lib
			${GLFW_ROOT_DIR}/lib
		DOC "The glfw3 static library")
endif()

# Handle REQUIRED argument, define *_FOUND variable
find_package_handle_standard_args(glfw3dll DEFAULT_MSG GLFW_INCLUDE_DIR GLFW_LIBRARY)

# Define FREEIMAGE_LIBRARIES and FREEIMAGE_INCLUDE_DIRS
if (GLFW3_FOUND)
	set(GLFW_LIBRARIES ${GLFW_LIBRARY})
	set(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIR})
endif()

# Hide some variables
mark_as_advanced(GLFW_INCLUDE_DIR GLFW_LIBRARY)