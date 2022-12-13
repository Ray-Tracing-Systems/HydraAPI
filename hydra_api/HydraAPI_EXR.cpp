#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

bool SaveImage4fToEXR(const float* rgb, int width, int height, const char* outfilename, float a_normConst = 1.0f, bool a_invertY = false) 
{
  EXRHeader header;
  InitEXRHeader(&header);

  EXRImage image;
  InitEXRImage(&image);
  image.num_channels = 3;

  std::vector<float> images[3];
  images[0].resize(width * height);
  images[1].resize(width * height);
  images[2].resize(width * height);

  // Split RGBARGBARGBA... into R, G and B layer
  if(a_invertY) {
    for(int y=0;y<height;y++) {
      const int offsetY1 = y*width*4;
      const int offsetY2 = (height-y-1)*width*4;
      for(int x=0;x<width;x++) {
        images[0][(offsetY1 >> 2) + x] = rgb[offsetY2 + x*4 + 0]*a_normConst;
        images[1][(offsetY1 >> 2) + x] = rgb[offsetY2 + x*4 + 1]*a_normConst;
        images[2][(offsetY1 >> 2) + x] = rgb[offsetY2 + x*4 + 2]*a_normConst; 
      }
    }   
  }
  else {
    for (size_t i = 0; i < size_t(width * height); i++) {
      images[0][i] = rgb[4*i+0]*a_normConst;
      images[1][i] = rgb[4*i+1]*a_normConst;
      images[2][i] = rgb[4*i+2]*a_normConst;
    }
  }

  float* image_ptr[3];
  image_ptr[0] = images[2].data(); // B
  image_ptr[1] = images[1].data(); // G
  image_ptr[2] = images[0].data(); // R

  image.images = (unsigned char**)image_ptr;
  image.width  = width;
  image.height = height;
  header.num_channels = 3;
  header.channels     = (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels);
  // Must be (A)BGR order, since most of EXR viewers expect this channel order.
  strncpy(header.channels[0].name, "B", 255); header.channels[0].name[strlen("B")] = '\0';
  strncpy(header.channels[1].name, "G", 255); header.channels[1].name[strlen("G")] = '\0';
  strncpy(header.channels[2].name, "R", 255); header.channels[2].name[strlen("R")] = '\0';

  header.pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
  header.requested_pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
  for (int i = 0; i < header.num_channels; i++) {
    header.pixel_types[i]           = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
    header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of output image to be stored in .EXR
  }
 
  const char* err = nullptr; 
  int ret = SaveEXRImageToFile(&image, &header, outfilename, &err);
  if (ret != TINYEXR_SUCCESS) {
    fprintf(stderr, "Save EXR err: %s\n", err);
    FreeEXRErrorMessage(err); // free's buffer for an error message
    return false;
  }
  //printf("Saved exr file. [%s] \n", outfilename);

  free(header.channels);
  free(header.pixel_types);
  free(header.requested_pixel_types);

  return true;
}