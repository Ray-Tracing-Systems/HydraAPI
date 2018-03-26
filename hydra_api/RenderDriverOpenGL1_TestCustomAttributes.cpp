#include "HydraRenderDriverAPI.h"
#include "RenderDriverOpenGL1.h"



bool RD_OGL1_ShowCustomAttr::UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input, const HRBatchInfo* a_batchList, int32_t a_listSize)
{
  if (a_input.triNum == 0) 
  {
    glNewList(m_displayLists + GLuint(a_meshId), GL_COMPILE);
    glEndList();
    return true;
  }

  bool invalidMaterial    = (m_diffTexId.size() == 0);

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////// get custom aatrib pointers 
  
  const float* color4f    = nullptr;
  const float* darkness1f = nullptr;

  pugi::xml_node colorNode = a_meshNode.child(L"color");
  pugi::xml_node darkNode  = a_meshNode.child(L"darkness");
   
  if (colorNode != nullptr)
    color4f = (float*)(a_input.allData + colorNode.attribute(L"offset").as_int());
  
  if(darkNode != nullptr)
    darkness1f = (float*)(a_input.allData + darkNode.attribute(L"offset").as_int());
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  glNewList(m_displayLists + GLuint(a_meshId), GL_COMPILE);

  for (int32_t batchId = 0; batchId < a_listSize; batchId++)
  {
    HRBatchInfo batch         = a_batchList[batchId];
    const int drawElementsNum = batch.triEnd - batch.triBegin;

    float color[3] = { 1,1,1 };

    if (!invalidMaterial)
    {
      if (m_diffTexId[batch.matId] >= 0)
      {
        int texId = m_diffTexId[batch.matId];
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_texturesList[texId]);
      }
      else
        glDisable(GL_TEXTURE_2D);
    
      color[0] = m_diffColors[batch.matId * 3 + 0];
      color[1] = m_diffColors[batch.matId * 3 + 1];
      color[2] = m_diffColors[batch.matId * 3 + 2];
    
      glColor3fv(color);
    }
    else
    {
      glDisable(GL_TEXTURE_2D);
      glColor3f(1.0f, 1.0f, 1.0f);
    }


    glBegin(GL_TRIANGLES);
    for (int triId = batch.triBegin; triId < batch.triBegin + drawElementsNum; triId++)
    {
      const int v0 = a_input.indices[triId * 3 + 0];
      const int v1 = a_input.indices[triId * 3 + 1];
      const int v2 = a_input.indices[triId * 3 + 2];

      if (color4f != nullptr)
        glColor3fv(&color4f[v0 * 4 + 0]);
      else if (darkness1f != nullptr)
      {
        const float mul = darkness1f[v0];
        glColor3f(color[0]*mul, color[1] * mul, color[2] * mul);
      }

      glTexCoord2f(a_input.texcoord2f[v0 * 2 + 0], a_input.texcoord2f[v0 * 2 + 1]);
      glNormal3f(a_input.norm4f[v0 * 4 + 0], a_input.norm4f[v0 * 4 + 1], a_input.norm4f[v0 * 4 + 2]);
      glVertex3f(a_input.pos4f[v0 * 4 + 0], a_input.pos4f[v0 * 4 + 1], a_input.pos4f[v0 * 4 + 2]);

      if (color4f != nullptr)
        glColor3fv(&color4f[v1 * 4 + 0]);
      else if (darkness1f != nullptr)
      {
        const float mul = darkness1f[v1];
        glColor3f(color[0] * mul, color[1] * mul, color[2] * mul);
      }

      glTexCoord2f(a_input.texcoord2f[v1 * 2 + 0], a_input.texcoord2f[v1 * 2 + 1]);
      glNormal3f(a_input.norm4f[v1 * 4 + 0], a_input.norm4f[v1 * 4 + 1], a_input.norm4f[v1 * 4 + 2]);
      glVertex3f(a_input.pos4f[v1 * 4 + 0], a_input.pos4f[v1 * 4 + 1], a_input.pos4f[v1 * 4 + 2]);

      if (color4f != nullptr)
        glColor3fv(&color4f[v2 * 4 + 0]);
      else if (darkness1f != nullptr)
      {
        const float mul = darkness1f[v2];
        glColor3f(color[0] * mul, color[1] * mul, color[2] * mul);
      }

      glTexCoord2f(a_input.texcoord2f[v2 * 2 + 0], a_input.texcoord2f[v2 * 2 + 1]);
      glNormal3f(a_input.norm4f[v2 * 4 + 0], a_input.norm4f[v2 * 4 + 1], a_input.norm4f[v2 * 4 + 2]);
      glVertex3f(a_input.pos4f[v2 * 4 + 0], a_input.pos4f[v2 * 4 + 1], a_input.pos4f[v2 * 4 + 2]);
    }
    glEnd();
  }

  glEndList();

  return true;
}


IHRRenderDriver* CreateOpenGL1Debug_TestCustomAttributes()
{
  return new RD_OGL1_ShowCustomAttr;
}

