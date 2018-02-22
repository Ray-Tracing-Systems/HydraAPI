#include "HydraRenderDriverAPI.h"
#include "RenderDriverOpenGL1.h"


#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "LiteMath.h"
using namespace HydraLiteMath;

// #if defined(WIN32)
//   #include <GLFW/glfw3.h>
//   #pragma comment(lib, "glfw3dll.lib")
// #else
//   #include <GLFW/glfw3.h>
// #endif

#include <vector>
#include <string>
#include <sstream>
#include <fstream>

struct RD_OGL1_TestSplit : public RD_OGL1_Plain
{
  RD_OGL1_TestSplit()
  {
    m_renderNormalLength = 0.5f;
    m_drawNormals = true;
    m_drawTangents = false;
    m_drawSolid = true;
    m_drawWire = false;
    m_drawAxis = false;

    m_axisArrorLen = 1.0f;
    m_axisArrorThickness = 0.1f;
    m_meshNum = 0;
  }

  void              ClearAll();
  HRDriverAllocInfo AllocAll(HRDriverAllocInfo a_info);

  bool     UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input, const HRBatchInfo* a_batchList, int32_t listSize);
  bool     UpdateSettings(pugi::xml_node a_settingsNode);

  /////////////////////////////////////////////////////////////////////////////////////////////

  void     InstanceMeshes(int32_t a_mesh_id, const float* a_matrices, int32_t a_instNum, const int* a_lightInstId, const int* a_remapId);

  void     EndScene();

protected:

  unsigned int m_meshNum;
  GLfloat m_renderNormalLength;

  bool m_drawSolid;
  bool m_drawWire;
  bool m_drawAxis;
  bool m_drawNormals;
  bool m_drawTangents;

  float m_axisArrorLen;
  float m_axisArrorThickness;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void _glutWireCube(GLdouble dSize)
{
  float s = float(dSize * 0.5f);

  glBegin(GL_LINE_LOOP); glVertex3d(+s, -s, +s); glVertex3d(+s, -s, -s); glVertex3d(+s, +s, -s); glVertex3d(+s, +s, +s); glEnd();
  glBegin(GL_LINE_LOOP); glVertex3d(+s, +s, +s); glVertex3d(+s, +s, -s); glVertex3d(-s, +s, -s); glVertex3d(-s, +s, +s); glEnd();
  glBegin(GL_LINE_LOOP); glVertex3d(+s, +s, +s); glVertex3d(-s, +s, +s); glVertex3d(-s, -s, +s); glVertex3d(+s, -s, +s); glEnd();
  glBegin(GL_LINE_LOOP); glVertex3d(-s, -s, +s); glVertex3d(-s, +s, +s); glVertex3d(-s, +s, -s); glVertex3d(-s, -s, -s); glEnd();
  glBegin(GL_LINE_LOOP); glVertex3d(-s, -s, +s); glVertex3d(-s, -s, -s); glVertex3d(+s, -s, -s); glVertex3d(+s, -s, +s); glEnd();
  glBegin(GL_LINE_LOOP); glVertex3d(-s, -s, -s); glVertex3d(-s, +s, -s); glVertex3d(+s, +s, -s); glVertex3d(+s, -s, -s); glEnd();
}

static void DrawBox(const float3 a_boxMin, const float3 a_boxMax)
{
  const float3 scale = a_boxMax - a_boxMin;
  const float3 cent  = a_boxMin + 0.5f*scale;

  glPushMatrix();
  glTranslatef(cent.x, cent.y, cent.z);
  glScalef(scale.x, scale.y, scale.z);
  _glutWireCube(1);
  glPopMatrix();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void RD_OGL1_TestSplit::ClearAll()
{
  RD_OGL1_Plain::ClearAll();
  m_meshNum = 0;
}

HRDriverAllocInfo RD_OGL1_TestSplit::AllocAll(HRDriverAllocInfo a_info)
{
  m_meshNum = a_info.geomNum;
  m_listNum = GLsizei(a_info.geomNum * 4);
  m_displayLists = glGenLists(GLsizei(m_listNum));

  m_texturesList.resize(a_info.imgNum);
  glGenTextures(GLsizei(a_info.imgNum), &m_texturesList[0]);

  m_diffColors.resize(a_info.matNum * 4);
  m_diffTexId.resize(a_info.matNum);

  for (size_t i = 0; i < m_diffTexId.size(); i++)
    m_diffTexId[i] = -1;

  return a_info;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// typedef void (APIENTRYP PFNGLGENERATEMIPMAPPROC)(GLenum target);
// PFNGLGENERATEMIPMAPPROC glad_glGenerateMipmap;
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool RD_OGL1_TestSplit::UpdateSettings(pugi::xml_node a_settingsNode)
{
  if (a_settingsNode.child(L"width") != nullptr)
    m_width = a_settingsNode.child(L"width").text().as_int();

  if (a_settingsNode.child(L"height") != nullptr)
    m_height = a_settingsNode.child(L"height").text().as_int();

  if (a_settingsNode.child(L"draw_length") != nullptr)
    m_renderNormalLength = a_settingsNode.child(L"draw_length").text().as_float();

  if (a_settingsNode.child(L"draw_normals") != nullptr)
    m_drawNormals = a_settingsNode.child(L"draw_normals").text().as_bool();

  if (a_settingsNode.child(L"draw_tangents") != nullptr)
    m_drawTangents = a_settingsNode.child(L"draw_tangents").text().as_bool();

  if (a_settingsNode.child(L"draw_solid") != nullptr)
    m_drawSolid = a_settingsNode.child(L"draw_solid").text().as_bool();

  if (a_settingsNode.child(L"draw_wire") != nullptr)
    m_drawWire = a_settingsNode.child(L"draw_wire").text().as_bool();

  if (a_settingsNode.child(L"draw_axis") != nullptr)
    m_drawAxis = a_settingsNode.child(L"draw_axis").text().as_bool();

  if (a_settingsNode.child(L"axis_arrow_length") != nullptr)
    m_axisArrorLen = a_settingsNode.child(L"axis_arrow_length").text().as_float();

  if (a_settingsNode.child(L"axis_arrow_thickness") != nullptr)
    m_axisArrorThickness = a_settingsNode.child(L"axis_arrow_thickness").text().as_float();

  if (m_width < 0 || m_height < 0)
  {
    m_msg = L"RD_OGL1_TestSplit::UpdateSettings, bad input resolution";
    return false;
  }

  return true;
}

#include <algorithm>
#include <queue>

namespace EarlySplit
{
  struct Box3f
  {
    Box3f() : vmin(1e38f, 1e38f, 1e38f), vmax(-1e38f, -1e38f, -1e38f){}
    float3 vmin;
    float3 vmax;

    inline float3 center() const { return 0.5f*(vmin+ vmax); }

    inline bool axisAligned(int axis, float split) const 
    { 
      float amin[3] = { vmin.x, vmin.y, vmin.z };
      float amax[3] = { vmax.x, vmax.y, vmax.z };

      return (amin[axis] == amax[axis]) && (amin[axis] == split);
    }

    inline void include(const float3 in_v)
    {
      vmin.x = fminf(vmin.x, in_v.x);
      vmin.y = fminf(vmin.y, in_v.y);
      vmin.z = fminf(vmin.z, in_v.z);

      vmax.x = fmaxf(vmax.x, in_v.x);
      vmax.y = fmaxf(vmax.y, in_v.y);
      vmax.z = fmaxf(vmax.z, in_v.z);
    }

    inline void intersect(const Box3f& in_box)
    {
      vmin.x = fmaxf(vmin.x, in_box.vmin.x);
      vmin.y = fmaxf(vmin.y, in_box.vmin.y);
      vmin.z = fmaxf(vmin.z, in_box.vmin.z);

      vmax.x = fminf(vmax.x, in_box.vmax.x);
      vmax.y = fminf(vmax.y, in_box.vmax.y);
      vmax.z = fminf(vmax.z, in_box.vmax.z);
    }
  };

  struct Triangle
  {
    Triangle() {}

    float4 A; 
    float4 B; 
    float4 C; 
  };

  struct TriRef
  {
    Box3f box;
    int   triId;
    float metric;
  };

  inline Box3f TriBounds(const Triangle& a_tri)
  {
    Box3f res;

    res.vmin.x = fminf(a_tri.A.x, fminf(a_tri.B.x, a_tri.C.x));
    res.vmin.y = fminf(a_tri.A.y, fminf(a_tri.B.y, a_tri.C.y));
    res.vmin.z = fminf(a_tri.A.z, fminf(a_tri.B.z, a_tri.C.z));

    res.vmax.x = fmaxf(a_tri.A.x, fmaxf(a_tri.B.x, a_tri.C.x));
    res.vmax.y = fmaxf(a_tri.A.y, fmaxf(a_tri.B.y, a_tri.C.y));
    res.vmax.z = fmaxf(a_tri.A.z, fmaxf(a_tri.B.z, a_tri.C.z));

    return res;
  }

  inline float SurfaceArea(const Box3f& a_box)
  {
    float a = a_box.vmax.x - a_box.vmin.x;
    float b = a_box.vmax.y - a_box.vmin.y;
    float c = a_box.vmax.z - a_box.vmin.z;
    return 2.0f * (a*b + a*c + b*c);
  }

  inline float SurfaceAreaOfTriangle(const Triangle& tri) { return length(cross(to_float3(tri.C) - to_float3(tri.A), to_float3(tri.B) - to_float3(tri.A))); }
  inline float SurfaceAreaOfTriangle(const float3 v[3]) { return length(cross(v[2] - v[0], v[1] - v[0])); }


  const float SubdivMetric(const Triangle& a_tri, const Box3f& a_box)
  {
    float triSA = SurfaceAreaOfTriangle(a_tri);
    float boxSA = SurfaceArea(a_box);
    return (boxSA*boxSA) / fmaxf(triSA, 1e-6f);
  }

  inline float& f3_at(float3& a_f, const int a_index)
  {
    float* pArr = &a_f.x;
    return pArr[a_index];
  }

  void SplitReference(const TriRef& a_ref, TriRef* a_pLeft, TriRef* a_pRight, float splitPos, int splitAxis, float3 verts[3])
  {
    *a_pLeft  = a_ref;
    *a_pRight = a_ref;

    Box3f& leftBox       = a_pLeft->box;
    Box3f& rightBox      = a_pRight->box;
    const Box3f& nodeBox = a_ref.box;

    leftBox = rightBox = Box3f();

    if (!a_ref.box.axisAligned(splitAxis, splitPos))
    {
      float3 edges[3][2] = { { verts[0], verts[1] },
                             { verts[2], verts[0] },
                             { verts[1], verts[2] } };

      // grow both boxes with vertices and edge-plane intersections
      //
      for (int i = 0; i<3; i++)
      {
        float v0p = f3_at(edges[i][0], splitAxis);
        float v1p = f3_at(edges[i][1], splitAxis);

        // Insert vertex to the boxes it belongs to.
        //
        if (v0p <= splitPos)
          leftBox.include(edges[i][0]);

        if (v0p >= splitPos)
          rightBox.include(edges[i][0]);

        // Edge intersects the plane => insert intersection to both boxes.
        //
        if ((v0p < splitPos && v1p > splitPos) || (v0p > splitPos && v1p < splitPos))
        {
          float3 t = lerp(edges[i][0], edges[i][1], ::clamp((splitPos - v0p) / (v1p - v0p), 0.0f, 1.0f));

          leftBox.include(t);
          rightBox.include(t);
        }
      }
    }

    f3_at(leftBox.vmax ,splitAxis) = splitPos;
    f3_at(rightBox.vmin,splitAxis) = splitPos;

    // Intersect with original bounds.
    //
    leftBox.intersect(nodeBox);
    rightBox.intersect(nodeBox);
  }

  std::vector<TriRef> SubdivideTriRef(const TriRef triRef, float3 tverts[3])
  {
    std::vector<TriRef> subdivs;

    const float triSA    = SurfaceAreaOfTriangle(tverts);
    const float boxSA    = SurfaceArea(triRef.box);
    const float relation = triSA / boxSA;

    int maxSubdivs = 32; 

    if (relation <= ((1.0f / 1024.f)))
      maxSubdivs = 64;
    else if (relation <= ((1.0f / 256.f)))
      maxSubdivs = 48;
    else if (relation <= ((1.0f / 64.f)))
      maxSubdivs = 32;
    else if (relation <= ((1.0f / 32.f)))
      maxSubdivs = 24;
    else if (relation <= ((1.0f / 16.f)))
      maxSubdivs = 16;
    else if (relation <= ((1.0f / 8.f)))
      maxSubdivs = 8;
    else if (relation <= ((1.0f / 4.f)))
      maxSubdivs = 4;
    else
      maxSubdivs = 2;

    maxSubdivs = maxSubdivs + 1;

    subdivs.reserve(maxSubdivs);

    // now do subdiv
    //
    std::queue<TriRef> queue;
    queue.push(triRef);
    
    while (!queue.empty())
    {
      const size_t currSize = queue.size();
      auto prim = queue.front(); queue.pop();
      auto a    = prim;
      auto b    = prim;
    
      float3 center  = prim.box.center();
      float3 boxSize = prim.box.vmax - prim.box.vmin;
      float splitPos = 0;
      int splitAxis  = 0;
      float maxSize  = -1;
      for (int i = 0; i<3; i++)
      {
        if (maxSize < f3_at(boxSize,i))
        {
          splitPos  = f3_at(center, i);
          splitAxis = i;
          maxSize   = f3_at(boxSize, i);
        }
      }

      SplitReference(prim, &a, &b, splitPos, splitAxis, tverts);
    
      if (currSize + subdivs.size() < maxSubdivs)
        queue.push(a);
      else
        subdivs.push_back(a);
    
      if (currSize + subdivs.size() < maxSubdivs)
        queue.push(b);
      else
        subdivs.push_back(b);
    }

    return subdivs;
  }

  size_t SplitTriBoxes(const std::vector<Triangle>& a_inputTri, std::vector<TriRef>& a_outRefs)
  {
    size_t lastSplittedTriangle = 0;

    constexpr float SPLIT_MEM_EXPANSION_FACTOR = 0.25f;
    constexpr float SPLIT_SA_OK_THRESHOLD      = 0.001f;

    // if (a_inputTri.size() <= 8) // this mesh is too small
    // {
    //   a_outTri = a_inputTri;
    //   return 0;
    // }

    std::vector<TriRef> inputRefs(a_inputTri.size());

    // init triangles
    //
    Box3f meshBox;
    for (size_t i = 0; i < a_inputTri.size(); i++)
    {
      inputRefs[i].box       = TriBounds(a_inputTri[i]);
      inputRefs[i].triId     = int(i);
      inputRefs[i].metric    = SubdivMetric(a_inputTri[i], inputRefs[i].box);
      meshBox.include(inputRefs[i].box.vmin);
      meshBox.include(inputRefs[i].box.vmax);
    }

    std::sort(inputRefs.begin(), inputRefs.end(), [](auto a, auto b) {return (a.metric > b.metric); });

    const size_t auxSize = size_t(float(inputRefs.size())*SPLIT_MEM_EXPANSION_FACTOR) + 64;
    const size_t maxSize = inputRefs.size() + auxSize + 100;

    // now split first a_inputTri worse triangles
    //
    const float worseTriangleMetric  = inputRefs[0].metric;
    const float medianTriangleMetric = inputRefs[inputRefs.size()/2].metric;

    const float mBoxSA        = SurfaceArea(meshBox);
    const float medianSA      = SurfaceArea(inputRefs[inputRefs.size() / 2].box);
    const float surfaceAreaOk = mBoxSA*SPLIT_SA_OK_THRESHOLD;

    //if (2.0f*worseTriangleMetric > medianTriangleMetric && medianSA < surfaceAreaOk) // this mesh id ok.
    //{
    //  //a_outTri = inputRefs;
    //  return 0;
    //}

    a_outRefs.reserve(maxSize + 100);
    a_outRefs.resize(0);

    size_t currId = 0;
    for (; currId < inputRefs.size(); currId++)
    {
      const auto ref     = inputRefs[currId];
      const auto tri     = a_inputTri[ref.triId];
      const float currSA = SurfaceArea(ref.box);

      if (currSA > surfaceAreaOk)
      {
        float3 verts[3] = { to_float3(tri.A), to_float3(tri.B), to_float3(tri.C) };
        const std::vector<TriRef> subdivided = SubdivideTriRef(ref, verts);
        a_outRefs.insert(a_outRefs.end(), subdivided.begin(), subdivided.end());
      }
      else // #TODO: add check if SA == 0, discard such triangles
        a_outRefs.push_back(inputRefs[currId]);
        
      if (a_outRefs.size() >= auxSize)
      {
        lastSplittedTriangle = a_outRefs.size();
        break;
      }
    }

    if (currId + 1 < inputRefs.size())
      a_outRefs.insert(a_outRefs.end(), inputRefs.begin() + currId + 1, inputRefs.end()); // insert the rest tail

    return lastSplittedTriangle;
  }

};

bool RD_OGL1_TestSplit::UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input, const HRBatchInfo* a_batchList, int32_t a_listSize)
{
  using EarlySplit::Triangle;
  using EarlySplit::TriRef;

  std::vector<Triangle> trianglesUnsplit(a_input.triNum);

  for(int triId = 0; triId<a_input.triNum; triId++)
  {
    const int iA = a_input.indices[triId * 3 + 0];
    const int iB = a_input.indices[triId * 3 + 1];
    const int iC = a_input.indices[triId * 3 + 2];

    trianglesUnsplit[triId].A = float4(a_input.pos4f[iA * 4 + 0], a_input.pos4f[iA * 4 + 1], a_input.pos4f[iA * 4 + 2], 0.0f);
    trianglesUnsplit[triId].B = float4(a_input.pos4f[iB * 4 + 0], a_input.pos4f[iB * 4 + 1], a_input.pos4f[iB * 4 + 2], 0.0f);
    trianglesUnsplit[triId].C = float4(a_input.pos4f[iC * 4 + 0], a_input.pos4f[iC * 4 + 1], a_input.pos4f[iC * 4 + 2], 0.0f);
  }

  std::vector<TriRef> splitted;

  size_t lastSplittedTri = EarlySplit::SplitTriBoxes(trianglesUnsplit, splitted);

  // draw shitty boxes and mesh ... )
  //

   // draw solid mode
  //
  glNewList(m_displayLists + GLuint(a_meshId), GL_COMPILE);

  glColor3f(0.5f, 0.5f, 0.6f);
  glBegin(GL_TRIANGLES);
  for (int triId = 0; triId < a_input.triNum; triId++)
  {
    const int iA = a_input.indices[triId * 3 + 0];
    const int iB = a_input.indices[triId * 3 + 1];
    const int iC = a_input.indices[triId * 3 + 2];

    float4 A = float4(a_input.pos4f[iA * 4 + 0], a_input.pos4f[iA * 4 + 1], a_input.pos4f[iA * 4 + 2], 0.0f);
    float4 B = float4(a_input.pos4f[iB * 4 + 0], a_input.pos4f[iB * 4 + 1], a_input.pos4f[iB * 4 + 2], 0.0f);
    float4 C = float4(a_input.pos4f[iC * 4 + 0], a_input.pos4f[iC * 4 + 1], a_input.pos4f[iC * 4 + 2], 0.0f);

    glVertex3f(A.x, A.y, A.z);
    glVertex3f(B.x, B.y, B.z);
    glVertex3f(C.x, C.y, C.z);
  }
  glEnd();

  glEndList();

  // draw boxes
  //
  glNewList(m_displayLists + 2 * m_meshNum + GLuint(a_meshId), GL_COMPILE);
  for (size_t i = 0; i < splitted.size(); i++)
    DrawBox(splitted[i].box.vmin, splitted[i].box.vmax);
  glEndList();

  // wire frame mode
  //
  glNewList(m_displayLists + 3 * m_meshNum + GLuint(a_meshId), GL_COMPILE);

  glDisable(GL_TEXTURE_2D);
  glColor3f(1.0f, 0.75f, 0.25f);

  glBegin(GL_LINES);
  for (int triId = 0; triId < a_input.triNum; triId++)
  {
    const int iA = a_input.indices[triId * 3 + 0];
    const int iB = a_input.indices[triId * 3 + 1];
    const int iC = a_input.indices[triId * 3 + 2];

    float4 A = float4(a_input.pos4f[iA * 4 + 0], a_input.pos4f[iA * 4 + 1], a_input.pos4f[iA * 4 + 2], 0.0f);
    float4 B = float4(a_input.pos4f[iB * 4 + 0], a_input.pos4f[iB * 4 + 1], a_input.pos4f[iB * 4 + 2], 0.0f);
    float4 C = float4(a_input.pos4f[iC * 4 + 0], a_input.pos4f[iC * 4 + 1], a_input.pos4f[iC * 4 + 2], 0.0f);

    if (triId < lastSplittedTri)
      glColor3f(1.0f, 0.50f, 0.25f);
    else
      glColor3f(1.0f, 0.75f, 0.25f);

    glVertex3f(A.x, A.y, A.z);
    glVertex3f(B.x, B.y, B.z);

    glVertex3f(A.x, A.y, A.z);
    glVertex3f(C.x, C.y, C.z);

    glVertex3f(B.x, B.y, B.z);
    glVertex3f(C.x, C.y, C.z);
  }
  glEnd();

  glEndList();

  return true;
}



void RD_OGL1_TestSplit::InstanceMeshes(int32_t a_mesh_id, const float* a_matrices, int32_t a_instNum, const int* a_lightInstId, const int* a_remapId)
{
  glDepthFunc(GL_LEQUAL); // for drawing wire frame correctly

  for (int32_t i = 0; i < a_instNum; i++)
  {
    float matrixT[16];
    mat4x4_transpose(matrixT, (float*)(a_matrices + i * 16));

    glPushMatrix();
    glMultMatrixf(matrixT);

    glCallList(m_displayLists + GLuint(a_mesh_id));

    glColor3f(1.0f, 1.0f, 1.0f);
    glCallList(m_displayLists + 2*m_meshNum + GLuint(a_mesh_id)); // draw boxes

    glColor3f(1.0f, 1.0f, 1.0f);
    glCallList(m_displayLists + 3*m_meshNum + GLuint(a_mesh_id));

    glPopMatrix();
  }
}

void RD_OGL1_TestSplit::EndScene()
{
  glFlush();
}


IHRRenderDriver* CreateOpenGL1TestSplit_RenderDriver()
{
  return new RD_OGL1_TestSplit;
}
