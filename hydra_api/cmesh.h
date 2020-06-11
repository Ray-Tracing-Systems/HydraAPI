//////////////////////////////////
// Created by frol on 23.11.19. //
//////////////////////////////////

#ifndef CMESH_GEOM_H
#define CMESH_GEOM_H

#include "aligned_alloc.h"
using cvex::aligned;

#include <vector>
#include <string>

#include <stdexcept>
#include <sstream>
#include <memory>
#include <cassert>

namespace cmesh
{
  // very simple utility mesh representation for working with geometry on the CPU in C++
  //
  struct SimpleMesh
  {
    enum SIMPLE_MESH_TOPOLOGY {SIMPLE_MESH_TRIANGLES = 0, SIMPLE_MESH_QUADS = 1};

    SimpleMesh() : topology(SIMPLE_MESH_TRIANGLES) {}
    SimpleMesh(int a_vertNum, int a_indNum) : topology(SIMPLE_MESH_TRIANGLES) { Resize(a_vertNum, a_indNum); }

    inline size_t VerticesNum()  const { return vPos4f.size()/4; }
    inline size_t IndicesNum()   const { return indices.size();  }
    inline size_t TrianglesNum() const { assert(topology == SIMPLE_MESH_TRIANGLES); return IndicesNum()/3;  }

    inline size_t QuadsNum()     const { assert(topology == SIMPLE_MESH_QUADS);     return IndicesNum()/4;  }
    inline int    PolySize()     const { return (topology == SIMPLE_MESH_QUADS) ? 4 : 3; }

    void Resize(int a_vertNum, int a_indNum);
    void Clear();
    void FreeMem();
    void Reserve(size_t vNum, size_t indNum);

    std::vector<float, aligned<float,16> > vPos4f;      // 
    std::vector<float, aligned<float,16> > vNorm4f;     // 
    std::vector<float, aligned<float,16> > vTang4f;     // 
    std::vector<float>                     vTexCoord2f; // 
    std::vector<uint32_t>                  indices;     // size = 3*TrianglesNum() for triangle mesh, 4*TrianglesNum() for quad mesh
    std::vector<uint32_t>                  matIndices;  // size = 1*TrianglesNum()
    
    SIMPLE_MESH_TOPOLOGY                   topology;
    
    struct CustArray
    {
      std::vector<int>   idata;
      std::vector<float> fdata;
      std::wstring       name;
      int depth;
      int apply;
    };

    std::vector<CustArray> customArrays;

  };

  SimpleMesh LoadMeshFromVSGF(const char* a_fileName);
  SimpleMesh CreateQuad(const int a_sizeX, const int a_sizeY, const float a_size);

  enum TANG_ALGORITHMS{ TANG_SIMPLE = 0, TANG_MIKEY = 1 };

  void ComputeNormals (cmesh::SimpleMesh& mesh, const int indexNum, bool useFaceNormals);
  void ComputeTangents(cmesh::SimpleMesh& mesh, int indexNum, TANG_ALGORITHMS a_alg = TANG_MIKEY);
  void WeldVertices   (cmesh::SimpleMesh& mesh, int indexNum);

  //struct MultiIndexMesh
  //{
  //  struct Triangle
  //  {
  //    int posInd[3];      // positions
  //    int normInd[3];     // both for normals and tangents ?
  //    int texCoordInd[3]; // tex coords
  //  };
  //
  //  std::vector<float> vPos4f;      // 
  //  std::vector<float> vNorm4f;     // 
  //  std::vector<float> vTang4f;     // 
  //  std::vector<float> vTexCoord2f; //    
  //};

};


#endif // CMESH_GEOM_H
