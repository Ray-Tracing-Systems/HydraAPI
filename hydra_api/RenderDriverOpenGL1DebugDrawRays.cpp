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

  std::vector<std::vector<float4> >  m_raysPos;
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
  
  glBegin(GL_LINE_STRIP);
  for (int vIndex = 0; vIndex < m_raysPos[rayIndex].size(); vIndex++)
  {
    if (m_raysPos[rayIndex][vIndex].w < 0.0f)
      glColor3f(1, 0, 0);
    else
      glColor3f(0, 1, 0);

    glVertex3fv(&m_raysPos[rayIndex][vIndex].x);
  }
  glEnd();

  // glColor3f(1, 1, 0);
  // glPointSize(3.0f);
  // glBegin(GL_POINTS);
  // for(size_t i=0;i<m_raysPos.size();i++)
  //   glVertex3fv(&m_raysPos[i].x);
  // glEnd();

  //glBegin(GL_LINES);
  //for (size_t i = 0; i < m_raysPos.size(); i++)
  //{
  //  float3 point2 = m_raysPos[i] + m_raysDir[i] * 0.1f;
  //  glVertex3fv(&m_raysPos[i].x);
  //  glVertex3fv(&point2.x);
  //}
  //glEnd();

}

void RD_OGL1_DebugDrawRays::LoadRays()
{
 std::string path = "D:/PROG/HydraAPP/hydra_app/z_rays2.txt";
 
 m_raysPos.resize(0); m_raysPos.reserve(100);
 
 std::ifstream fin(path.c_str());
 if (!fin.is_open())
   return;
 

 int n; 
 fin >> n;
 
 for (int i = 0; i < n; i++) 
 {
   int d = 0;
   fin >> d;
 
   std::vector<float4> path(d);
   for (int j = 0; j < d; j++) 
   {
     float4 rayPos;
     fin >> rayPos.x >> rayPos.y >> rayPos.z >> rayPos.w;
     path[j] = rayPos;
   }
 
   m_raysPos.push_back(path);
 }

 std::cout << "after loading rays ... " << std::endl;

}


IHRRenderDriver* CreateOpenGL1DrawRays_RenderDriver() { return new RD_OGL1_DebugDrawRays; }

