#include "tests.h"
#include <math.h>
#include <iomanip>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <math.h>
#include <sstream>
#include <fstream>

#include "../hydra_api/HR_HDRImage.h"
#include "../hydra_api/HR_HDRImageTool.h"

namespace HydraRender
{

  void MyCustomFilter(pugi::xml_node a_settings, const HDRImage4f& a_inImage,
                      HDRImage4f* a_outImage)
  {

  }

  void MyToneMapping(pugi::xml_node a_settings, const HDRImage4f& a_inImage,
                     std::vector<int>* a_outImage)
  {

  }

  void BloomExtractBrightPixels(pugi::xml_node a_settings, const HDRImage4f& a_inImage,
                                HDRImage4f* a_outImage)
  {

  }

  void BloomFinalPass(pugi::xml_node a_settings, const HDRImage4f& a_inImage, const HDRImage4f& a_inBrightPixels,
                      HDRImage4f* a_outImage)
  {

  }

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using HydraRender::HDRImage4f;

void image_p_sandbox()
{
  std::cout << "image_p main" << std::endl;

  pugi::xml_document settingsDoc;

  auto settings1 = settingsDoc.append_child(L"settings");
  settings1.force_attribute(L"radius").set_value(1);
  settings1.force_attribute(L"color_shift").set_value(L"0.1 0.0 0.05");


  // example of filter chain with pin-pong
  //
  HDRImage4f img1, img2;

  MyCustomFilter(settings1, img1, &img2); // 1 ==> 2
  MyCustomFilter(settings1, img2, &img1); // 1 <== 2
  MyCustomFilter(settings1, img1, &img2); // 1 ==> 2

  std::vector<int> resulLDR;
  MyToneMapping(settings1, img2,
                &resulLDR);                 // got final result to ldr buffer
                                            
                                            // more complex example, bloom
                                            //
  HDRImage4f brightPixels;

  BloomExtractBrightPixels(settings1, img1,
                           &brightPixels);

  BloomFinalPass(settings1, img1, brightPixels,
                 &img2);

  MyToneMapping(settings1, img2,
                &resulLDR);                // got final result to ldr buffer
}


