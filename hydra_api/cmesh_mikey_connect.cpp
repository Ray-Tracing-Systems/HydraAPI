#include "cmesh.h"
#include "cmesh_vsgf.h"

#include <cmath>
#include <fstream>

#include "../utils/mikktspace/mikktspace.h"

namespace cmesh
{
  // ***************************************************************** Mikey Tangent Space Calc *****************************************************************

  // Return number of primitives in the geometry.
  int getNumFaces(const SMikkTSpaceContext *context)
  {
    auto *pInput = static_cast <cmesh::SimpleMesh*> (context->m_pUserData);
  
    return int(pInput->indices.size()/3);
  }

  // Return number of vertices in the primitive given by index.
  int getNumVerticesOfFace(const SMikkTSpaceContext *context, const int primnum)
  {
    return 3;
  }

  // Write 3-float position of the vertex's point.
  void getPosition(const SMikkTSpaceContext *context, float outpos[], const int primnum, const int vtxnum)
  {
    auto *pInput = static_cast <cmesh::SimpleMesh*> (context->m_pUserData);
    auto index   = pInput->indices.at(primnum * 3 + vtxnum);

    outpos[0] = pInput->vPos4f.at(index * 4 + 0);
    outpos[1] = pInput->vPos4f.at(index * 4 + 1);
    outpos[2] = pInput->vPos4f.at(index * 4 + 2);
  }

  // Write 3-float vertex normal.
  void getNormal(const SMikkTSpaceContext *context, float outnormal[], const int primnum, const int vtxnum)
  {
    auto *pInput = static_cast <cmesh::SimpleMesh*> (context->m_pUserData);
    auto index   = pInput->indices.at(primnum * 3 + vtxnum);
  
    outnormal[0] = pInput->vNorm4f.at(index * 4 + 0);
    outnormal[1] = pInput->vNorm4f.at(index * 4 + 1);
    outnormal[2] = pInput->vNorm4f.at(index * 4 + 2);
  }

  // Write 2-float vertex uv.
  void getTexCoord(const SMikkTSpaceContext *context, float outuv[], const int primnum, const int vtxnum)
  {
    auto *pInput = static_cast <cmesh::SimpleMesh*> (context->m_pUserData);
    auto index   = pInput->indices.at(primnum * 3 + vtxnum);
  
    outuv[0] = pInput->vTexCoord2f.at(index * 2 + 0);
    outuv[1] = pInput->vTexCoord2f.at(index * 2 + 1);
  }

  // Compute and set attributes on the geometry vertex. Basic version.
  void setTSpaceBasic(const SMikkTSpaceContext *context,
                      const float tangentu[],
                      const float sign,
                      const int primnum,
                      const int vtxnum)
  {
    auto *pInput = static_cast <cmesh::SimpleMesh*> (context->m_pUserData);
    auto index   = pInput->indices.at(primnum * 3 + vtxnum);
  
    pInput->vTang4f.at(index * 4 + 0) = tangentu[0];
    pInput->vTang4f.at(index * 4 + 1) = tangentu[1];
    pInput->vTang4f.at(index * 4 + 2) = tangentu[2];
    pInput->vTang4f.at(index * 4 + 3) = sign;
  }

  void setTSpace(const SMikkTSpaceContext *context,
                 const float tangentu[],
                 const float tangentv[],
                 const float magu,
                 const float magv,
                 const tbool keep,
                 const int primnum,
                 const int vtxnum)
  {
    auto *pInput = static_cast <cmesh::SimpleMesh*> (context->m_pUserData);
    auto index   = pInput->indices[primnum * 3 + vtxnum];
  
    pInput->vTang4f[index * 4 + 0] = tangentu[0];
    pInput->vTang4f[index * 4 + 1] = tangentu[1];
    pInput->vTang4f[index * 4 + 2] = tangentu[2];
    pInput->vTang4f[index * 4 + 3] = 1.0f;
  }

  void MikeyTSpaceCalc(cmesh::SimpleMesh* pInput, bool basic)
  { 
    assert(pInput != nullptr);
  
    SMikkTSpaceInterface iface;
    iface.m_getNumFaces          = getNumFaces;
    iface.m_getNumVerticesOfFace = getNumVerticesOfFace;
    iface.m_getPosition          = getPosition;
    iface.m_getNormal            = getNormal;
    iface.m_getTexCoord          = getTexCoord;
    iface.m_setTSpaceBasic       = basic ? setTSpaceBasic : nullptr;
    iface.m_setTSpace            = basic ? nullptr : setTSpace;
  
    SMikkTSpaceContext context;
    context.m_pInterface = &iface;
    context.m_pUserData  = pInput;
  
    genTangSpaceDefault(&context);
  }

};
