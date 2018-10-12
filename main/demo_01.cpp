//
// Created by frol on 10/12/18.
//

// #include <iostream>
// #include <vector>
// #include <chrono>
// #include <thread>
//

#include "../hydra_api/HydraAPI.h"
#include "../hydra_api/HydraXMLVerify.h"

// #include "../hydra_api/LiteMath.h"
//
// using namespace HydraLiteMath;
//
// #include "mesh_utils.h"
// #include "simplerandom.h"


void demo_01_plane_box()
{
  
  hrSceneLibraryOpen(L"demos/demo_01", HR_WRITE_DISCARD);
  
  auto mat0 = hrMaterialCreate(L"MyFirstMaterial");
  
  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    auto node = hrMaterialParamNode(mat0);
    
    auto diff = node.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type") = L"lambert";
  }
  hrMaterialClose(mat0);
  
  hrFlush();
  
  
  
  
  
}
