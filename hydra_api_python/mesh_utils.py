import numpy as np


def createPlane(a_size):
  vertices = np.array([-a_size, 0.0, -a_size, 1.0,
                       +a_size, 0.0, -a_size, 1.0,
                       +a_size, 0.0, +a_size, 1.0,
                       -a_size, 0.0, +a_size, 1.0], dtype=np.float32)

  normals = np.array([0.0, 1.0, 0.0, 1.0,
                      0.0, 1.0, 0.0, 1.0,
                      0.0, 1.0, 0.0, 1.0,
                      0.0, 1.0, 0.0, 1.0,], dtype=np.float32)

  texCoords = np.array([0.0, 0.0,
                        1.0, 0.0,
                        1.0, 1.0,
                        0.0, 1.0], dtype=np.float32)

  triIndices = np.array([0, 1, 2,
                         0, 2, 3], dtype=np.int32)
                          
  #matIndices = np.array([0, 0], dtype=np.int32)           
             
  return (vertices, normals, texCoords, triIndices)       


def createCube(a_size):
  numberVertices = 24
  numberIndices  = 36

  vertices = np.array([-1.0, -1.0, -1.0, +1.0,
                       -1.0, -1.0, +1.0, +1.0,
                       +1.0, -1.0, +1.0, +1.0,
                       +1.0, -1.0, -1.0, +1.0,
                       -1.0, +1.0, -1.0, +1.0,
                       -1.0, +1.0, +1.0, +1.0,
                       +1.0, +1.0, +1.0, +1.0,
                       +1.0, +1.0, -1.0, +1.0,
                       -1.0, -1.0, -1.0, +1.0,
                       -1.0, +1.0, -1.0, +1.0,
                       +1.0, +1.0, -1.0, +1.0,
                       +1.0, -1.0, -1.0, +1.0,
                       -1.0, -1.0, +1.0, +1.0,
                       -1.0, +1.0, +1.0, +1.0,
                       +1.0, +1.0, +1.0, +1.0,
                       +1.0, -1.0, +1.0, +1.0,
                       -1.0, -1.0, -1.0, +1.0,
                       -1.0, -1.0, +1.0, +1.0,
                       -1.0, +1.0, +1.0, +1.0,
                       -1.0, +1.0, -1.0, +1.0,
                       +1.0, -1.0, -1.0, +1.0,
                       +1.0, -1.0, +1.0, +1.0,
                       +1.0, +1.0, +1.0, +1.0,
                       +1.0, +1.0, -1.0, +1.0], dtype=np.float32)

  normals = np.array([0.0, -1.0, 0.0, +1.0,
                      0.0, -1.0, 0.0, +1.0,
                      0.0, -1.0, 0.0, +1.0,
                      0.0, -1.0, 0.0, +1.0,
                      0.0, +1.0, 0.0, +1.0,
                      0.0, +1.0, 0.0, +1.0,
                      0.0, +1.0, 0.0, +1.0,
                      0.0, +1.0, 0.0, +1.0,
                      0.0, 0.0, -1.0, +1.0,
                      0.0, 0.0, -1.0, +1.0,
                      0.0, 0.0, -1.0, +1.0,
                      0.0, 0.0, -1.0, +1.0,
                      0.0, 0.0, +1.0, +1.0,
                      0.0, 0.0, +1.0, +1.0,
                      0.0, 0.0, +1.0, +1.0,
                      0.0, 0.0, +1.0, +1.0,
                      -1.0, 0.0, 0.0, +1.0,
                      -1.0, 0.0, 0.0, +1.0,
                      -1.0, 0.0, 0.0, +1.0,
                      -1.0, 0.0, 0.0, +1.0,
                      +1.0, 0.0, 0.0, +1.0,
                      +1.0, 0.0, 0.0, +1.0,
                      +1.0, 0.0, 0.0, +1.0,
                      +1.0, 0.0, 0.0, +1.0], dtype=np.float32)

  texCoords = np.array([0.0, 0.0,
                        0.0, 1.0,
                        1.0, 1.0,
                        1.0, 0.0,
                        1.0, 0.0,
                        1.0, 1.0,
                        0.0, 1.0,
                        0.0, 0.0,
                        0.0, 0.0,
                        0.0, 1.0,
                        1.0, 1.0,
                        1.0, 0.0,
                        0.0, 0.0,
                        0.0, 1.0,
                        1.0, 1.0,
                        1.0, 0.0,
                        0.0, 0.0,
                        0.0, 1.0,
                        1.0, 1.0,
                        1.0, 0.0,
                        0.0, 0.0,
                        0.0, 1.0,
                        1.0, 1.0,
                        1.0, 0.0], dtype=np.float32)

  triIndices =  np.array([0, 2, 1,
                          0, 3, 2,
                          4, 5, 6,
                          4, 6, 7,
                          8, 9, 10,
                          8, 10, 11,
                          12, 15, 14,
                          12, 14, 13,
                          16, 17, 18,
                          16, 18, 19,
                          20, 23, 22,
                          20, 22, 21], dtype=np.int32)
                           
  return (vertices * a_size, normals, texCoords, triIndices)                           
 
 
def createCubeOpen(a_size):
  vertices = np.array([-1.0, -1.0, -1.0, +1.0,
                       -1.0, -1.0, +1.0, +1.0,
                       +1.0, -1.0, +1.0, +1.0,
                       +1.0, -1.0, -1.0, +1.0,

                       -1.0, +1.0, -1.0, +1.0,
                       -1.0, +1.0, +1.0, +1.0,
                       +1.0, +1.0, +1.0, +1.0,
                       +1.0, +1.0, -1.0, +1.0,

                       -1.0, -1.0, -1.0, +1.0,
                       -1.0, +1.0, -1.0, +1.0,
                       +1.0, +1.0, -1.0, +1.0,
                       +1.0, -1.0, -1.0, +1.0,

                       -1.0, -1.0, +1.0, +1.0,
                       -1.0, +1.0, +1.0, +1.0,
                       +1.0, +1.0, +1.0, +1.0,
                       +1.0, -1.0, +1.0, +1.0,

                       -1.0, -1.0, -1.0, +1.0,
                       -1.0, -1.0, +1.0, +1.0,
                       -1.0, +1.0, +1.0, +1.0,
                       -1.0, +1.0, -1.0, +1.0,
                      
                       +1.0, -1.0, -1.0, +1.0,
                       +1.0, -1.0, +1.0, +1.0,
                       +1.0, +1.0, +1.0, +1.0,
                       +1.0, +1.0, -1.0, +1.0], dtype=np.float32)
   
  normals = np.array([0.0, -1.0, 0.0, 0.0, 
                      0.0, -1.0, 0.0, 0.0,
                      0.0, -1.0, 0.0, 0.0,
                      0.0, -1.0, 0.0, 0.0,
                                         
                      0.0, +1.0, 0.0, 0.0,
                      0.0, +1.0, 0.0, 0.0,
                      0.0, +1.0, 0.0, 0.0,
                      0.0, +1.0, 0.0, 0.0,
                     
                      0.0, 0.0, -1.0, 0.0,
                      0.0, 0.0, -1.0, 0.0,
                      0.0, 0.0, -1.0, 0.0,
                      0.0, 0.0, -1.0, 0.0,
                            
                      0.0, 0.0, +1.0, 0.0,
                      0.0, 0.0, +1.0, 0.0,
                      0.0, 0.0, +1.0, 0.0,
                      0.0, 0.0, +1.0, 0.0,
                              
                      -1.0, 0.0, 0.0, 0.0,
                      -1.0, 0.0, 0.0, 0.0,
                      -1.0, 0.0, 0.0, 0.0,
                      -1.0, 0.0, 0.0, 0.0,
                                     
                      +1.0, 0.0, 0.0, 0.0,
                      +1.0, 0.0, 0.0, 0.0,
                      +1.0, 0.0, 0.0, 0.0,
                      +1.0, 0.0, 0.0, 0.0], dtype=np.float32)
                            

  texCoords = np.array([0.0, 0.0,
                        0.0, 1.0,
                        1.0, 1.0,
                        1.0, 0.0,

                        1.0, 0.0,
                        1.0, 1.0,
                        0.0, 1.0,
                        0.0, 0.0,

                        0.0, 0.0,
                        0.0, 1.0,
                        1.0, 1.0,
                        1.0, 0.0,

                        0.0, 0.0,
                        0.0, 1.0,
                        1.0, 1.0,
                        1.0, 0.0,

                        0.0, 0.0,
                        0.0, 1.0,
                        1.0, 1.0,
                        1.0, 0.0,

                        0.0, 0.0,
                        0.0, 1.0,
                        1.0, 1.0,
                        1.0, 0.0], dtype=np.float32)

  triIndices = np.array([0, 2, 1,
                         0, 3, 2,

                         4, 5, 6,
                         4, 6, 7,

                         #8, 9, 10,
                         #8, 10, 11, 

                         12, 15, 14,
                         12, 14, 13,

                         16, 17, 18,
                         16, 18, 19,

                         20, 23, 22,
                         20, 22, 21], dtype=np.int32)
                          
  return (vertices * a_size, 1.0 * normals , texCoords, triIndices)  


import hydraPy as hy

def createCubeRef(name, a_size, a_matId):
    (cubeVertices, cubeNormals, cubeTexCoords, cubeIndices) = createCube(a_size)
    numberIndicesCube = cubeIndices.size

    cubeRef = hy.hrMeshCreate(name)
    hy.hrMeshOpen(cubeRef, hy.HR_TRIANGLE_IND3, hy.HR_WRITE_DISCARD)
    hy.hrMeshVertexAttribPointer4fNumPy(cubeRef, "pos", cubeVertices, 0)
    hy.hrMeshVertexAttribPointer4fNumPy(cubeRef, "norm", cubeNormals, 0)
    hy.hrMeshVertexAttribPointer2fNumPy(cubeRef, "texcoord", cubeTexCoords, 0)
    hy.hrMeshMaterialId(cubeRef, a_matId)
    hy.hrMeshAppendTriangles3NumPy(cubeRef, numberIndicesCube, cubeIndices)
    hy.hrMeshClose(cubeRef)
    
    return cubeRef
    
def createCubeOpenRef(name, a_size, a_matId):
    (cubeVertices, cubeNormals, cubeTexCoords, cubeIndices) = createCubeOpen(a_size)
    numberIndicesCube = cubeIndices.size

    cubeRef = hy.hrMeshCreate(name)
    hy.hrMeshOpen(cubeRef, hy.HR_TRIANGLE_IND3, hy.HR_WRITE_DISCARD)
    hy.hrMeshVertexAttribPointer4fNumPy(cubeRef, "pos", cubeVertices, 0)
    hy.hrMeshVertexAttribPointer4fNumPy(cubeRef, "norm", cubeNormals, 0)
    hy.hrMeshVertexAttribPointer2fNumPy(cubeRef, "texcoord", cubeTexCoords, 0)
    hy.hrMeshMaterialId(cubeRef, a_matId)
    hy.hrMeshAppendTriangles3NumPy(cubeRef, numberIndicesCube, cubeIndices)
    hy.hrMeshClose(cubeRef)
    
    return cubeRef
    
def createCornellCubeOpenRef(name, a_size, a_matIdR, a_matIdG, a_matIdW):
    (cubeVertices, cubeNormals, cubeTexCoords, cubeIndices) = createCubeOpen(a_size)
    numberIndicesCube = cubeIndices.size
    
    cubeMatIndices = np.array([a_matIdW, a_matIdW, a_matIdW, a_matIdW,
                               a_matIdW, a_matIdW, a_matIdG, a_matIdG,
                               a_matIdR, a_matIdR], dtype=np.int32)


    cubeRef = hy.hrMeshCreate(name)
    hy.hrMeshOpen(cubeRef, hy.HR_TRIANGLE_IND3, hy.HR_WRITE_DISCARD)
    hy.hrMeshVertexAttribPointer4fNumPy(cubeRef, "pos", cubeVertices, 0)
    hy.hrMeshVertexAttribPointer4fNumPy(cubeRef, "norm", cubeNormals, 0)
    hy.hrMeshVertexAttribPointer2fNumPy(cubeRef, "texcoord", cubeTexCoords, 0)
    hy.hrMeshPrimitiveAttribPointer1iNumPy(cubeRef, "mind", cubeMatIndices, 0)
    hy.hrMeshAppendTriangles3NumPy(cubeRef, numberIndicesCube, cubeIndices)
    hy.hrMeshClose(cubeRef)
    
    return cubeRef
    
def createPlaneRef(name, a_size, a_matId):
    (planeVertices, planeNormals, planeTexCoords, planeIndices) = createPlane(a_size)
    numberIndicesPlane = planeIndices.size

    planeRef = hy.hrMeshCreate(name)
    hy.hrMeshOpen(planeRef, hy.HR_TRIANGLE_IND3, hy.HR_WRITE_DISCARD)
    hy.hrMeshVertexAttribPointer4fNumPy(planeRef, "pos", planeVertices, 0)
    hy.hrMeshVertexAttribPointer4fNumPy(planeRef, "norm", planeNormals, 0)
    hy.hrMeshVertexAttribPointer2fNumPy(planeRef, "texcoord", planeTexCoords, 0)
    hy.hrMeshMaterialId(planeRef, a_matId)
    hy.hrMeshAppendTriangles3NumPy(planeRef, numberIndicesPlane, planeIndices)
    hy.hrMeshClose(planeRef)
    
    return planeRef
