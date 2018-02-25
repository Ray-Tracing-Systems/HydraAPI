//
// Created by hikawa on 04.08.17.
//

#pragma once


#include "HydraRenderDriverAPI.h"
#include "OpenGLCoreProfileUtils.h"


using namespace HydraLiteMath;
using namespace GL_RENDER_DRIVER_UTILS;

struct RD_OGL32_Forward : public IHRRenderDriver
{
    RD_OGL32_Forward();

    void ClearAll() override;
    HRDriverAllocInfo AllocAll(HRDriverAllocInfo a_info) override;

    bool UpdateImage(int32_t a_texId, int32_t w, int32_t h, int32_t bpp, const void* a_data, pugi::xml_node a_texNode) override;
    bool UpdateMaterial(int32_t a_matId, pugi::xml_node a_materialNode) override;
    bool UpdateLight(int32_t a_lightIdId, pugi::xml_node a_lightNode) override;
    bool UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input, const HRBatchInfo* a_batchList, int32_t a_listSize) override;

    bool UpdateImageFromFile(int32_t a_texId, const wchar_t* a_fileName, pugi::xml_node a_texNode) override {return false;}
    bool UpdateMeshFromFile(int32_t a_meshId, pugi::xml_node a_meshNode, const wchar_t* a_fileName) override {return false;}


    bool UpdateCamera(pugi::xml_node a_camNode) override;
    bool UpdateSettings(pugi::xml_node a_settingsNode) override;

    /////////////////////////////////////////////////////////////////////////////////////////////

    void BeginScene(pugi::xml_node a_sceneNode) override;
    void EndScene() override;
    void InstanceMeshes(int32_t a_mesh_id, const float* a_matrices, int32_t a_instNum, const int* a_lightInstId, const int* a_remapId) override;
    void InstanceLights(int32_t a_light_id, const float* a_matrix, pugi::xml_node* a_custAttrArray, int32_t a_instNum, int32_t a_lightGroupId) override;

    void Draw() override;

    HRRenderUpdateInfo HaveUpdateNow(int a_maxRaysPerPixel) override;

    void GetFrameBufferHDR(int32_t w, int32_t h, float*   a_out, const wchar_t* a_layerName) override;
    void GetFrameBufferLDR(int32_t w, int32_t h, int32_t* a_out) override;

    void GetGBufferLine(int32_t a_lineNumber, HRGBufferPixel* a_lineData, int32_t a_startX, int32_t a_endX) override {}

    HRDriverInfo Info() override;
    const HRRenderDeviceInfoListElem* DeviceList() const override { return nullptr; }
    bool EnableDevice(int32_t id, bool a_enable) override { return true; }

protected:

    std::wstring m_libPath;
    std::wstring m_msg;

    using meshData = std::unordered_map<int, std::pair<GLuint, int>>;

    std::unordered_map<int32_t, meshData> m_objects; //meshId -> {matId -> vao, indicesNum}
    std::unordered_map<std::string, ShaderProgram> m_programs;
    ShaderProgram m_quadProgram;
    ShaderProgram m_matProgram;

    std::unique_ptr<RenderTexture2D> m_fullScreenTexture;
    std::unique_ptr<FullScreenQuad>  m_quad;

    std::vector<GLuint> m_texturesList;

    std::vector<float3> m_diffColors;
    std::vector<float3> m_reflColors;
    std::vector<float>  m_reflGloss;

    std::vector<int> m_diffTexId;
    std::vector<int> m_reflTexId;

    GLuint m_whiteTex;

    // camera parameters
    //
    float camPos[3];
    float camLookAt[3];
    float camUp[3];

    float camFov;
    float camNearPlane;
    float camFarPlane;
    int   m_width;
    int   m_height;

};
