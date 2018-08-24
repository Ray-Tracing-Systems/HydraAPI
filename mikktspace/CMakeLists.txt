set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
    mikktspace.h
    mikktspace.c
    )


add_library(mikktspace STATIC ${SOURCE_FILES})

set_property(TARGET mikktspace PROPERTY POSITION_INDEPENDENT_CODE ON)

