//
// Created by vsan on 23.03.18.
//

#ifndef HYDRAAPI_EX_RENDERDRIVEROPENGL3_UTILITY_H
#define HYDRAAPI_EX_RENDERDRIVEROPENGL3_UTILITY_H

#include "HydraRenderDriverAPI.h"
#include "RenderDriverOpenGL32Forward.h"


using namespace HydraLiteMath;
using namespace GL_RENDER_DRIVER_UTILS;

struct RD_OGL32_Utility : IHRRenderDriver
{
    RD_OGL32_Utility();

    void ClearAll() override;
    HRDriverAllocInfo AllocAll(HRDriverAllocInfo a_info) override;

    bool UpdateImage(int32_t a_texId, int32_t w, int32_t h, int32_t bpp, const void* a_data, pugi::xml_node a_texNode) override;
    bool UpdateMaterial(int32_t a_matId, pugi::xml_node a_materialNode) override;
    bool UpdateLight(int32_t a_lightIdId, pugi::xml_node a_lightNode) override;

    bool UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput &a_input, const HRBatchInfo *a_batchList, int32_t a_listSize) override;

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

    void GetGBufferLine(int32_t a_lineNumber, HRGBufferPixel* a_lineData, int32_t a_startX, int32_t a_endX, const std::unordered_set<int32_t>& a_shadowCatchers) override {}

    HRDriverInfo Info() override;
    const HRRenderDeviceInfoListElem* DeviceList() const override { return nullptr; }
    bool EnableDevice(int32_t id, bool a_enable) override { return true; }

    std::unordered_map<uint32_t, uint32_t> GetMipLevelsDict() { return m_mipLevelDict; };

protected:

    void SetMaterialsTBO();
    void CreateMatricesUBO();
    void CreateMaterialsTBO();
    void FillMipLevelsDict();

    std::wstring m_libPath;

    using meshData = std::unordered_map<int, std::pair<GLuint, int>>;

    std::unordered_map<int32_t, meshData> m_objects; //meshId -> {matId -> vao, indicesNum}
    ShaderProgram m_lodBufferProgram;
    ShaderProgram m_quadProgram;

    std::unique_ptr<RenderTexture2D> m_fullScreenTexture;
    std::unique_ptr<LODBuffer> m_lodBuffer;
    std::unique_ptr<FullScreenQuad>  m_quad;

    std::vector<int4> m_materials_pt1;
    std::vector<int4> m_materials_pt2;

    std::unordered_map<uint32_t, uint32_t> m_mipLevelDict; //texId -> mipLevel

    GLuint m_materialsTBOs[2];
    GLuint m_materialsTBOTexIds[2];

    GLuint m_matricesUBO;
    GLuint m_matricesUBOBindingPoint;

    unsigned int m_texNum;

    //std::vector<float4x4> m_texMatrices;

    GLuint m_whiteTex;

    float4x4 lookAt;
    float4x4 projection;

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

GLFWwindow * InitGLForUtilityDriver();


#endif //HYDRAAPI_EX_RENDERDRIVEROPENGL3_UTILITY_H
