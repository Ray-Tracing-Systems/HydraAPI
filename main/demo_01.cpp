//
// Created by frol on 10/12/18.
//

#include <iostream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>

#include <iomanip> // for (std::fixed, std::setfill, std::setw)

#include "../hydra_api/HydraAPI.h"
#include "../hydra_api/HydraXMLVerify.h"

#include "mesh_utils.h"

//int hr_mkdir(const wchar_t* a_folder); // not a part of an API, just to make folder

void demo_01_plane_box()
{
  //hr_mkdir(L"demos");
  
  hrSceneLibraryOpen(L"demos/demo_01", HR_WRITE_DISCARD);
  
  // create materials for plane and cubes
  //
  auto mat0 = hrMaterialCreate(L"MyFirstMaterial");
  auto mat1 = hrMaterialCreate(L"MyFirstMaterial");
  
  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    auto node = hrMaterialParamNode(mat0);
    
    auto diff = node.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type") = L"lambert";
  
    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.75 0.5");
  
    VERIFY_XML(node);
  }
  hrMaterialClose(mat0);
  
  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    auto node = hrMaterialParamNode(mat1);
    
    auto diff = node.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type") = L"lambert";
    
    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.25 0.25 0.25");
  
    VERIFY_XML(node);
  }
  hrMaterialClose(mat1);
  
  // create plane and cube meshes. #NOTE: 'SimpleMesh' is just an example class, not the part of HydraAPI!
  //
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
  
  // create area light
  //
  HRLightRef rectLight = hrLightCreate(L"my_area_light");
  
  hrLightOpen(rectLight, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(rectLight);
    
    lightNode.attribute(L"type").set_value(L"area");
    lightNode.attribute(L"shape").set_value(L"rect");
    lightNode.attribute(L"distribution").set_value(L"diffuse"); // you can use both 'set_value' or '='
    
    auto sizeNode = lightNode.append_child(L"size");
    
    sizeNode.append_attribute(L"half_length") = 1.0f;
    sizeNode.append_attribute(L"half_width")  = 1.0f;
    
    auto intensityNode = lightNode.append_child(L"intensity");
    
    intensityNode.append_child(L"color").append_attribute(L"val")      = L"1 1 1";
    intensityNode.append_child(L"multiplier").append_attribute(L"val") = 8.0f;
    
    VERIFY_XML(lightNode);
  }
  hrLightClose(rectLight);
  
  
  // create and set up camera
  //
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
  
  
  // create and set up renderer object
  //
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
  hrRenderEnableDevice(renderRef, 0, true);
  
  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    auto node = hrRenderParamNode(renderRef);
    
    node.append_child(L"width").text()  = 512;
    node.append_child(L"height").text() = 512;
    
    node.append_child(L"method_primary").text()   = L"pathtracing";
    node.append_child(L"trace_depth").text()      = 6;
    node.append_child(L"diff_trace_depth").text() = 4;
    node.append_child(L"maxRaysPerPixel").text()  = 256;
    node.append_child(L"qmc_variant").text()      = (HYDRA_QMC_DOF_FLAG | HYDRA_QMC_MTL_FLAG | HYDRA_QMC_LGT_FLAG); // enable all of them, results to '7'
  }
  hrRenderClose(renderRef);
  
  
  // create and set up scene
  //
  HRSceneInstRef sceneRef = hrSceneCreate(L"myscene");
  const int NFrames = 10;
  for(int frame = 0; frame < NFrames; frame++)
  {
  
    // #NOTE: each frame we discard old scene and create the new one by instancing of existing geometry and lights
    //
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
  
    // now we launch our renderer
    //
    hrFlush(sceneRef, renderRef, camRef);
  
    std::cout << "rendering frame " << frame << " of " << NFrames << std::endl;
    std::cout.flush();
    
    // and wait while it finish
    //
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
    strOut << std::fixed  << L"demos/demo_01/zout_" << std::setfill(L"0"[0]) << std::setw(2) << frame << L".png";
    auto str = strOut.str();
    
    //hrRenderSaveFrameBufferLDR(renderRef, L"demos/demo_01/out.png");
    hrRenderSaveFrameBufferLDR(renderRef, str.c_str());
  
  }
  
  std::cout << "please look at the folder 'main/demos/demo01'" << std::endl;
}
