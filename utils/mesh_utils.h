#pragma once

#include <vector>

struct SimpleMesh
{
  SimpleMesh()                                  = default;
  SimpleMesh(SimpleMesh&& a_in)                 = default;
  SimpleMesh(const SimpleMesh& a_in)            = default;
  SimpleMesh& operator=(SimpleMesh&& a_in)      = default;
  SimpleMesh& operator=(const SimpleMesh& a_in) = default;

  std::vector<float> vPos;
  std::vector<float> vNorm;
  std::vector<float> vTexCoord;
  std::vector<int>   triIndices;
  std::vector<int>   matIndices;
};


SimpleMesh CreatePlane(float a_size = 1.0f);
SimpleMesh CreateCube(float a_size = 1.0f);
SimpleMesh CreateCubeOpen(float a_size = 1.0f);
SimpleMesh CreateSphere(float radius, int numberSlices);
SimpleMesh CreateTorus(float innerRadius, float outerRadius, int numSides, int numFaces);
