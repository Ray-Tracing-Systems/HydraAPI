cmake_minimum_required(VERSION 3.16)
project(hydraPy LANGUAGES C CXX)

set(Python3_FIND_REGISTRY NEVER)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(HYDRA_API_CMAKE)

option(USE_GL "Build with OpenGL render drivers" OFF)

# -------------------------------------------------------------------------
# Python discovery
# -------------------------------------------------------------------------

set(Python3_FIND_STRATEGY LOCATION)
set(Python3_FIND_REGISTRY LAST)

if(DEFINED PYTHON_EXECUTABLE)
    set(Python3_EXECUTABLE "${PYTHON_EXECUTABLE}")
    message(STATUS "Python explicitly specified: ${Python3_EXECUTABLE}")
else()
    # Пытаемся найти Blender
    find_program(BLENDER_EXE NAMES blender blender.exe)
    if(BLENDER_EXE)
        get_filename_component(BLENDER_DIR "${BLENDER_EXE}" DIRECTORY)
        file(GLOB POSSIBLE_PYTHON_DIRS "${BLENDER_DIR}/*/python")
        if(POSSIBLE_PYTHON_DIRS)
            list(SORT POSSIBLE_PYTHON_DIRS ORDER DESCENDING)
            list(GET POSSIBLE_PYTHON_DIRS 0 BLENDER_PYTHON_ROOT)
            if(EXISTS "${BLENDER_PYTHON_ROOT}/bin/python.exe" OR
               EXISTS "${BLENDER_PYTHON_ROOT}/bin/python")
                set(Python3_ROOT_DIR "${BLENDER_PYTHON_ROOT}")
                message(STATUS "Found Blender → using its Python: ${BLENDER_PYTHON_ROOT}")
            endif()
        endif()
    endif()
endif()

# Ищем Python3 + Development.Module (обязательно для pybind11)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)

if(NOT Python3_Development.Module_FOUND)
    message(FATAL_ERROR
"Cannot find Python 3 development files (headers + python3*.lib / libpython3*.so)

Try one of these:

  cmake .. -DPYTHON_EXECUTABLE=C:/Program Files/Python311/python.exe
  cmake .. -DPYTHON_EXECUTABLE=\"C:/.../Blender 4.2/4.2/python/bin/python.exe\"
  cmake .. -DPYTHON_EXECUTABLE=/usr/bin/python3.11
")
endif()

message(STATUS "Python version .......... : ${Python3_VERSION}")
message(STATUS "Python executable ....... : ${Python3_EXECUTABLE}")
message(STATUS "Python include dirs ..... : ${Python3_INCLUDE_DIRS}")
message(STATUS "Python libraries ........ : ${Python3_LIBRARIES}")

# -------------------------------------------------------------------------
# pybind11
# -------------------------------------------------------------------------
set(PYBIND11_FINDPYTHON ON)
add_subdirectory(pybind11)

# -------------------------------------------------------------------------
# Пути к Hydra
# -------------------------------------------------------------------------
set(HYDRA_ROOT        ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(HYDRA_API_SRC     ${HYDRA_ROOT}/hydra_api)
set(HYDRA_BIN_DIR     ${HYDRA_ROOT}/bin/Release)          # ← под твою сборку

set(FREEIMAGE_INCLUDE ${HYDRA_ROOT}/dependencies/include)
set(FREEIMAGE_LIB     ${HYDRA_ROOT}/dependencies/lib_x64_win/FreeImage.lib)

# Проверка наличия важных библиотек
foreach(_lib
    "${HYDRA_BIN_DIR}/corto.lib"
    "${HYDRA_BIN_DIR}/ies_parser.lib"
    "${HYDRA_BIN_DIR}/mikktspace.lib"
    "${FREEIMAGE_LIB}")
    if(NOT EXISTS "${_lib}")
        message(FATAL_ERROR "Missing required library:\n  ${_lib}")
    endif()
endforeach()

# -------------------------------------------------------------------------
# Исходные файлы
# -------------------------------------------------------------------------
set(SOURCES
    HydraPy.cpp

    ${HYDRA_API_SRC}/HydraAPI.cpp
    ${HYDRA_API_SRC}/HRMeshCommon.cpp
    ${HYDRA_API_SRC}/HRTextureCommon.cpp
    ${HYDRA_API_SRC}/HydraAPI_Camera.cpp
    ${HYDRA_API_SRC}/HydraAPI_FrameBuffer.cpp
    ${HYDRA_API_SRC}/HydraAPI_Geom.cpp
    ${HYDRA_API_SRC}/HydraAPI_GBuffer.cpp
    ${HYDRA_API_SRC}/HydraAPI_Light.cpp
    ${HYDRA_API_SRC}/HydraAPI_LoadExistingLibrary.cpp
    ${HYDRA_API_SRC}/HydraAPI_Material.cpp
    ${HYDRA_API_SRC}/HydraAPI_Texture.cpp
    ${HYDRA_API_SRC}/HydraAPI_TextureProcLex.cpp
    # ${HYDRA_API_SRC}/HydraAPI_EXR.cpp

    ${HYDRA_API_SRC}/../dependencies/include/miniz.c
    ${HYDRA_API_SRC}/xxhash.c

    ${HYDRA_API_SRC}/HydraTextureUtils.cpp
    ${HYDRA_API_SRC}/HydraLegacyUtils.cpp
    ${HYDRA_API_SRC}/HydraDriverUpdate.cpp
    ${HYDRA_API_SRC}/HydraObjectManager.cpp
    ${HYDRA_API_SRC}/HydraRenderDriverAPI.cpp
    ${HYDRA_API_SRC}/HydraRngUtils.cpp
    ${HYDRA_API_SRC}/VirtualBuffer.cpp
    ${HYDRA_API_SRC}/cube2sphere.cpp

    ${HYDRA_API_SRC}/HydraFsUtils.cpp
    ${HYDRA_API_SRC}/RenderDriverHydraConnection.cpp
    ${HYDRA_API_SRC}/RenderDriverDebugPrint.cpp

    ${HYDRA_API_SRC}/HydraXMLHelpers.cpp
    ${HYDRA_API_SRC}/HydraXMLVerify.cpp
    ${HYDRA_API_SRC}/pugixml.cpp

    ${HYDRA_API_SRC}/ssemath.cpp
    ${HYDRA_API_SRC}/HR_HDRImage4f.cpp
    ${HYDRA_API_SRC}/HR_HDRImageTool.cpp

    ${HYDRA_API_SRC}/HydraAPI_GeomProcessing.cpp
    ${HYDRA_API_SRC}/HydraVSGFExport.cpp
    ${HYDRA_API_SRC}/HydraVSGFCompress.cpp
    ${HYDRA_API_SRC}/cmesh.cpp
    ${HYDRA_API_SRC}/cmesh_mikey_connect.cpp
    ${HYDRA_API_SRC}/cmesh_processing_weld.cpp
    ${HYDRA_API_SRC}/cmesh_processing.cpp
)

if(WIN32)
    list(APPEND SOURCES
        ${HYDRA_API_SRC}/SystemWin.cpp
        ${HYDRA_API_SRC}/HR_AccumImageWin.cpp
        ${HYDRA_API_SRC}/HydraLaunchProcessWin32.cpp
    )
endif()

# -------------------------------------------------------------------------
# Модуль
# -------------------------------------------------------------------------
pybind11_add_module(hydraPy ${SOURCES})

target_include_directories(hydraPy PRIVATE
    ${FREEIMAGE_INCLUDE}
    ${HYDRA_API_SRC}
    ${HYDRA_ROOT}/dependencies/include
)

if(MSVC)
    target_compile_options(hydraPy PRIVATE /arch:SSE4.1)
else()
    target_compile_options(hydraPy PRIVATE -msse4.1)
endif()

# -------------------------------------------------------------------------
# Линковка
# -------------------------------------------------------------------------
target_link_libraries(hydraPy PRIVATE
    ${FREEIMAGE_LIB}
    "${HYDRA_BIN_DIR}/corto.lib"
    "${HYDRA_BIN_DIR}/ies_parser.lib"
    "${HYDRA_BIN_DIR}/mikktspace.lib"
    "${HYDRA_BIN_DIR}/clew.lib"
    "${HYDRA_BIN_DIR}/hydra_api.lib"
)

if(WIN32)
    target_link_libraries(hydraPy PRIVATE opengl32 glu32)
endif()

if(MSVC)
    set_target_properties(hydraPy PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
endif()

message(STATUS "───────────────────────────────────────────────")
message(STATUS "hydraPy will be built with Python ${Python3_VERSION}")
message(STATUS "  → ${Python3_EXECUTABLE}")
message(STATUS "───────────────────────────────────────────────")