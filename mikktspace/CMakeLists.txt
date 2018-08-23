set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
    mikktspace.h
    mikktspace.c
    )


add_library(mikttspace STATIC ${SOURCE_FILES})

set_property(TARGET mikttspace PROPERTY POSITION_INDEPENDENT_CODE ON)

