//
// Created by frol on 10/12/18.
//

#include <iostream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>

#include "../hydra_api/HydraAPI.h"
#include "../hydra_api/HydraXMLVerify.h"

// #include "../hydra_api/LiteMath.h"
//
// using namespace HydraLiteMath;
//
#include "mesh_utils.h"
// #include "simplerandom.h"

void initGLIfNeeded();

void demo_01_plane_box()
{
  initGLIfNeeded();
  
  hrSceneLibraryOpen(L"demos/demo_01", HR_WRITE_DISCARD);
  
  auto mat0 = hrMaterialCreate(L"MyFirstMaterial");
  auto mat1 = hrMaterialCreate(L"MyFirstMaterial");
  
  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    auto node = hrMaterialParamNode(mat0);
    
    auto diff = node.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type") = L"lambert";
  
    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.75 0.5");
  }
  hrMaterialClose(mat0);
  
  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    auto node = hrMaterialParamNode(mat1);
    
    auto diff = node.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type") = L"lambert";
    
    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.25 0.25 0.25");
  }
  hrMaterialClose(mat1);
  
  SimpleMesh meshPlane = CreatePlane(10.0f);
  SimpleMesh meshCube  = CreateCube(1.0f);
  
  HRMeshRef cubeRef = hrMeshCreate(L"cube");
  
  hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeRef, L"pos",      &meshCube.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeRef, L"norm",     &meshCube.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", &meshCube.vTexCoord[0]);
    
    hrMeshMaterialId(cubeRef, mat0.id);
    
    hrMeshAppendTriangles3(cubeRef, int(meshCube.triIndices.size()), &meshCube.triIndices[0]);
  }
  hrMeshClose(cubeRef);
  
  HRMeshRef planeRef = hrMeshCreate(L"plane");
  
  hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(planeRef, L"pos",      &meshPlane.vPos[0]);
    hrMeshVertexAttribPointer4f(planeRef, L"norm",     &meshPlane.vNorm[0]);
    hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &meshPlane.vTexCoord[0]);
    
    hrMeshMaterialId(planeRef, mat1.id);
    
    hrMeshAppendTriangles3(planeRef, int(meshPlane.triIndices.size()), &meshPlane.triIndices[0]);
  }
  hrMeshClose(planeRef);
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  HRLightRef rectLight = hrLightCreate(L"my_area_light");
  
  hrLightOpen(rectLight, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(rectLight);
    
    lightNode.attribute(L"type").set_value(L"area");
    lightNode.attribute(L"shape").set_value(L"rect");
    lightNode.attribute(L"distribution").set_value(L"diffuse");
    
    auto sizeNode = lightNode.append_child(L"size");
    
    sizeNode.append_attribute(L"half_length").set_value(1.0f);
    sizeNode.append_attribute(L"half_width").set_value(1.0f);
    
    auto intensityNode = lightNode.append_child(L"intensity");
    
    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"8.0");
    
    VERIFY_XML(lightNode);
  }
  hrLightClose(rectLight);
  
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Camera
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  HRCameraRef camRef = hrCameraCreate(L"my camera");
  
  hrCameraOpen(camRef, HR_WRITE_DISCARD);
  {
    auto camNode = hrCameraParamNode(camRef);
    
    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");
    
    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 3 20");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
    
    VERIFY_XML(camNode);
  }
  hrCameraClose(camRef);
  
  
  //HRRenderRef renderRef = hrRenderCreate(L"opengl1");
  
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
  hrRenderEnableDevice(renderRef, 0, true);
  
  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    auto node = hrRenderParamNode(renderRef);
    
    node.append_child(L"width").text()  = 512;
    node.append_child(L"height").text() = 512;
    
    node.append_child(L"method_primary").text()   = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text()  = L"pathtracing";
    node.append_child(L"method_caustic").text()   = L"pathtracing";
    node.append_child(L"shadows").text()          = L"1";
    
    node.append_child(L"trace_depth").text()      = L"6";
    node.append_child(L"diff_trace_depth").text() = L"3";
    node.append_child(L"maxRaysPerPixel").text()  = 256;
  }
  hrRenderClose(renderRef);
  
  
  //hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  //{
  //  auto node = hrRenderParamNode(renderRef);
  //
  //  node.append_child(L"width").text() = 512;
  //  node.append_child(L"height").text() = 512;
  //}
  //hrRenderClose(renderRef);
  
  HRSceneInstRef sceneRef = hrSceneCreate(L"myscene");
  
  for(int frame = 0; frame < 10; frame++)
  {
    
    hrSceneOpen(sceneRef, HR_WRITE_DISCARD);
    {
    
      float m1[16] = {1, 0, 0, 0,
                      0, 1, 0, 0,
                      0, 0, 1, 0,
                      0, 0, 0, 1,};
    
      float m2[16] = {1, 0, 0, 0,
                      0, 1, 0, 1,
                      0, 0, 1, 0,
                      0, 0, 0, 1,};
    
      float m3[16] = {1, 0, 0, 0,
                      0, 1, 0, 5,
                      0, 0, 1, 0,
                      0, 0, 0, 1,};
    
      hrMeshInstance(sceneRef, planeRef, m1);
      hrMeshInstance(sceneRef, cubeRef, m2);
      hrLightInstance(sceneRef, rectLight, m3);
    
    
      for (int z = -2; z <= 2; z++)
      {
        for (int x = -2; x <= 2; x++)
        {
          float m4[16] = {1, 0, 0, float(x) * float(frame),
                          0, 1, 0, 1,
                          0, 0, 1, float(z) * float(frame),
                          0, 0, 0, 1,};
        
          hrMeshInstance(sceneRef, cubeRef, m4);
        }
      }
    
    }
    hrSceneClose(sceneRef);
  
  
    hrFlush(sceneRef, renderRef, camRef);
  
    std::cout << "rendering frame " << frame << std::endl;
    std::cout.flush();
    
    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
      HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);
    
      if (info.haveUpdateFB)
      {
        std::cout << "rendering progress = " << info.progress << "% \r";
        std::cout.flush();
      }
    
      if (info.finalUpdate)
        break;
    }
  
    std::wstringstream strOut;
    strOut << L"demos/demo_01/out_" << frame << L".png";
    auto str = strOut.str();
    
    //hrRenderSaveFrameBufferLDR(renderRef, L"demos/demo_01/out.png");
    hrRenderSaveFrameBufferLDR(renderRef, str.c_str());
  
  }
}
