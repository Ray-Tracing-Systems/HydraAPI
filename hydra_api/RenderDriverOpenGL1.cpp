#include "RenderDriverOpenGL1.h"
#include "LiteMath.h"
using namespace HydraLiteMath;

#include <iostream>

void RD_OGL1_Plain::ClearAll()
{
  if (m_displayLists != -1)
    glDeleteLists(m_displayLists, m_listNum);

  if(!m_texturesList.empty())
    glDeleteTextures(m_texturesList.size(), m_texturesList.data());
  
  m_displayLists = -1;
  m_listNum      = 0;

  m_texturesList.resize(0);

  m_diffColors.resize(0);
  m_diffTexId.resize(0);

}

HRDriverAllocInfo RD_OGL1_Plain::AllocAll(HRDriverAllocInfo a_info)
{
  m_listNum      = GLsizei(a_info.geomNum);
  m_displayLists = glGenLists(GLsizei(a_info.geomNum));

  m_texturesList.resize(a_info.imgNum);
  glGenTextures(GLsizei(a_info.imgNum), &m_texturesList[0]);

  m_diffColors.resize(a_info.matNum * 3);
  m_diffTexId.resize(a_info.matNum);

  for (size_t i = 0; i < m_diffTexId.size(); i++)
    m_diffTexId[i] = -1;

  m_libPath = std::wstring(a_info.libraryPath);

  return a_info;
}

HRDriverInfo RD_OGL1_Plain::Info() 
{
  HRDriverInfo info; // very simple render driver implementation, does not support any other/advanced stuff

  info.supportHDRFrameBuffer        = false;
  info.supportHDRTextures           = false;
  info.supportMultiMaterialInstance = false;

  info.supportImageLoadFromInternalFormat = false;
  info.supportImageLoadFromExternalFormat = false;
  info.supportMeshLoadFromInternalFormat  = false;
  info.supportLighting                    = false;
  
  info.memTotal = int64_t(8) * int64_t(1024 * 1024 * 1024);

  return info;
}

#pragma warning(disable:4996) // for wcscpy to be ok

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// typedef void (APIENTRYP PFNGLGENERATEMIPMAPPROC)(GLenum target);
// PFNGLGENERATEMIPMAPPROC glad_glGenerateMipmap;
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


float toneExposure(float vColor, float average)
{
	float T = pow(average, -1);

	float result = 0.0;
	result = 1 - exp(-T * vColor);

	return result;
}

bool RD_OGL1_Plain::UpdateImage(int32_t a_texId, int32_t w, int32_t h, int32_t bpp, const void* a_data, pugi::xml_node a_texNode)
{
  if (a_data == nullptr) 
    return false; 

	GLubyte *convertedData = nullptr;
	
	if (bpp > 4) // well, perhaps this is not error, we just don't support hdr textures in this render
	{
		convertedData = new GLubyte[w*h*bpp/sizeof(float)];

		#pragma omp parallel for
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				float r = ((float*)a_data)[(y*w + x) * 4 + 0];
				float g = ((float*)a_data)[(y*w + x) * 4 + 1];
				float b = ((float*)a_data)[(y*w + x) * 4 + 2];
				float a = ((float*)a_data)[(y*w + x) * 4 + 3];

				convertedData[(y*w + x) * 4 + 0] = GLubyte(clamp(r, 0.0, 1.0) * 255.0f);
				convertedData[(y*w + x) * 4 + 1] = GLubyte(clamp(g, 0.0, 1.0) * 255.0f);
				convertedData[(y*w + x) * 4 + 2] = GLubyte(clamp(b, 0.0, 1.0) * 255.0f);
				convertedData[(y*w + x) * 4 + 3] = GLubyte(clamp(a, 0.0, 1.0) * 255.0f);
			}
		}
	}
    

  glBindTexture(GL_TEXTURE_2D, m_texturesList[a_texId]);
	if (bpp > 4)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, convertedData);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, a_data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  //glGenerateMipmap(GL_TEXTURE_2D); // this function is from OpenGL 3.0
	if (bpp > 4)
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, w, h, GL_RGBA, GL_UNSIGNED_BYTE, convertedData);
	else
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, w, h, GL_RGBA, GL_UNSIGNED_BYTE, a_data);
  
	if (bpp > 4) 
    delete [] convertedData;

  return true;
}


bool RD_OGL1_Plain::UpdateMaterial(int32_t a_matId, pugi::xml_node a_materialNode)
{
  pugi::xml_node clrNode = a_materialNode.child(L"diffuse").child(L"color");
  pugi::xml_node texNode = a_materialNode.child(L"diffuse").child(L"texture");
  pugi::xml_node mtxNode = a_materialNode.child(L"diffuse").child(L"sampler").child(L"matrix");

  if (clrNode == nullptr)
    clrNode = a_materialNode.child(L"emission").child(L"color"); // no diffuse color ? => draw emission color instead!

  if (clrNode != nullptr)
  {
    const wchar_t* clrStr = nullptr;
    
    if (clrNode.attribute(L"val") != nullptr)
      clrStr = clrNode.attribute(L"val").as_string();
    else
      clrStr = clrNode.text().as_string();

    if (std::wstring(clrStr) != L"")
    {
      float color[3];
      std::wstringstream input(clrStr);
      input >> color[0] >> color[1] >> color[2];

      m_diffColors[a_matId * 3 + 0] = color[0];
      m_diffColors[a_matId * 3 + 1] = color[1];
      m_diffColors[a_matId * 3 + 2] = color[2];
    }
  }

  if (texNode != nullptr)
    m_diffTexId[a_matId] = texNode.attribute(L"id").as_int();
  else
    m_diffTexId[a_matId] = -1;

  return true;
}

bool RD_OGL1_Plain::UpdateLight(int32_t a_lightIdId, pugi::xml_node a_lightNode)
{
  return true;
}


bool RD_OGL1_Plain::UpdateCamera(pugi::xml_node a_camNode)
{
  if (a_camNode == nullptr)
    return true;

  this->camUseMatrices = false;

  if (std::wstring(a_camNode.attribute(L"type").as_string()) == L"two_matrices")
  {
    const wchar_t* m1 = a_camNode.child(L"mWorldView").text().as_string();
    const wchar_t* m2 = a_camNode.child(L"mProj").text().as_string();

    float mWorldView[16];
    float mProj[16];

    std::wstringstream str1(m1), str2(m2);
    for (int i = 0; i < 16; i++)
    {
      str1 >> mWorldView[i];
      str2 >> mProj[i];
    }

    this->camWorldViewMartrixTransposed = transpose(float4x4(mWorldView));
    this->camProjMatrixTransposed       = transpose(float4x4(mProj));
    this->camUseMatrices                = true;

    return true;
  }

  const wchar_t* camPosStr = a_camNode.child(L"position").text().as_string();
  const wchar_t* camLAtStr = a_camNode.child(L"look_at").text().as_string();
  const wchar_t* camUpStr  = a_camNode.child(L"up").text().as_string();
  //const wchar_t* testStr   = a_camNode.child(L"test").text().as_string();

  if (!a_camNode.child(L"fov").text().empty())
    camFov = a_camNode.child(L"fov").text().as_float();

  if (!a_camNode.child(L"nearClipPlane").text().empty())
    camNearPlane = a_camNode.child(L"nearClipPlane").text().as_float();

  if (!a_camNode.child(L"farClipPlane").text().empty())
    camFarPlane = a_camNode.child(L"farClipPlane").text().as_float();

  if (std::wstring(camPosStr) != L"")
  {
    std::wstringstream input(camPosStr);
    input >> camPos[0] >> camPos[1] >> camPos[2];
  }

  if (std::wstring(camLAtStr) != L"")
  {
    std::wstringstream input(camLAtStr);
    input >> camLookAt[0] >> camLookAt[1] >> camLookAt[2];
  }

  if (std::wstring(camUpStr) != L"")
  {
    std::wstringstream input(camUpStr);
    input >> camUp[0] >> camUp[1] >> camUp[2];
  }

  return true;
}

bool RD_OGL1_Plain::UpdateSettings(pugi::xml_node a_settingsNode)
{
  if (a_settingsNode.child(L"width") != nullptr)
    m_width = a_settingsNode.child(L"width").text().as_int();

  if (a_settingsNode.child(L"height") != nullptr)
    m_height = a_settingsNode.child(L"height").text().as_int();

  if (m_width < 0 || m_height < 0)
  {
    if (m_pInfoCallBack != nullptr)
      m_pInfoCallBack(L"bad input resolution", L"RD_OGL1_Plain::UpdateSettings", HR_SEVERITY_ERROR);
    return false;
  }

  return true;
}

void _hrDebugPrintMesh(const HRMeshDriverInput& a_input, const wchar_t* a_fileName)
{
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
  std::wstring s1(a_fileName);
  std::string  s2(s1.begin(), s1.end());
  std::ofstream fout(s2.c_str(), std::ios::binary);
#elif defined WIN32
  std::ofstream fout(a_fileName, std::ios::binary);
#endif

  fout << "vertNum = " << a_input.vertNum << std::endl;
  fout << "triNum  = " << a_input.triNum << std::endl;

  fout << "vpos  = [ " << std::endl;

  for (int i = 0; i < a_input.vertNum; i++)
    fout << "  " << a_input.pos4f[i * 4 + 0] << ", " << a_input.pos4f[i * 4 + 1] << ", " << a_input.pos4f[i * 4 + 2] << ", " << a_input.pos4f[i * 4 + 3] << std::endl;

  fout << "]" << std::endl << std::endl;


  fout << "vnorm  = [ " << std::endl;

  for (int i = 0; i < a_input.vertNum; i++)
    fout << "  " << a_input.norm4f[i * 4 + 0] << ", " << a_input.norm4f[i * 4 + 1] << ", " << a_input.norm4f[i * 4 + 2] << ", " << a_input.norm4f[i * 4 + 3] << std::endl;

  fout << "]" << std::endl << std::endl;


  fout << "vtxcoord  = [ " << std::endl;

  for (int i = 0; i < a_input.vertNum; i++)
    fout << "  " << a_input.texcoord2f[i * 2 + 0] << ", " << a_input.texcoord2f[i * 2 + 1] << std::endl;

  fout << "]" << std::endl << std::endl;


  fout << "indices  = [ " << std::endl;

  for (int i = 0; i < a_input.triNum; i++)
    fout << "  " << a_input.indices[i * 3 + 0] << ", " << a_input.indices[i * 3 + 1] << ", " << a_input.indices[i * 3 + 2] << std::endl;

  fout << "]" << std::endl << std::endl;

  fout << "mindices  = [ " << std::endl;

  for (int i = 0; i < a_input.triNum; i++)
    fout << "  " << a_input.triMatIndices[i] << std::endl;

  fout << "]" << std::endl << std::endl;

  fout.close();
}

#include "HydraVSGFExport.h"

void _hrDebugPrintVSGF(const wchar_t* a_fileNameIn, const wchar_t* a_fileNameOut)
{
  HydraGeomData data;
  data.read(a_fileNameIn);

  HRMeshDriverInput mi;

  mi.triNum = data.getIndicesNumber() / 3;
  mi.vertNum = data.getVerticesNumber();
  mi.indices = (int*)data.getTriangleVertexIndicesArray();
  mi.triMatIndices = (int*)data.getTriangleMaterialIndicesArray();

  mi.pos4f = (float*)data.getVertexPositionsFloat4Array();
  mi.norm4f = (float*)data.getVertexNormalsFloat4Array();
  mi.tan4f = (float*)data.getVertexTangentsFloat4Array();
  mi.texcoord2f = (float*)data.getVertexTexcoordFloat2Array();

  _hrDebugPrintMesh(mi, a_fileNameOut);
}


bool RD_OGL1_Plain::UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input, const HRBatchInfo* a_batchList, int32_t a_listSize)
{
  if (a_input.triNum == 0) // don't support loading mesh from file 'a_fileName'
  {
    glNewList(m_displayLists + GLuint(a_meshId), GL_COMPILE);
    glEndList();
    return true;
  }

  bool invalidMaterial = m_diffTexId.empty();

  //DebugPrintMesh(a_input, "z_mesh.txt");

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

    // glBegin(GL_TRIANGLES);
    // for (int triId = batch.triBegin; triId < batch.triBegin + drawElementsNum; triId++)
    // {
    //   const int v0 = a_input.indices[triId * 3 + 0];
    //   const int v1 = a_input.indices[triId * 3 + 1];
    //   const int v2 = a_input.indices[triId * 3 + 2];
    // 
    //   glTexCoord2f(a_input.texcoord2f[v0 * 2 + 0], a_input.texcoord2f[v0 * 2 + 1]);
    //   glNormal3f(a_input.norm4f[v0 * 4 + 0], a_input.norm4f[v0 * 4 + 1], a_input.norm4f[v0 * 4 + 2]);
    //   glVertex3f(a_input.pos4f[v0 * 4 + 0], a_input.pos4f[v0 * 4 + 1], a_input.pos4f[v0 * 4 + 2]);
    // 
    //   glTexCoord2f(a_input.texcoord2f[v1 * 2 + 0], a_input.texcoord2f[v1 * 2 + 1]);
    //   glNormal3f(a_input.norm4f[v1 * 4 + 0], a_input.norm4f[v1 * 4 + 1], a_input.norm4f[v1 * 4 + 2]);
    //   glVertex3f(a_input.pos4f[v1 * 4 + 0], a_input.pos4f[v1 * 4 + 1], a_input.pos4f[v1 * 4 + 2]);
    // 
    //   glTexCoord2f(a_input.texcoord2f[v2 * 2 + 0], a_input.texcoord2f[v2 * 2 + 1]);
    //   glNormal3f(a_input.norm4f[v2 * 4 + 0], a_input.norm4f[v2 * 4 + 1], a_input.norm4f[v2 * 4 + 2]);
    //   glVertex3f(a_input.pos4f[v2 * 4 + 0], a_input.pos4f[v2 * 4 + 1], a_input.pos4f[v2 * 4 + 2]);
    // }
    // glEnd();


  }

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  glEndList();


  return true;
}


void RD_OGL1_Plain::BeginScene(pugi::xml_node a_sceneNode)
{
  glViewport(0, 0, (GLint)m_width, (GLint)m_height);

  glShadeModel(GL_SMOOTH);							              // Enable Smooth Shading. 
  glClearColor(0.0f, 0.0f, 0.20f, 1.0f);				      // Dark dark blue background
  glClearDepth(1.0f);									                // Depth Buffer Setup
  glEnable(GL_DEPTH_TEST);							              // Enables Depth Testing
  glDepthFunc(GL_LEQUAL);								              // The Type Of Depth Testing To Do
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);


  if (camUseMatrices)
  {
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(camProjMatrixTransposed.L());

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(camWorldViewMartrixTransposed.L());
  }
  else
  {

    glMatrixMode(GL_PROJECTION);			// Select The Projection Matrix

    const float aspect = float(m_width) / float(m_height);
    float4x4 projMatrixInv = projectionMatrixTransposed(camFov, aspect, camNearPlane, camFarPlane);
    glLoadMatrixf(projMatrixInv.L());

    glMatrixMode(GL_MODELVIEW);			  // Select The Modelview Matrix

    camPos[2] -= 0.1;
    float3 eye(camPos[0], camPos[1], camPos[2]);
    float3 center(camLookAt[0], camLookAt[1], camLookAt[2]);
    float3 up(camUp[0], camUp[1], camUp[2]);

    float4x4 lookAtMatrix = lookAtTransposed(eye, center, up); // get inverse lookAt matrix

    glLoadMatrixf(lookAtMatrix.L());
  }
}

void RD_OGL1_Plain::EndScene()
{
  glFlush();
}

void RD_OGL1_Plain::Draw()
{
  // like glFinish();
}


void RD_OGL1_Plain::InstanceMeshes(int32_t a_mesh_id, const float* a_matrices, int32_t a_instNum, const int* a_lightInstId, const int* a_remapId, const int* a_realInstId)
{
  for (int32_t i = 0; i < a_instNum; i++)
  {
    float matrixT2[16];
    mat4x4_transpose(matrixT2, (float*)(a_matrices + i*16));

    glPushMatrix();    
    glMultMatrixf(matrixT2);
    glCallList(m_displayLists + GLuint(a_mesh_id));
    glPopMatrix();
  }
}


void RD_OGL1_Plain::InstanceLights(int32_t a_light_id, const float* a_matrix, pugi::xml_node* a_custAttrArray, int32_t a_instNum, int32_t a_lightGroupId)
{

}

HRRenderUpdateInfo RD_OGL1_Plain::HaveUpdateNow(int a_maxRaysPerPixel)
{
  //glFlush();
  HRRenderUpdateInfo res;
  res.finalUpdate   = true;
  res.haveUpdateFB  = true;
  res.progress      = 100.0f;
  return res;
}


void RD_OGL1_Plain::GetFrameBufferHDR(int32_t w, int32_t h, float*   a_out, const wchar_t* a_layerName)
{

}

void RD_OGL1_Plain::GetFrameBufferLDR(int32_t w, int32_t h, int32_t* a_out)
{
  glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)a_out);
}


IHRRenderDriver* CreateOpenGL1_RenderDriver()
{
  return new RD_OGL1_Plain;
}
