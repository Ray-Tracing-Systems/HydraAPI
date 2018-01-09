#include "HydraRenderDriverAPI.h"
#include "RenderDriverOpenGL1.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include "LiteMath.h"
using namespace HydraLiteMath;

#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include <string>

extern int g_drawBVHIdInput;

struct RD_OGL1_DebugDrawRays : public RD_OGL1_Debug
{
  constexpr static int DEBUG_PATH_MAX_DEPTH  = 6;     ///< this is for debug needs only
  constexpr static int DEBUG_PATH_MAX_NUMBER = 1024;  ///< this is for debug needs only

  typedef RD_OGL1_Debug Base;

  RD_OGL1_DebugDrawRays()
  {
    m_drawWire = true;
    LoadRays();
  }

  ~RD_OGL1_DebugDrawRays() { }

  void BeginScene() override;
  void EndScene  () override;

protected:

  void LoadRays();
  int  m_numDebugRays;
  int  m_rayDepth;

  std::vector<float4> m_raysPos;
  std::vector<float4> m_raysDir;
};


void RD_OGL1_DebugDrawRays::BeginScene()
{
  m_drawWire = true;
  Base::BeginScene();
}

void RD_OGL1_DebugDrawRays::EndScene()
{
  Base::EndScene();
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
 
  int rayIndex = g_drawBVHIdInput;
  
  glColor3f(1, 1, 1);
  
  // glBegin(GL_LINE_STRIP);
  // for (int vIndex = 0; vIndex < m_raysPos[rayIndex].size(); vIndex++)
  // {
  //   if (m_raysPos[rayIndex][vIndex].w < 0.0f)
  //     glColor3f(1, 0, 0);
  //   else
  //     glColor3f(0, 1, 0);
  // 
  //   glVertex3fv(&m_raysPos[rayIndex][vIndex].x);
  // }
  // glEnd();

  glColor3f(1, 0, 0);

  glBegin(GL_LINES);
  for (size_t i = 0; i < m_raysPos.size(); i++)
  {
    if (i % 100 == 0)
    {
      float3 point2 = to_float3(m_raysPos[i]) + to_float3(m_raysDir[i]) * 1.25f;
      glVertex3fv(&m_raysPos[i].x);
      glVertex3fv(&point2.x);
    }
  }
  glEnd();

}

void RD_OGL1_DebugDrawRays::LoadRays()
{
 std::string path1 = "D:/PROG/HydraCore/hydra_app/z_rpos.array4f";
 std::string path2 = "D:/PROG/HydraCore/hydra_app/z_rdir.array4f";

 std::ifstream fin1(path1.c_str(), std::iostream::binary);
 std::ifstream fin2(path2.c_str(), std::iostream::binary);

 int size1 = 0, size2 = 0;

 fin1.read((char*)&size1, sizeof(int));
 fin2.read((char*)&size2, sizeof(int));

 m_raysPos.resize(size1);
 m_raysDir.resize(size2);
 
 fin1.read((char*)&m_raysPos[0], size1*sizeof(float)*4);
 fin2.read((char*)&m_raysDir[0], size2*sizeof(float)*4);

 fin1.close();
 fin2.close();

 std::cout << "after loading rays ... " << std::endl;
}


IHRRenderDriver* CreateOpenGL1DrawRays_RenderDriver() { return new RD_OGL1_DebugDrawRays; }

