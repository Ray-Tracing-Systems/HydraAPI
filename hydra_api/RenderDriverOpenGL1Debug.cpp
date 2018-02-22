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


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define RADPERDEG 0.0174533

static void Arrow(GLdouble x1, GLdouble y1, GLdouble z1, GLdouble x2, GLdouble y2, GLdouble z2, GLdouble D)
{
  double x = x2 - x1;
  double y = y2 - y1;
  double z = z2 - z1;
  double L = sqrt(x*x + y*y + z*z);

  GLUquadricObj *quadObj;

  glPushMatrix();

  glTranslated(x1, y1, z1);

  if ((x != 0.) || (y != 0.)) {
    glRotated(atan2(y, x) / RADPERDEG, 0., 0., 1.);
    glRotated(atan2(sqrt(x*x + y*y), z) / RADPERDEG, 0., 1., 0.);
  }
  else if (z<0) {
    glRotated(180, 1., 0., 0.);
  }

  glTranslatef(0, 0, GLfloat(L) - 4.0f * GLfloat(D));

  quadObj = gluNewQuadric();
  gluQuadricDrawStyle(quadObj, GLU_FILL);
  gluQuadricNormals(quadObj, GLU_SMOOTH);
  gluCylinder(quadObj, 2 * D, 0.0, 4 * D, 32, 1);
  gluDeleteQuadric(quadObj);

  quadObj = gluNewQuadric();
  gluQuadricDrawStyle(quadObj, GLU_FILL);
  gluQuadricNormals(quadObj, GLU_SMOOTH);
  gluDisk(quadObj, 0.0, 2 * D, 32, 1);
  gluDeleteQuadric(quadObj);

  glTranslatef(0, 0, GLfloat (-L) + 4.0f * GLfloat(D));

  quadObj = gluNewQuadric();
  gluQuadricDrawStyle(quadObj, GLU_FILL);
  gluQuadricNormals(quadObj, GLU_SMOOTH);
  gluCylinder(quadObj, D, D, L - 4 * D, 32, 1);
  gluDeleteQuadric(quadObj);

  quadObj = gluNewQuadric();
  gluQuadricDrawStyle(quadObj, GLU_FILL);
  gluQuadricNormals(quadObj, GLU_SMOOTH);
  gluDisk(quadObj, 0.0, D, 32, 1);
  gluDeleteQuadric(quadObj);

  glPopMatrix();

}

static void DrawAxes(GLdouble length, GLdouble thickness)
{
  glColor3f(1, 0, 0);
  glPushMatrix();
  //glTranslatef(GLfloat(-length), 0, 0);
  Arrow(0, 0, 0, 2 * length, 0, 0, thickness);
  glPopMatrix();

  glColor3f(0, 1, 0);
  glPushMatrix();
  //glTranslatef(0, GLfloat (-length), 0);
  Arrow(0, 0, 0, 0, 2 * length, 0, thickness);
  glPopMatrix();

  glColor3f(0, 0, 1);
  glPushMatrix();
  //glTranslatef(0, 0, GLfloat (-length));
  Arrow(0, 0, 0, 0, 0, 2 * length, thickness);
  glPopMatrix();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void RD_OGL1_Debug::ClearAll()
{
	RD_OGL1_Plain::ClearAll();
	m_meshNum = 0;
}

HRDriverAllocInfo RD_OGL1_Debug::AllocAll(HRDriverAllocInfo a_info)
{
	m_meshNum      = a_info.geomNum;
  m_listNum      = GLsizei(a_info.geomNum*4);
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


bool RD_OGL1_Debug::UpdateSettings(pugi::xml_node a_settingsNode)
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

  if(a_settingsNode.child(L"draw_solid") != nullptr)
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
    m_msg = L"RD_OGL1_Debug::UpdateSettings, bad input resolution";
    return false;
  }

  return true;
}


bool RD_OGL1_Debug::UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input, const HRBatchInfo* a_batchList, int32_t a_listSize)
{
  if (a_input.triNum == 0)
  {
    glNewList(m_displayLists + GLuint(a_meshId), GL_COMPILE);
    glEndList();

    glNewList(m_displayLists + 3 * m_meshNum + GLuint(a_meshId), GL_COMPILE);
    glEndList();

    return true;
  }

  bool invalidMaterial = (m_diffTexId.size() == 0);

  // DebugPrintMesh(a_input, "z_mesh.txt");
  
  // draw solid mode
  //
  glNewList(m_displayLists + GLuint(a_meshId), GL_COMPILE);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glVertexPointer(4, GL_FLOAT, 0, a_input.pos4f);
  glNormalPointer(GL_FLOAT, sizeof(float) * 4, a_input.norm4f);
  glTexCoordPointer(2, GL_FLOAT, 0, a_input.texcoord2f);

  for (int32_t batchId = 0; batchId < a_listSize; batchId++)
  {
    HRBatchInfo batch = a_batchList[batchId];
    
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

      glColor3fv(&m_diffColors[batch.matId * 3 + 0]);
    }
    else
    {
      glDisable(GL_TEXTURE_2D);
      glColor3f(1.0f, 1.0f, 1.0f);
    }
    const int drawElementsNum = batch.triEnd - batch.triBegin;

    glDrawElements(GL_TRIANGLES, drawElementsNum*3, GL_UNSIGNED_INT, a_input.indices + batch.triBegin*3);
  }

  glEndList();

  // draw normals
  //
	glNewList(m_displayLists + m_meshNum + GLuint(a_meshId), GL_COMPILE);

  glBegin(GL_LINES);
	for (int i = 0; i < a_input.vertNum; i++)
	{
		float4 A = float4(a_input.pos4f[i*4 + 0], a_input.pos4f[i*4 + 1], a_input.pos4f[i*4 + 2], a_input.pos4f[i*4 + 3]);
		float4 B = float4(a_input.norm4f[i*4 + 0], a_input.norm4f[i*4 + 1], a_input.norm4f[i*4 + 2], a_input.norm4f[i*4 + 3]);
		glVertex3f(A.x, A.y, A.z);
		glVertex3f(A.x + m_renderNormalLength*B.x, A.y + m_renderNormalLength*B.y, A.z + m_renderNormalLength*B.z);
	}
  glEnd();

	glEndList();

  // draw tangent
  //
  glNewList(m_displayLists + 2*m_meshNum + GLuint(a_meshId), GL_COMPILE);

  glBegin(GL_LINES);
  for (int i = 0; i < a_input.vertNum; i++)
  {
    float4 A = float4(a_input.pos4f[i * 4 + 0], a_input.pos4f[i * 4 + 1], a_input.pos4f[i * 4 + 2], 0.0f);
    float4 B = float4(a_input.tan4f[i * 4 + 0], a_input.tan4f[i * 4 + 1], a_input.tan4f[i * 4 + 2], 0.0f);
    glVertex3f(A.x, A.y, A.z);
    glVertex3f(A.x + m_renderNormalLength*B.x, A.y + m_renderNormalLength*B.y, A.z + m_renderNormalLength*B.z);
  }
  glEnd();

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


void RD_OGL1_Debug::InstanceMeshes(int32_t a_mesh_id, const float* a_matrices, int32_t a_instNum, const int* a_lightInstId, const int* a_remapId)
{
  glDepthFunc(GL_LEQUAL); // for drawing wire frame correctly

  for (int32_t i = 0; i < a_instNum; i++)
  {
    float matrixT[16];
    mat4x4_transpose(matrixT, (float*)(a_matrices + i*16));

    glPushMatrix();
    glMultMatrixf(matrixT);
    
    if(m_drawSolid)
      glCallList(m_displayLists + GLuint(a_mesh_id));
    
    glColor3f(1.0f, 0.0f, 0.0f);
    if(m_drawNormals)
		  glCallList(m_displayLists + m_meshNum + GLuint(a_mesh_id));
    
    glColor3f(0.0f, 1.0f, 0.0f);
    if (m_drawTangents)
      glCallList(m_displayLists + 2*m_meshNum + GLuint(a_mesh_id));
   
    //glLineWidth(1.0f);
    //glEnable(GL_LINE_SMOOTH);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    //glPushMatrix();
    //glScalef(1.001f, 1.001f, 1.001f);
    glColor3f(1.0f, 1.0f, 1.0f);
    if (m_drawWire)
      glCallList(m_displayLists + 3*m_meshNum + GLuint(a_mesh_id));
    //glPopMatrix();

    glPopMatrix();
  }
}

void RD_OGL1_Debug::EndScene()
{
  if (m_drawAxis)
    DrawAxes(m_axisArrorLen, m_axisArrorThickness);

  glFlush();
}


IHRRenderDriver* CreateOpenGL1Debug_RenderDriver()
{
  return new RD_OGL1_Debug;
}
