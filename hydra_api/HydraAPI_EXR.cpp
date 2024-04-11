#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

#include <cmath>
#include <cstring>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif


#ifdef TINYEXR_IMPL
#define TINYEXR_IMPLEMENTATION
#endif
#include "tinyexr.h"

void HrError(std::wstring a_str);

namespace hr_exr
{
  
  bool SaveImageToEXR(const float* data, int width, int height, int channels, const char* outfilename, float a_normConst = 1.0f, bool a_invertY = false)
  {
    EXRHeader header;
    InitEXRHeader(&header);

    EXRImage image;
    InitEXRImage(&image);
    image.num_channels = channels;

    std::vector <std::vector<float>> images(channels);
    for (auto& img : images)
    {
      img.resize(width* height);
    }
    
    
    if (a_invertY) 
    {
      for (int y = 0; y < height; y++) 
      {
        const int offsetY1 = y * width * channels;
        const int offsetY2 = (height - y - 1) * width * channels;
        for (int x = 0; x < width; x++) 
        {
          for (int c = 0; c < channels; ++c)
          {
            images[c][offsetY1 + x] = data[offsetY2 + x * channels + c] * a_normConst;
          }
        }
      }
    }
    else 
    {
      for (size_t i = 0; i < size_t(width * height); i++) 
      {
        for (int c = 0; c < channels; ++c)
        {
          images[c][i] = data[channels * i + c] * a_normConst;
        }
      }
    }

    std::vector<float*> image_ptrs(channels);
    {  
      int i = 0;
      for (auto& img_ptr : image_ptrs)
      {
        img_ptr = images[i].data();
        i++;
      }
    }

    image.images = (unsigned char**)image_ptrs.data();
    image.width = width;
    image.height = height;
    header.num_channels = channels;
    header.channels = (EXRChannelInfo*)malloc(sizeof(EXRChannelInfo) * header.num_channels);
    
    strncpy(header.channels[0].name, "Y", 255);
    header.channels[0].name[strlen("Y")] = '\0';
    for (int i = 1; i < channels; ++i)
    {
      std::string channel_name = std::to_string(i);
      strncpy(header.channels[i].name, channel_name.c_str(), 255);
      header.channels[i].name[strlen(channel_name.c_str())] = '\0';
    }

    header.pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
    header.requested_pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
    for (int i = 0; i < header.num_channels; i++) 
    {
      header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; 
      header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; 
    }

    const char* err = nullptr;
    int ret = SaveEXRImageToFile(&image, &header, outfilename, &err);
    if (ret != TINYEXR_SUCCESS) {
      fprintf(stderr, "Save EXR err: %s\n", err);
      FreeEXRErrorMessage(err); 
      return false;
    }

    free(header.channels);
    free(header.pixel_types);
    free(header.requested_pixel_types);

    return true;
  }


  bool SaveImagesToMultilayerEXR(const float** data, int width, int height, const char** outchannelnames, int n_images, const char* outfilename, bool a_invertY = false)
  {
    EXRHeader header;
    InitEXRHeader(&header);

    EXRImage image;
    InitEXRImage(&image);
    image.num_channels = n_images;
    image.images = (unsigned char**)data;
    
    std::vector<float*> image_ptrs(n_images);
    std::vector <std::vector<float>> images(n_images);
    if (a_invertY) // need to create a copy to invert pixels,
    {
      int i = 0;
      for (auto& img : images)
      {
        img.resize(width * height);
        //memcpy(img.data(), data[i], width * height * sizeof(float));
        for (int y = 0; y < height; y++)
        {
          const int offsetY1 = y * width;
          const int offsetY2 = (height - y - 1) * width;
          for (int x = 0; x < width; x++)
          {
            img[offsetY1 + x] = data[i][offsetY2 + x];
          }
        }
        image_ptrs[i] = img.data();
        i++;
      }
      
      image.images = (unsigned char**)image_ptrs.data();
    }

    image.width = width;
    image.height = height;
    header.num_channels = n_images;
    header.channels = (EXRChannelInfo*)malloc(sizeof(EXRChannelInfo) * header.num_channels);

    for (int i = 0; i < n_images; ++i)
    {
      if (strlen(outchannelnames[i]) >= 255)
      {
        std::wstringstream ws;
        ws << L"[SaveImagesToMulitlayerEXR] ::channel name too long" << strlen(outchannelnames[i]);
        HrError(ws.str());
      }
      strncpy(header.channels[i].name, outchannelnames[i], 255);
      header.channels[i].name[strlen(outchannelnames[i])] = '\0';
    }

    header.pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
    header.requested_pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
    for (int i = 0; i < header.num_channels; i++)
    {
      header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
      header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
    }

    const char* err = nullptr;
    int ret = SaveEXRImageToFile(&image, &header, outfilename, &err);
    if (ret != TINYEXR_SUCCESS) {
      fprintf(stderr, "Save EXR err: %s\n", err);
      FreeEXRErrorMessage(err);
      return false;
    }

    free(header.channels);
    free(header.pixel_types);
    free(header.requested_pixel_types);

    return true;
  }


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
}