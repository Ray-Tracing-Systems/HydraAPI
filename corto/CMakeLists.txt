set(CMAKE_CXX_STANDARD 14)

SET(LIB_HEADERS
	  bitstream.h
	  color_attribute.h
	  corto.h
	  cstream.h
	  decoder.h
	  encoder.h
	  index_attribute.h
	  normal_attribute.h
	  point.h
	  tunstall.h
	  vertex_attribute.h
	  zpoint.h)

SET(LIB_SOURCES
	  bitstream.cpp
	  color_attribute.cpp
	  cstream.cpp
	  decoder.cpp
	  encoder.cpp
	  normal_attribute.cpp
	  tunstall.cpp)

ADD_LIBRARY(corto STATIC ${LIB_SOURCES} ${LIB_HEADERS})

set_property(TARGET corto PROPERTY POSITION_INDEPENDENT_CODE ON)

if(MSVC)
	SET(CORTO_CFLAGS /nologo /W3)
	LIST(APPEND CORTO_DEFINITIONS _CRT_SECURE_NO_DEPRECATE)
    add_definitions(-DUNICODE -D_UNICODE)
else()
	SET(CORTO_CFLAGS -O2 -W -Wall -c -std=c++11 -pedantic)
endif()

#SET(CORTOLIB_CFLAGS ${CORTO_CFLAGS})

#if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
#	LIST(APPEND CORTOLIB_CFLAGS -fPIC)
#elseif(APPLE)
#	LIST(APPEND CORTOLIB_CFLAGS -fvisibility=hidden)
#	if(IOS)
#		LIST(APPEND CORTOLIB_CFLAGS -fembed-bitcode)
#		set_xcode_property(corto IPHONEOS_DEPLOYMENT_TARGET "9.2")
#	endif(IOS)
#endif()

#target_include_directories(cortolib PUBLIC ${CORTO_SOURCE_PATH})
#target_compile_definitions(cortolib PUBLIC ${CORTO_DEFINITIONS})
#target_compile_options    (cortolib PUBLIC ${CORTOLIB_CFLAGS})
#set_target_properties     (cortolib PROPERTIES DEBUG_POSTFIX "d")
#set_target_properties     (cortolib PROPERTIES OUTPUT_NAME "corto")
