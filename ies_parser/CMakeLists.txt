set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
    IESNA.H
    IESNA.C
    IES_READ.C
    )


add_library(ies_parser STATIC ${SOURCE_FILES})

set_property(TARGET ies_parser PROPERTY POSITION_INDEPENDENT_CODE ON)

if(WIN32)
  add_definitions(-DUNICODE -D_UNICODE)
endif()

