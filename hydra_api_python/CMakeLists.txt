# CMakeLists.txt (hydra_api_python)

cmake_minimum_required(VERSION 3.16)
project(hydraPy LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(HYDRA_API_CMAKE)

option(USE_GL "Build OpenGL render drivers" OFF)

set(HYDRA_API_ROOT_DIR ${CMAKE_SOURCE_DIR}/..)
set(HYDRA_API_SRC_DIR ${HYDRA_API_ROOT_DIR}/hydra_api)
set(HYDRA_API_LIB_DIR ${HYDRA_API_ROOT_DIR}/bin)

# ---------- FreeImage (vendored) ----------

set(FREEIMAGE_INCLUDE_DIR
    ${HYDRA_API_ROOT_DIR}/dependencies/include)

set(FREEIMAGE_LIBRARY
    ${HYDRA_API_ROOT_DIR}/dependencies/lib_x64_linux/libfreeimage.so)

# ---------- Python (modern way) ----------

set(PYBIND11_FINDPYTHON ON)
find_package(Python REQUIRED COMPONENTS Interpreter Development)

add_subdirectory(pybind11)

# ---------- Source files ----------

set(HAPI_SOURCE_FILES
    HydraPy.cpp

    ${HYDRA_API_SRC_DIR}/HydraAPI.cpp
    ${HYDRA_API_SRC_DIR}/HRMeshCommon.cpp
    ${HYDRA_API_SRC_DIR}/HRTextureCommon.cpp
    ${HYDRA_API_SRC_DIR}/HydraAPI_Camera.cpp
    ${HYDRA_API_SRC_DIR}/HydraAPI_FrameBuffer.cpp
    ${HYDRA_API_SRC_DIR}/HydraAPI_Geom.cpp
    ${HYDRA_API_SRC_DIR}/HydraAPI_GBuffer.cpp
    ${HYDRA_API_SRC_DIR}/HydraAPI_Light.cpp
    ${HYDRA_API_SRC_DIR}/HydraAPI_LoadExistingLibrary.cpp
    ${HYDRA_API_SRC_DIR}/HydraAPI_Material.cpp
    ${HYDRA_API_SRC_DIR}/HydraAPI_Texture.cpp
    ${HYDRA_API_SRC_DIR}/HydraAPI_TextureProcLex.cpp
    ${HYDRA_API_SRC_DIR}/HydraAPI_EXR.cpp

    ${HYDRA_API_SRC_DIR}/../dependencies/include/miniz.c
    ${HYDRA_API_SRC_DIR}/xxhash.c

    ${HYDRA_API_SRC_DIR}/HydraTextureUtils.cpp
    ${HYDRA_API_SRC_DIR}/HydraLegacyUtils.cpp
    ${HYDRA_API_SRC_DIR}/HydraDriverUpdate.cpp
    ${HYDRA_API_SRC_DIR}/HydraObjectManager.cpp
    ${HYDRA_API_SRC_DIR}/HydraRenderDriverAPI.cpp
    ${HYDRA_API_SRC_DIR}/HydraRngUtils.cpp
    ${HYDRA_API_SRC_DIR}/VirtualBuffer.cpp
    ${HYDRA_API_SRC_DIR}/cube2sphere.cpp

    ${HYDRA_API_SRC_DIR}/HydraFsUtils.cpp
    ${HYDRA_API_SRC_DIR}/RenderDriverHydraConnection.cpp
    ${HYDRA_API_SRC_DIR}/RenderDriverDebugPrint.cpp

    ${HYDRA_API_SRC_DIR}/HydraXMLHelpers.cpp
    ${HYDRA_API_SRC_DIR}/HydraXMLVerify.cpp
    ${HYDRA_API_SRC_DIR}/pugixml.cpp

    ${HYDRA_API_SRC_DIR}/ssemath.cpp
    ${HYDRA_API_SRC_DIR}/HR_HDRImage4f.cpp
    ${HYDRA_API_SRC_DIR}/HR_HDRImageTool.cpp

    ${HYDRA_API_SRC_DIR}/HydraAPI_GeomProcessing.cpp
    ${HYDRA_API_SRC_DIR}/HydraVSGFExport.cpp
    ${HYDRA_API_SRC_DIR}/HydraVSGFCompress.cpp
    ${HYDRA_API_SRC_DIR}/cmesh.cpp
    ${HYDRA_API_SRC_DIR}/cmesh_mikey_connect.cpp
    ${HYDRA_API_SRC_DIR}/cmesh_processing_weld.cpp
    ${HYDRA_API_SRC_DIR}/cmesh_processing.cpp
)

if(UNIX)
    list(APPEND HAPI_SOURCE_FILES
        ${HYDRA_API_SRC_DIR}/SystemUnix.cpp
        ${HYDRA_API_SRC_DIR}/HR_AccumImageLinux.cpp
        ${HYDRA_API_SRC_DIR}/HydraLaunchProcessLinux.cpp
    )
endif()

# ---------- Module ----------

pybind11_add_module(hydraPy ${HAPI_SOURCE_FILES})

target_link_options(hydraPy PRIVATE
    -Wl,--disable-new-dtags)

target_include_directories(hydraPy PRIVATE
    ${FREEIMAGE_INCLUDE_DIR}
    ${HYDRA_API_SRC_DIR}
)

target_compile_options(hydraPy PRIVATE -msse4.1)

# ---------- Link ----------

find_library(HYDRA_API_IES_PARSER_LIB ies_parser HINTS ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
find_library(HYDRA_API_MIKKTSPACE mikktspace HINTS ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
find_library(HYDRA_API_CORTO corto HINTS ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)

target_link_libraries(hydraPy PRIVATE
    ${FREEIMAGE_LIBRARY}
    ${HYDRA_API_IES_PARSER_LIB}
    ${HYDRA_API_MIKKTSPACE}
    ${HYDRA_API_CORTO}
    OpenCL
    glfw
    dl
    rt
)

# ---------- RPATH (correct way) ----------

set_target_properties(hydraPy PROPERTIES
    BUILD_RPATH "$ORIGIN/../dependencies/lib_x64_linux"
    INSTALL_RPATH "$ORIGIN/../dependencies/lib_x64_linux"
)
