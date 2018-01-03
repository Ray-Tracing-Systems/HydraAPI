#include "HydraRenderDriverAPI.h"
#include "RenderDriverOpenGL1.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include "LiteMath.h"
using namespace HydraLiteMath;

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include <assert.h>

#define MAX_BVH_LEVELS 32

int g_drawBVHIdInput = 0;

static void glutWireCube(GLdouble dSize)
{
  float s = float(dSize * 0.5f);

  glBegin(GL_LINE_LOOP); glVertex3d(+s, -s, +s); glVertex3d(+s, -s, -s); glVertex3d(+s, +s, -s); glVertex3d(+s, +s, +s); glEnd(); 
  glBegin(GL_LINE_LOOP); glVertex3d(+s, +s, +s); glVertex3d(+s, +s, -s); glVertex3d(-s, +s, -s); glVertex3d(-s, +s, +s); glEnd(); 
  glBegin(GL_LINE_LOOP); glVertex3d(+s, +s, +s); glVertex3d(-s, +s, +s); glVertex3d(-s, -s, +s); glVertex3d(+s, -s, +s); glEnd(); 
  glBegin(GL_LINE_LOOP); glVertex3d(-s, -s, +s); glVertex3d(-s, +s, +s); glVertex3d(-s, +s, -s); glVertex3d(-s, -s, -s); glEnd(); 
  glBegin(GL_LINE_LOOP); glVertex3d(-s, -s, +s); glVertex3d(-s, -s, -s); glVertex3d(+s, -s, -s); glVertex3d(+s, -s, +s); glEnd(); 
  glBegin(GL_LINE_LOOP); glVertex3d(-s, -s, -s); glVertex3d(-s, +s, -s); glVertex3d(+s, +s, -s); glVertex3d(+s, -s, -s); glEnd(); 

}


static void DrawBox(const float4 a_boxMin, const float4 a_boxMax)
{
  const float4 scale = a_boxMax - a_boxMin;
  const float4 cent  = a_boxMin + 0.5f*scale;

  glPushMatrix();
  glTranslatef(cent.x, cent.y, cent.z);
  glScalef(scale.x, scale.y, scale.z);
  glutWireCube(1);
  glPopMatrix();
}


struct RD_OGL1_DebugDrawBVH : public RD_OGL1_Debug
{
  typedef RD_OGL1_Debug Base;

  RD_OGL1_DebugDrawBVH()  
  { 
    m_boxesDisplayLists = glGenLists(GLsizei(MAX_BVH_LEVELS)); 
    LoadRays(); 
  }

  ~RD_OGL1_DebugDrawBVH() { glDeleteLists(m_boxesDisplayLists, MAX_BVH_LEVELS); }

  void BeginScene() override;
  void EndScene() override;

protected:

  void LoadRays();

  GLuint m_boxesDisplayLists;
  int m_numDebugRays;

};


void RD_OGL1_DebugDrawBVH::BeginScene()
{
  Base::BeginScene();

}

void RD_OGL1_DebugDrawBVH::EndScene()
{
  Base::EndScene();

  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glColor3f(0, 1, 0);
  glCallList(m_boxesDisplayLists + g_drawBVHIdInput);
}


void RD_OGL1_DebugDrawBVH::LoadRays()
{
 
  bool fileIsOpened = false;
  int  currLevel = 1;
  std::string path = "D:/temp/bvh_layers2";
  
  m_numDebugRays = 0;
   
  do
  {
    std::stringstream fileNameStream;
    fileNameStream << path.c_str() << "/level_" << currLevel << ".array4f";
    std::stringstream fileNameStream2;
    fileNameStream2 << path.c_str() << "/matrix_level_" << currLevel << ".array4f";

    const std::string fileName1 = fileNameStream.str();
    const std::string fileName2 = fileNameStream2.str();
  
    std::cout << "loading level = " << currLevel << std::endl;
  
    int arraySize = 0;
    int arraySize2 = 0;
  
    std::ifstream fin(fileName1.c_str(), std::ios::binary);
    std::ifstream fin2(fileName2.c_str(), std::ios::binary);
  
    fileIsOpened = fin.is_open();
    if (fileIsOpened)
    {
      fin.read((char*)&arraySize, sizeof(int));
      fin2.read((char*)&arraySize2, sizeof(int));
   
      assert(arraySize2 == arraySize*2);

      std::vector<float4> boxesData(arraySize);
      fin.read((char*)&boxesData[0], boxesData.size() * sizeof(float4));
      
      std::vector<float4x4> matrixData(arraySize/2);
      fin2.read((char*)&matrixData[0], matrixData.size() * sizeof(float4x4));

      fin.close();
      fin2.close();

      glNewList(m_boxesDisplayLists + GLuint(m_numDebugRays), GL_COMPILE);
  
      for (size_t i = 0; i < boxesData.size(); i += 2)
      {
        float4   boxMin  = boxesData[i + 0];
        float4   boxMax  = boxesData[i + 1];
        float4x4 matrixT = transpose(matrixData[i / 2]);

        glPushMatrix();
        glMultMatrixf(matrixT.L());
        DrawBox(boxMin, boxMax);
        glPopMatrix();
      }

      glEndList();
    }
  
    currLevel++;
    m_numDebugRays++;
  
    if (currLevel >= 22)
    {
      m_numDebugRays++;
      break;
    }
  
  } while (fileIsOpened && m_numDebugRays < MAX_BVH_LEVELS);
  
  
  std::cout << "after loading bvh levels ... " << std::endl;

}


IHRRenderDriver* CreateOpenGL1DrawBVH_RenderDriver() { return new RD_OGL1_DebugDrawBVH;}
