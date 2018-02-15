#!/usr/bin python3.5
# -*- coding: utf-8 -*-


import hydraPy as hy
import numpy as np
import random

from mesh_utils import *
from light_math import *
from test_utils import *


DEG_TO_RAD = 0.01745329251


def render_scene(scenelib_path):

  hy.hrSceneLibraryOpen(scenelib_path, hy.HR_OPEN_EXISTING)
   
  renderRef = hy.HRRenderRef()
  renderRef.id = 0

  scnRef = hy.HRSceneInstRef()
  scnRef.id = 0
  
  camRef = hy.HRCameraRef()
  camRef.id = 0
  
  hy.hrRenderEnableDevice(renderRef, 0, True)
  #hy.hrFlush(scnRef, renderRef, camRef)
  hy.hrCommit(scnRef, renderRef, camRef)
  hy.hrRenderCommand(renderRef, "resume", "")
  
  initAndStartOpenGL(renderRef, 1024, 768, scenelib_path)

def test01_render_cubes(report_file, inBG):

  test_name = "test01_render_cubes"

  hy.hrSceneLibraryOpen("tests/" + test_name, hy.HR_WRITE_DISCARD)

  (cubeVertices, cubeNormals, cubeTexCoords, cubeIndices) = createCube("cube", 1.0)
  numberIndicesCube = cubeIndices.size
  
  (planeVertices, planeNormals, planeTexCoords, planeIndices) = createPlane("plane", 50.0)
  numberIndicesPlane = planeIndices.size
  
  testTex = hy.hrTexture2DCreateFromFile("../../main/data/textures/texture1.bmp")
  planeTex = hy.hrTexture2DCreateFromFile("../../main/data/textures/checker_16x16.bmp")

  mat0 = hy.hrMaterialCreate("mysimplemat")
  hy.hrMaterialOpen(mat0, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(mat0)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("0.0 0.75 0.0")
  hy.hrTextureBind(testTex, diff)
  hy.hrMaterialClose(mat0)
  
  mat1 = hy.hrMaterialCreate("plane")
  hy.hrMaterialOpen(mat1, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(mat1)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("1.0 1.0 1.0")
  hy.hrTextureBind(planeTex, diff)
  hy.hrMaterialClose(mat1)

  cubeRef = hy.hrMeshCreate("cube")
  hy.hrMeshOpen(cubeRef, hy.HR_TRIANGLE_IND3, hy.HR_WRITE_DISCARD)
  hy.hrMeshVertexAttribPointer4fNumPy(cubeRef, "pos", cubeVertices, 0)
  hy.hrMeshVertexAttribPointer4fNumPy(cubeRef, "norm", cubeNormals, 0)
  hy.hrMeshVertexAttribPointer2fNumPy(cubeRef, "texcoord", cubeTexCoords, 0)
  hy.hrMeshMaterialId(cubeRef, mat0.id)
  hy.hrMeshAppendTriangles3NumPy(cubeRef, numberIndicesCube, cubeIndices)
  hy.hrMeshClose(cubeRef)
  
  
  planeRef = hy.hrMeshCreate("plane")
  hy.hrMeshOpen(planeRef, hy.HR_TRIANGLE_IND3, hy.HR_WRITE_DISCARD)
  hy.hrMeshVertexAttribPointer4fNumPy(planeRef, "pos", planeVertices, 0)
  hy.hrMeshVertexAttribPointer4fNumPy(planeRef, "norm", planeNormals, 0)
  hy.hrMeshVertexAttribPointer2fNumPy(planeRef, "texcoord", planeTexCoords, 0)
  hy.hrMeshMaterialId(planeRef, mat1.id)
  hy.hrMeshAppendTriangles3NumPy(planeRef, numberIndicesPlane, planeIndices)
  hy.hrMeshClose(planeRef)


  camRef = hy.hrCameraCreate("my camera")
  hy.hrCameraOpen(camRef, hy.HR_WRITE_DISCARD)
  camNode = hy.hrCameraParamNode(camRef)
  camNode.append_child("fov").text().set("45")
  camNode.append_child("nearClipPlane").text().set("0.01")
  camNode.append_child("farClipPlane").text().set("100.0")
  camNode.append_child("up").text().set("0 1 0")
  camNode.append_child("position").text().set("0 0 5")
  camNode.append_child("look_at").text().set("0 0 -1")
  hy.hrCameraClose(camRef)

  light = hy.hrLightCreate("my_area_light")

  hy.hrLightOpen(light, hy.HR_WRITE_DISCARD);
  lightNode = hy.hrLightParamNode(light);
  lightNode.attribute("type").set_value("area");
  lightNode.attribute("shape").set_value("rect");
  lightNode.attribute("distribution").set_value("diffuse");

  sizeNode = lightNode.append_child("size")

  sizeNode.append_attribute("half_length").set_value(1.0);
  sizeNode.append_attribute("half_width").set_value(1.0);

  intensityNode = lightNode.append_child("intensity");

  intensityNode.append_child("color").text().set("1 1 1");
  intensityNode.append_child("multiplier").text().set("10.0");
  hy.hrLightClose(light);


  renderRef = hy.hrRenderCreate("HydraModern")
  hy.hrRenderEnableDevice(renderRef, 0, True);
  hy.hrRenderOpen(renderRef, hy.HR_WRITE_DISCARD)
  node = hy.hrRenderParamNode(renderRef)
  node.force_child("width").text().set(1024)
  node.force_child("height").text().set(768)
  node.force_child("method_primary").text().set("pathtracing")
  node.force_child("method_caustic").text().set("pathtracing")
  node.force_child("trace_depth").text().set(5)
  node.force_child("diff_trace_depth").text().set(3)
  node.force_child("maxRaysPerPixel").text().set(512)
  hy.hrRenderClose(renderRef)


  scnRef = hy.hrSceneCreate("my scene")

  matrixT  = np.dot(translateM4x4(np.array([-2.0, -1.0, -5.0])), scaleM4x4(np.array([1.0, 2.0, 1.0])))
  matrixT2 = translateM4x4(np.array([0.0, 2.0, 0.0]))  
  matrixT3 = np.dot(translateM4x4(np.array([2.0, -2.0, -5.0])), rotateYM4x4(45 * DEG_TO_RAD)) 
  #matrixT4 = np.dot(translateM4x4(np.array([0.0, -4.0, 0.0])), scaleM4x4(np.array([50.0, 1.0, 50.0])))
  matrixT4 = translateM4x4(np.array([0.0, -3.0, 0.0]))

  hy.hrSceneOpen(scnRef, hy.HR_WRITE_DISCARD)
  hy.hrMeshInstance(scnRef, cubeRef, matrixT.flatten())
  hy.hrMeshInstance(scnRef, cubeRef, matrixT3.flatten())
  hy.hrMeshInstance(scnRef, planeRef, matrixT4.flatten())
  hy.hrLightInstance(scnRef, light, matrixT2.flatten());
  hy.hrSceneClose(scnRef)
  hy.hrFlush(scnRef, renderRef, camRef)

  if(inBG):
    runRenderInBG(renderRef, 1024, 768, test_name)
    (res, mse) = check_images(test_name, 1, 20.0)
    if(res):
      report_file.write(test_name + " PASSED, MSE : {}\n".format(mse))
    else:
      report_file.write(test_name + " FAILED, MSE : {}\n".format(mse))   
  else:
    initAndStartOpenGL(renderRef, 1024, 768, test_name)
  
  return 
    
    
def test02_mesh_from_vsgf(report_file, inBG):

  test_name = "test02_mesh_form_vsgf"
  hy.hrSceneLibraryOpen("tests/" + test_name, hy.HR_WRITE_DISCARD)

  teapotRef = hy.hrMeshCreateFromFileDL("../../main/data/meshes/lucy.vsgf")
  
  mat0 = hy.hrMaterialCreate("mysimplemat")
  hy.hrMaterialOpen(mat0, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(mat0)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("1.0 1.0 1.0")
  testTex = hy.hrTexture2DCreateFromFile("checker_16x16.bmp")
  hy.hrTextureBind(testTex, diff)
  refl = matNode.append_child("reflectivity")
  refl.append_attribute("brdf_type").set_value("phong")
  refl.append_child("color").text().set("1.0 1.0 1.0")
  refl.append_child("glossiness").text().set("0.99")
  refl.append_child("fresnel_IOR").text().set("1.85");
  refl.append_child("fresnel").text().set("1");
  hy.hrTextureBind(testTex, refl)
  hy.hrMaterialClose(mat0)
  
  mat1 = hy.hrMaterialCreate("matBlue")
  hy.hrMaterialOpen(mat1, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(mat1)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("0.05 0.01 0.75")
  refl = matNode.append_child("reflectivity")
  refl.append_attribute("brdf_type").set_value("phong")
  refl.append_child("color").text().set("0.1 0.1 0.1")
  refl.append_child("glossiness").text().set("1.0")
  refl.append_child("fresnel_IOR").text().set("1.3");
  refl.append_child("fresnel").text().set("0");
  hy.hrMaterialClose(mat1)
  
  planeRef = createPlaneRef("plane", 4.0, mat0.id)

  camRef = hy.hrCameraCreate("my camera")
  hy.hrCameraOpen(camRef, hy.HR_WRITE_DISCARD)
  camNode = hy.hrCameraParamNode(camRef)
  camNode.append_child("fov").text().set("45")
  camNode.append_child("nearClipPlane").text().set("0.01")
  camNode.append_child("farClipPlane").text().set("100.0")
  camNode.append_child("up").text().set("0 1 0")
  camNode.append_child("position").text().set("0 0 5")
  camNode.append_child("look_at").text().set("0 0 -1")
  hy.hrCameraClose(camRef)

  light = hy.hrLightCreate("my_area_light")

  hy.hrLightOpen(light, hy.HR_WRITE_DISCARD);
  lightNode = hy.hrLightParamNode(light);
  lightNode.attribute("type").set_value("area");
  lightNode.attribute("shape").set_value("rect");
  lightNode.attribute("distribution").set_value("diffuse");

  sizeNode = lightNode.append_child("size")

  sizeNode.append_attribute("half_length").set_value(1.0);
  sizeNode.append_attribute("half_width").set_value(1.0);

  intensityNode = lightNode.append_child("intensity");

  intensityNode.append_child("color").text().set("1 1 1");
  intensityNode.append_child("multiplier").text().set("10.0");
  hy.hrLightClose(light);


  renderRef = hy.hrRenderCreate("HydraModern")
  hy.hrRenderEnableDevice(renderRef, 0, True);
  hy.hrRenderOpen(renderRef, hy.HR_WRITE_DISCARD)
  node = hy.hrRenderParamNode(renderRef)
  node.force_child("width").text().set(1024)
  node.force_child("height").text().set(768)
  node.force_child("method_primary").text().set("pathtracing")
  node.force_child("method_caustic").text().set("pathtracing")
  node.force_child("trace_depth").text().set(5)
  node.force_child("diff_trace_depth").text().set(3)
  node.force_child("maxRaysPerPixel").text().set(256)
  hy.hrRenderClose(renderRef)


  scnRef = hy.hrSceneCreate("my scene")
  

  matrixT_1 = np.dot(translateM4x4(np.array([0.0, -1.0, 0.0])), scaleM4x4(np.array([0.33, 0.33, 0.33])))
  matrixT_2 = np.dot(translateM4x4(np.array([-1.0, -1.0, 0.0])), scaleM4x4(np.array([0.33, 0.33, 0.33])))
  matrixT_3 = np.dot(translateM4x4(np.array([1.0, -1.0, 0.0])), scaleM4x4(np.array([0.33, 0.33, 0.33])))
  matrixT_4 = np.dot(translateM4x4(np.array([2.0, -1.0, 0.0])), scaleM4x4(np.array([0.33, 0.33, 0.33])))
  matrixT_5 = np.dot(translateM4x4(np.array([-2.0, -1.0, 0.0])), scaleM4x4(np.array([0.33, 0.33, 0.33])))
    
  matrixT_plane = translateM4x4(np.array([0.0, -1.0, 0.0]))
  matrixT_light = translateM4x4(np.array([0.0, 2.0, 0.0]))


  hy.hrSceneOpen(scnRef, hy.HR_WRITE_DISCARD)
  hy.hrMeshInstance(scnRef, teapotRef, matrixT_1.flatten())
  hy.hrMeshInstance(scnRef, teapotRef, matrixT_2.flatten())
  hy.hrMeshInstance(scnRef, teapotRef, matrixT_3.flatten())
  hy.hrMeshInstance(scnRef, teapotRef, matrixT_4.flatten())
  hy.hrMeshInstance(scnRef, teapotRef, matrixT_5.flatten())
  hy.hrMeshInstance(scnRef, planeRef, matrixT_plane.flatten())
  hy.hrLightInstance(scnRef, light, matrixT_light.flatten());
  hy.hrSceneClose(scnRef)
  hy.hrFlush(scnRef, renderRef, camRef)
  

  if(inBG):
    runRenderInBG(renderRef, 1024, 768, test_name)
    (res, mse) = check_images(test_name, 1, 30.0)
    if(res):
      report_file.write(test_name + " PASSED, MSE : {}\n".format(mse))
    else:
      report_file.write(test_name + " FAILED, MSE : {}\n".format(mse))   
  else:
    initAndStartOpenGL(renderRef, 1024, 768, test_name)
    

  return 

def test03_cornell_box(report_file, inBG):

  test_name = "test03_cornell_box"
  hy.hrSceneLibraryOpen("tests/" + test_name, hy.HR_WRITE_DISCARD)
  
  matW = hy.hrMaterialCreate("mWhite")
  hy.hrMaterialOpen(matW, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matW)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("0.5 0.5 0.5")
  hy.hrMaterialClose(matW)
  
  matTeapot = hy.hrMaterialCreate("teapot")
  hy.hrMaterialOpen(matTeapot, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matTeapot)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("0.207843 0.188235 0.0")
  refl = matNode.append_child("reflectivity")
  refl.append_attribute("brdf_type").set_value("phong")
  refl.append_child("color").text().set("0.367059 0.345882 0")
  refl.append_child("glossiness").text().set("0.5")
  refl.append_child("fresnel_IOR").text().set("14");
  refl.append_child("fresnel").text().set("1");
  hy.hrMaterialClose(matTeapot)
  
  matR = hy.hrMaterialCreate("mRed")
  hy.hrMaterialOpen(matR, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matR)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("0.5 0.0 0.0")
  hy.hrMaterialClose(matR)
 
  matG = hy.hrMaterialCreate("mGreen")
  hy.hrMaterialOpen(matG, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matG)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("0.0 0.5 0.0")
  hy.hrMaterialClose(matG)


  cubeOpenRef = createCornellCubeOpenRef("cornellBox", 4.0, matR.id, matG.id, matW.id);
  teapotRef = hy.hrMeshCreateFromFileDL("../../main/data/meshes/teapot.vsgf")

  camRef = hy.hrCameraCreate("my camera")
  hy.hrCameraOpen(camRef, hy.HR_WRITE_DISCARD)
  camNode = hy.hrCameraParamNode(camRef)
  camNode.append_child("fov").text().set("45")
  camNode.append_child("nearClipPlane").text().set("0.01")
  camNode.append_child("farClipPlane").text().set("100.0")
  camNode.append_child("up").text().set("0 1 0")
  camNode.append_child("position").text().set("0 0 15")
  camNode.append_child("look_at").text().set("0 0 0")
  hy.hrCameraClose(camRef)


  light = hy.hrLightCreate("my_area_light")

  hy.hrLightOpen(light, hy.HR_WRITE_DISCARD);
  lightNode = hy.hrLightParamNode(light);
  lightNode.attribute("type").set_value("area");
  lightNode.attribute("shape").set_value("rect");
  lightNode.attribute("distribution").set_value("diffuse");

  sizeNode = lightNode.append_child("size")
  sizeNode.append_attribute("half_length").set_value(1.0);
  sizeNode.append_attribute("half_width").set_value(1.0);

  intensityNode = lightNode.append_child("intensity");
  intensityNode.append_child("color").text().set("1 1 1");
  intensityNode.append_child("multiplier").text().set("10.0");
  hy.hrLightClose(light);

  renderRef = hy.hrRenderCreate("HydraModern")
  hy.hrRenderEnableDevice(renderRef, 0, True);
  hy.hrRenderOpen(renderRef, hy.HR_WRITE_DISCARD)
  node = hy.hrRenderParamNode(renderRef)
  node.force_child("width").text().set(1024)
  node.force_child("height").text().set(768)
  node.force_child("method_primary").text().set("IBPT")
  node.force_child("method_caustic").text().set("IBPT")
  node.force_child("trace_depth").text().set(5)
  node.force_child("diff_trace_depth").text().set(3)
  node.force_child("maxRaysPerPixel").text().set(512)
  hy.hrRenderClose(renderRef)


  scnRef = hy.hrSceneCreate("my scene")

  matrixT_1 = np.dot(translateM4x4(np.array([0.0, -2.555, 0.0])), scaleM4x4(np.array([3.65, 3.65, 3.65])))
  matrixT_2 = rotateYM4x4(DEG_TO_RAD * 180.0)
  matrixT_light = translateM4x4(np.array([0.0, 3.85, 0.0]))
  mI = identityM4x4()

  hy.hrSceneOpen(scnRef, hy.HR_WRITE_DISCARD)
  hy.hrMeshInstance(scnRef, teapotRef, matrixT_1.flatten())
  hy.hrMeshInstance(scnRef, cubeOpenRef, matrixT_2.flatten())
  hy.hrLightInstance(scnRef, light, matrixT_light.flatten());
  hy.hrSceneClose(scnRef)
  hy.hrFlush(scnRef, renderRef, camRef)
  

  if(inBG):
    runRenderInBG(renderRef, 1024, 768, test_name)
    (res, mse) = check_images(test_name, 1, 30.0)
    if(res):
      report_file.write(test_name + " PASSED, MSE : {}\n".format(mse))
    else:
      report_file.write(test_name + " FAILED, MSE : {}\n".format(mse))   
  else:
    initAndStartOpenGL(renderRef, 1024, 768, test_name)
  return

def test04_instancing(report_file, inBG):

  test_name = "test04_instancing"
  hy.hrSceneLibraryOpen("tests/" + test_name, hy.HR_WRITE_DISCARD)
  
  texPattern = hy.hrTexture2DCreateFromFile("../../main/data/meshes/bigleaf3.tga")
  
  matGray = hy.hrMaterialCreate("matGray");
  matTrunk = hy.hrMaterialCreate("Trunk");
  matWigglers = hy.hrMaterialCreate("Wigglers");
  matBranches = hy.hrMaterialCreate("Branches");
  matPllarRts = hy.hrMaterialCreate("PillarRoots");
  matLeaves = hy.hrMaterialCreate("Leaves");
  matCanopy = hy.hrMaterialCreate("Canopy");
  matCube = hy.hrMaterialCreate("CubeMap");
  
  hy.hrMaterialOpen(matCube, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matCube)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("0.5 0.5 0.5")
  hy.hrMaterialClose(matCube)
  
  hy.hrMaterialOpen(matGray, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matGray)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("0.5 0.5 0.5")
  hy.hrMaterialClose(matGray)
  
  hy.hrMaterialOpen(matTrunk, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matTrunk)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("0.345098 0.215686 0.0117647")
  hy.hrMaterialClose(matTrunk)
  
  hy.hrMaterialOpen(matWigglers, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matWigglers)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("0.345098 0.215686 0.0117647")
  hy.hrMaterialClose(matWigglers)
  
  hy.hrMaterialOpen(matBranches, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matBranches)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("0.345098 0.215686 0.0117647")
  hy.hrMaterialClose(matBranches)
  
  hy.hrMaterialOpen(matPllarRts, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matPllarRts)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("0.345098 0.215686 0.0117647")
  hy.hrMaterialClose(matPllarRts)
  
  hy.hrMaterialOpen(matLeaves, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matLeaves)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("0.0533333 0.208627 0.00627451")
  opacity = matNode.append_child("opacity")
  opacity.append_child("skip_shadow").append_attribute("val").set_value(0)
  hy.hrTextureBind(texPattern, opacity)
  hy.hrMaterialClose(matLeaves)
  
  hy.hrMaterialOpen(matCanopy, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matCanopy)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("0.0941176 0.4 0.0")
  hy.hrMaterialClose(matCanopy)
  

  cubeR = createCubeRef("cube", 2.0, matCube.id);
  planeRef = createPlaneRef("plane", 10000.0, matGray.id);
  treeRef = hy.hrMeshCreateFromFileDL("../../main/data/meshes/bigtree.vsgf")
  
  sky = hy.hrLightCreate("sky")
  sun = hy.hrLightCreate("sun")

  hy.hrLightOpen(sky, hy.HR_WRITE_DISCARD);
  lightNode = hy.hrLightParamNode(sky);
  lightNode.attribute("type").set_value("sky");
  lightNode.attribute("distribution").set_value("perez");

  intensityNode = lightNode.append_child("intensity");
  intensityNode.append_child("color").text().set("1 1 1");
  intensityNode.append_child("multiplier").text().set("1.0");
  
  sunModel = lightNode.append_child("perez")
  sunModel.append_attribute("sun_id").set_value(sun.id)
  sunModel.append_attribute("turbidity").set_value(2.0)
  
  hy.hrLightClose(sky)
  
  
  hy.hrLightOpen(sun, hy.HR_WRITE_DISCARD);
  lightNode = hy.hrLightParamNode(sun);
  lightNode.attribute("type").set_value("directional")
  lightNode.attribute("shape").set_value("point")
  lightNode.attribute("distribution").set_value("directional")
  
  sizeNode = lightNode.append_child("size")
  sizeNode.append_attribute("inner_radius").set_value(0.0);
  sizeNode.append_attribute("outer_radius").set_value(10000.0);

  intensityNode = lightNode.append_child("intensity");

  intensityNode.append_child("color").text().set("1.0 0.85 0.64");
  intensityNode.append_child("multiplier").text().set("2.0");
  
  lightNode.append_child("shadow_softness").append_attribute("val").set_value(1.0)
  hy.hrLightClose(sky);
  

  camRef = hy.hrCameraCreate("my camera")
  hy.hrCameraOpen(camRef, hy.HR_WRITE_DISCARD)
  camNode = hy.hrCameraParamNode(camRef)
  camNode.append_child("fov").text().set("45")
  camNode.append_child("nearClipPlane").text().set("0.01")
  camNode.append_child("farClipPlane").text().set("100.0")
  camNode.append_child("up").text().set("0 1 0")
  camNode.append_child("position").text().set("0 25 75")
  camNode.append_child("look_at").text().set("10 15 0")
  hy.hrCameraClose(camRef)


  renderRef = hy.hrRenderCreate("HydraModern")
  hy.hrRenderEnableDevice(renderRef, 0, True);
  hy.hrRenderOpen(renderRef, hy.HR_WRITE_DISCARD)
  node = hy.hrRenderParamNode(renderRef)
  node.force_child("width").text().set(1024)
  node.force_child("height").text().set(768)
  node.force_child("method_primary").text().set("pathtracing")
  node.force_child("method_caustic").text().set("pathtracing")
  node.force_child("trace_depth").text().set(5)
  node.force_child("diff_trace_depth").text().set(3)
  node.force_child("maxRaysPerPixel").text().set(128)
  hy.hrRenderClose(renderRef)


  scnRef = hy.hrSceneCreate("my scene")
  
  hy.hrSceneOpen(scnRef, hy.HR_WRITE_DISCARD)

  matrixT_1 = translateM4x4(np.array([0.0, -1.0, 0.0]))
  hy.hrMeshInstance(scnRef, planeRef, matrixT_1.flatten())
  
  mTranslate = translateM4x4(np.array([-4.75, 1.0, 5.0]))
  mRot = rotateYM4x4(60.0 * DEG_TO_RAD)
  mRes = np.dot(mTranslate, mRot)
  
  hy.hrMeshInstance(scnRef, cubeR, mRes.flatten())
  
  random.seed(1008)
  
  dist1 = 40.0
  SQUARESIZE1 = 100
  
  for i in range(-SQUARESIZE1, SQUARESIZE1):
    for j in range(-SQUARESIZE1, SQUARESIZE1):
      (x_offset, y_offset) = random.uniform(-1.0, 1.0), random.uniform(-1.0, 1.0)
      pos = dist1 * np.array([float(i), 0.0, float(j)]) + dist1 * np.array([x_offset, 0.0, y_offset])
      mTranslate = translateM4x4(np.array([pos[0], 1.0, pos[2]]));
      mRot = rotateYM4x4(random.uniform(-180.0*DEG_TO_RAD, +180.0*DEG_TO_RAD));
      mRes = np.dot(mTranslate, mRot);

      hy.hrMeshInstance(scnRef, cubeR, mRes.flatten())

  for i in range(-SQUARESIZE1, SQUARESIZE1):
    for j in range(-SQUARESIZE1, SQUARESIZE1):
      (x_offset, y_offset) = random.uniform(-1.0, 1.0), random.uniform(-1.0, 1.0)
      pos = dist1 * np.array([float(i), 0.0, float(j)]) + dist1 * np.array([x_offset, 0.0, y_offset])
      mTranslate = translateM4x4(np.array([pos[0], 1.0, pos[2]]));
      mScale = scaleM4x4(np.array([5.0, 5.0, 5.0]))
      mRot = rotateYM4x4(random.uniform(-180.0*DEG_TO_RAD, +180.0*DEG_TO_RAD));
      mRes = np.dot(mTranslate, np.dot(mRot, mScale))

      if(random.uniform(0.0, 1.0) > 0.8):
        hy.hrMeshInstance(scnRef, treeRef, mRes.flatten())

  mRes = identityM4x4()
  hy.hrLightInstance(scnRef, sky, mRes.flatten())
  
  mTranslate = translateM4x4(np.array([200.0, 200.0, -100.0]));
  mRot  = rotateXM4x4(10.0*DEG_TO_RAD)
  mRot2 = rotateZM4x4(30.*DEG_TO_RAD)
  mRes  = np.dot(mRot2, mRot)
  mRes  = np.dot(mTranslate, mRes)
  
  hy.hrLightInstance(scnRef, sun, mRes.flatten());
  hy.hrSceneClose(scnRef)
  hy.hrFlush(scnRef, renderRef, camRef)
  

  if(inBG):
    runRenderInBG(renderRef, 1024, 768, test_name)
    (res, mse) = check_images(test_name, 1, 50.0)
    if(res):
      report_file.write(test_name + " PASSED, MSE : {}\n".format(mse))
    else:
      report_file.write(test_name + " FAILED, MSE : {}\n".format(mse))   
  else:
    initAndStartOpenGL(renderRef, 1024, 768, test_name)
    

  return

def test05_load_existing(report_file, inBG):
  test_name = "test05_load_existing"

  hy.hrSceneLibraryOpen("tests/" + test_name, hy.HR_OPEN_EXISTING)
  renderRef = hy.HRRenderRef()
  renderRef.id = 0

  scnRef = hy.HRSceneInstRef()
  scnRef.id = 0
  
  camRef = hy.HRCameraRef()
  camRef.id = 0
  
  
  hy.hrRenderEnableDevice(renderRef, 0, True)
  #hy.hrFlush(scnRef, renderRef, camRef)
  hy.hrCommit(scnRef, renderRef, camRef)
  hy.hrRenderCommand(renderRef, "resume", "")
  
  if(inBG):
    runRenderInBG(renderRef, 1024, 768, test_name)
    (res, mse) = check_images(test_name, 1, 50.0)
    if(res):
      report_file.write(test_name + " PASSED, MSE : {}\n".format(mse))
    else:
      report_file.write(test_name + " FAILED, MSE : {}\n".format(mse))   
  else:
    initAndStartOpenGL(renderRef, 1024, 768, test_name)

def test06_blend_simple(report_file, inBG):

  test_name = "test06_blend_simple"
  hy.hrSceneLibraryOpen("tests/" + test_name, hy.HR_WRITE_DISCARD)
  
  matGold    = hy.hrMaterialCreate("matGold")
  matSilver  = hy.hrMaterialCreate("matSilver")
  matLacquer = hy.hrMaterialCreate("matLacquer")
  matGlass   = hy.hrMaterialCreate("matGlass")
  matBricks1 = hy.hrMaterialCreate("matBricks1")
  matBricks2 = hy.hrMaterialCreate("matBricks2")
  matGray = hy.hrMaterialCreate("matGray")
  
  texChecker = hy.hrTexture2DCreateFromFile("../../main/data/textures/chess_white.bmp")
  texYinYang = hy.hrTexture2DCreateFromFile("../../main/data/textures/yinyang.png")
  
  hy.hrMaterialOpen(matGold, hy.HR_WRITE_DISCARD);
  matNode = hy.hrMaterialParamNode(matGold);
  diff = matNode.append_child("diffuse");
  diff.append_attribute("brdf_type").set_value("lambert");
  diff.append_child("color").append_attribute("val").set_value("0.88 0.61 0.05");
  refl = matNode.append_child("reflectivity");
  refl.append_attribute("brdf_type").set_value("torranse_sparrow");
  refl.append_child("color").append_attribute("val").set_value("0.88 0.61 0.05");
  refl.append_child("glossiness").append_attribute("val").set_value("0.98");
  refl.append_child("extrusion").append_attribute("val").set_value("maxcolor");
  refl.append_child("fresnel").append_attribute("val").set_value(1);
  refl.append_child("fresnel_IOR").append_attribute("val").set_value(8.0);
  hy.hrMaterialClose(matGold);
  
  hy.hrMaterialOpen(matSilver, hy.HR_WRITE_DISCARD);
  matNode = hy.hrMaterialParamNode(matSilver);
  diff = matNode.append_child("diffuse");
  diff.append_attribute("brdf_type").set_value("lambert");
  diff.append_child("color").append_attribute("val").set_value("0.8 0.8 0.8");
  refl = matNode.append_child("reflectivity");
  refl.append_attribute("brdf_type").set_value("torranse_sparrow");
  refl.append_child("color").append_attribute("val").set_value("0.8 0.8 0.8");
  refl.append_child("glossiness").append_attribute("val").set_value("0.98");
  refl.append_child("extrusion").append_attribute("val").set_value("maxcolor");
  refl.append_child("fresnel").append_attribute("val").set_value(1);
  refl.append_child("fresnel_IOR").append_attribute("val").set_value(8.0);
  hy.hrMaterialClose(matSilver);
  
  hy.hrMaterialOpen(matLacquer, hy.HR_WRITE_DISCARD);
  matNode = hy.hrMaterialParamNode(matLacquer);
  diff = matNode.append_child("diffuse");
  diff.append_attribute("brdf_type").set_value("lambert");
  diff.append_child("color").append_attribute("val").set_value("0.05 0.05 0.05");
  refl = matNode.append_child("reflectivity");
  refl.append_attribute("brdf_type").set_value("phong");
  refl.append_child("color").append_attribute("val").set_value("0.5 0.5 0.5");
  refl.append_child("glossiness").append_attribute("val").set_value("1.0");
  refl.append_child("extrusion").append_attribute("val").set_value("maxcolor");
  refl.append_child("fresnel").append_attribute("val").set_value(1);
  refl.append_child("fresnel_IOR").append_attribute("val").set_value(1.5);
  hy.hrMaterialClose(matLacquer);
  
  hy.hrMaterialOpen(matGlass, hy.HR_WRITE_DISCARD);
  matNode = hy.hrMaterialParamNode(matGlass);
  refl = matNode.append_child("reflectivity");
  refl.append_attribute("brdf_type").set_value("phong");
  refl.append_child("color").append_attribute("val").set_value("0.1 0.1 0.1 ");
  refl.append_child("glossiness").append_attribute("val").set_value(1.0);
  refl.append_child("extrusion").append_attribute("val").set_value("maxcolor");
  refl.append_child("fresnel").append_attribute("val").set_value(1);
  refl.append_child("fresnel_IOR").append_attribute("val").set_value(1.5);
  transp = matNode.append_child("transparency");
  transp.append_attribute("brdf_type").set_value("phong");
  transp.append_child("color").append_attribute("val").set_value("1.0 1.0 1.0");
  transp.append_child("glossiness").append_attribute("val").set_value(1.0);
  transp.append_child("thin_walled").append_attribute("val").set_value(0);
  transp.append_child("fog_color").append_attribute("val").set_value("0.9 0.9 1.0");
  transp.append_child("fog_multiplier").append_attribute("val").set_value(1.0);
  transp.append_child("IOR").append_attribute("val").set_value(1.5);
  hy.hrMaterialClose(matGlass);
  
  hy.hrMaterialOpen(matBricks1, hy.HR_WRITE_DISCARD);
  matNode = hy.hrMaterialParamNode(matBricks1);
  diff = matNode.append_child("diffuse");
  diff.append_attribute("brdf_type").set_value("lambert");
  color = diff.append_child("color");
  color.append_attribute("val").set_value("0.2 0.2 0.75");
  color.append_attribute("tex_apply_mode ").set_value("multiply");
  texNode = hy.hrTextureBind(texChecker, diff.child("color"));
  texNode.append_attribute("matrix");
  samplerMatrix = np.array([ 16,  0, 0, 0,
                              0, 16, 0, 0,
                              0,  0, 1, 0,
                              0,  0, 0, 1], dtype = np.float32)
  texNode.append_attribute("addressing_mode_u").set_value("wrap");
  texNode.append_attribute("addressing_mode_v").set_value("wrap");
  texNode.append_attribute("input_gamma").set_value(2.2);
  texNode.append_attribute("input_alpha").set_value("rgb");
  hy.WriteMatrix4x4(texNode, "matrix", samplerMatrix);
  hy.hrMaterialClose(matBricks1)
  
  
  hy.hrMaterialOpen(matBricks2, hy.HR_WRITE_DISCARD);
  matNode = hy.hrMaterialParamNode(matBricks2);
  diff = matNode.append_child("diffuse");
  diff.append_attribute("brdf_type").set_value("lambert");
  color = diff.append_child("color");
  color.append_attribute("val").set_value("0.1 0.1 0.1");
  color.append_attribute("tex_apply_mode ").set_value("multiply");
  texNode = hy.hrTextureBind(texChecker, diff.child("color"));
  texNode.append_attribute("matrix");
  samplerMatrix2 = np.array([ 8,  0, 0, 0,
                              0,  8, 0, 0,
                              0,  0, 1, 0,
                              0,  0, 0, 1], dtype = np.float32)
  texNode.append_attribute("addressing_mode_u").set_value("wrap");
  texNode.append_attribute("addressing_mode_v").set_value("wrap");
  texNode.append_attribute("input_gamma").set_value(2.2);
  texNode.append_attribute("input_alpha").set_value("rgb");
  hy.WriteMatrix4x4(texNode, "matrix", samplerMatrix2);
  refl = matNode.append_child("reflectivity");
  refl.append_attribute("brdf_type").set_value("phong");
  refl.append_child("color").append_attribute("val").set_value("0.9 0.9 0.9");
  refl.append_child("glossiness").append_attribute("val").set_value("1.0");
  refl.append_child("extrusion").append_attribute("val").set_value("maxcolor");
  refl.append_child("fresnel").append_attribute("val").set_value(1);
  refl.append_child("fresnel_IOR").append_attribute("val").set_value(1.5);
  hy.hrMaterialClose(matBricks2);

  hy.hrMaterialOpen(matGray, hy.HR_WRITE_DISCARD);
  matNode = hy.hrMaterialParamNode(matGray);
  diff = matNode.append_child("diffuse");
  diff.append_attribute("brdf_type").set_value("lambert");
  diff.append_child("color").append_attribute("val").set_value("0.5 0.5 0.5");
  hy.hrMaterialClose(matGray);
  
  matBlend1 = hy.hrMaterialCreateBlend("matBlend1", matGold, matSilver);
  matBlend2 = hy.hrMaterialCreateBlend("matBlend2", matLacquer, matGlass);
  matBlend3 = hy.hrMaterialCreateBlend("matBlend3", matBricks1, matBricks2);
    
    
  hy.hrMaterialOpen(matBlend1, hy.HR_WRITE_DISCARD);
  matNode = hy.hrMaterialParamNode(matBlend1);
  blend = matNode.append_child("blend");
  blend.append_attribute("type").set_value("mask_blend");
  mask = blend.append_child("mask");
  mask.append_attribute("val").set_value(1.0);
  texNode = mask.append_child("texture");
  texNode.append_attribute("matrix");
  samplerMatrix3 = np.array([ 4,  0, 0, 0,
                              0,  4, 0, 0,
                              0,  0, 1, 0,
                              0,  0, 0, 1], dtype = np.float32)
  texNode.append_attribute("addressing_mode_u").set_value("wrap");
  texNode.append_attribute("addressing_mode_v").set_value("wrap");
  texNode.append_attribute("input_gamma").set_value(2.2);
  texNode.append_attribute("input_alpha").set_value("rgb");
  hy.WriteMatrix4x4(texNode, "matrix", samplerMatrix3);
  hy.hrTextureBind(texChecker, mask);
  hy.hrMaterialClose(matBlend1);
  
  hy.hrMaterialOpen(matBlend2, hy.HR_WRITE_DISCARD);
  matNode = hy.hrMaterialParamNode(matBlend2);
  blend = matNode.append_child("blend");
  blend.append_attribute("type").set_value("mask_blend");
  mask = blend.append_child("mask");
  mask.append_attribute("val").set_value(1.0);
  texNode = mask.append_child("texture");
  texNode.append_attribute("matrix");
  texNode.append_attribute("addressing_mode_u").set_value("wrap");
  texNode.append_attribute("addressing_mode_v").set_value("wrap");
  texNode.append_attribute("input_gamma").set_value(2.2);
  texNode.append_attribute("input_alpha").set_value("rgb");
  hy.WriteMatrix4x4(texNode, "matrix", samplerMatrix3);
  hy.hrTextureBind(texYinYang, mask);
  hy.hrMaterialClose(matBlend2);
  
  hy.hrMaterialOpen(matBlend3, hy.HR_WRITE_DISCARD);
  matNode = hy.hrMaterialParamNode(matBlend3);
  blend = matNode.append_child("blend");
  blend.append_attribute("type").set_value("mask_blend");
  mask = blend.append_child("mask");
  mask.append_attribute("val").set_value(1.0);
  texNode = mask.append_child("texture");
  texNode.append_attribute("matrix");
  texNode.append_attribute("addressing_mode_u").set_value("wrap");
  texNode.append_attribute("addressing_mode_v").set_value("wrap");
  texNode.append_attribute("input_gamma").set_value(2.2);
  texNode.append_attribute("input_alpha").set_value("rgb");
  hy.WriteMatrix4x4(texNode, "matrix", samplerMatrix3);
  hy.hrTextureBind(texChecker, mask);
  hy.hrMaterialClose(matBlend3);

  cubeOpenRef = createCubeOpenRef("box", 18.0, matGray.id);
  teapotRef1 = hy.hrMeshCreateFromFileDL("../../main/data/meshes/teapot.vsgf")
  teapotRef2 = hy.hrMeshCreateFromFileDL("../../main/data/meshes/teapot.vsgf")
  teapotRef3 = hy.hrMeshCreateFromFileDL("../../main/data/meshes/teapot.vsgf")
  
  hy.hrMeshOpen(teapotRef1, hy.HR_TRIANGLE_IND3, hy.HR_OPEN_EXISTING)
  hy.hrMeshMaterialId(teapotRef1, matBlend1.id)
  hy.hrMeshClose(teapotRef1)
  
  hy.hrMeshOpen(teapotRef2, hy.HR_TRIANGLE_IND3, hy.HR_OPEN_EXISTING)
  hy.hrMeshMaterialId(teapotRef2, matBlend2.id)
  hy.hrMeshClose(teapotRef2)
  
  hy.hrMeshOpen(teapotRef3, hy.HR_TRIANGLE_IND3, hy.HR_OPEN_EXISTING)
  hy.hrMeshMaterialId(teapotRef3, matBlend3.id)
  hy.hrMeshClose(teapotRef3)

  camRef = hy.hrCameraCreate("my camera")
  hy.hrCameraOpen(camRef, hy.HR_WRITE_DISCARD)
  camNode = hy.hrCameraParamNode(camRef)
  camNode.append_child("fov").text().set("45")
  camNode.append_child("nearClipPlane").text().set("0.01")
  camNode.append_child("farClipPlane").text().set("100.0")
  camNode.append_child("up").text().set("0 1 0")
  camNode.append_child("position").text().set("0 10 8")
  camNode.append_child("look_at").text().set("0 0 0")
  hy.hrCameraClose(camRef)


  light = hy.hrLightCreate("my_area_light")

  hy.hrLightOpen(light, hy.HR_WRITE_DISCARD);
  lightNode = hy.hrLightParamNode(light);
  lightNode.attribute("type").set_value("area");
  lightNode.attribute("shape").set_value("rect");
  lightNode.attribute("distribution").set_value("diffuse");

  sizeNode = lightNode.append_child("size")
  sizeNode.append_attribute("half_length").set_value(1.0);
  sizeNode.append_attribute("half_width").set_value(1.0);

  intensityNode = lightNode.append_child("intensity");
  intensityNode.append_child("color").text().set("1 1 1");
  intensityNode.append_child("multiplier").text().set("10.0");
  hy.hrLightClose(light);

  renderRef = hy.hrRenderCreate("HydraModern")
  hy.hrRenderEnableDevice(renderRef, 0, True);
  hy.hrRenderOpen(renderRef, hy.HR_WRITE_DISCARD)
  node = hy.hrRenderParamNode(renderRef)
  node.force_child("width").text().set(1024)
  node.force_child("height").text().set(768)
  node.force_child("method_primary").text().set("pathtracing")
  node.force_child("method_caustic").text().set("pathtracing")
  node.force_child("trace_depth").text().set(5)
  node.force_child("diff_trace_depth").text().set(3)
  node.force_child("maxRaysPerPixel").text().set(512)
  hy.hrRenderClose(renderRef)

  scnRef = hy.hrSceneCreate("my scene")

  mTmp = np.dot(rotateYM4x4(DEG_TO_RAD * 90.0), scaleM4x4(np.array([3.3, 3.3, 3.3])))
  matrixTeapot_1 = np.dot(translateM4x4(np.array([-4.25, 1.25, 0.0])), mTmp)
  matrixTeapot_2 = np.dot(translateM4x4(np.array([0.0, 1.25, 0.0])), mTmp)
  matrixTeapot_3 = np.dot(translateM4x4(np.array([4.25, 1.25, 0.0])), mTmp)
  matrixT_2 = np.dot(translateM4x4(np.array([0.0, -1.0, 0.0])), rotateYM4x4(DEG_TO_RAD * 180.0))
  matrixT_light = translateM4x4(np.array([0.0, 15.5, 0.0]))
  mI = identityM4x4()

  hy.hrSceneOpen(scnRef, hy.HR_WRITE_DISCARD)
  hy.hrMeshInstance(scnRef, teapotRef1, matrixTeapot_1.flatten())
  hy.hrMeshInstance(scnRef, teapotRef2, matrixTeapot_2.flatten())
  hy.hrMeshInstance(scnRef, teapotRef3, matrixTeapot_3.flatten())
  hy.hrMeshInstance(scnRef, cubeOpenRef, matrixT_2.flatten())
  hy.hrLightInstance(scnRef, light, matrixT_light.flatten());
  hy.hrSceneClose(scnRef)
  hy.hrFlush(scnRef, renderRef, camRef)
  

  if(inBG):
    runRenderInBG(renderRef, 1024, 768, test_name)
    (res, mse) = check_images(test_name, 1, 30.0)
    if(res):
      report_file.write(test_name + " PASSED, MSE : {}\n".format(mse))
    else:
      report_file.write(test_name + " FAILED, MSE : {}\n".format(mse))   
  else:
    initAndStartOpenGL(renderRef, 1024, 768, test_name)
  return


def test07_sky_hdr_rotate(report_file, inBG):
  test_name = "test07_sky_hdr_rotate"
  hy.hrSceneLibraryOpen("tests/" + test_name, hy.HR_WRITE_DISCARD)
  
  matGray = hy.hrMaterialCreate("matGray");
  matChecker = hy.hrMaterialCreate("matChecker");
  matRefl = hy.hrMaterialCreate("matRefl");
  texEnv = hy.hrTexture2DCreateFromFile("../../main/data/textures/23_antwerp_night.hdr");
  texCheck = hy.hrTexture2DCreateFromFile("../../main/data/textures/chess_white.bmp");
  
  hy.hrMaterialOpen(matGray, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matGray)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("0.5 0.5 0.5")
  hy.hrMaterialClose(matGray)
  
  hy.hrMaterialOpen(matChecker, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matChecker)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("0.5 0.5 0.5")
  texNode = hy.hrTextureBind(texCheck, diff.child("color"));
  texNode.append_attribute("matrix");
  samplerMatrix = np.array([8, 0, 0, 0,
                            0, 8, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1], dtype = np.float32)

  texNode.append_attribute("addressing_mode_u").set_value("repeat");
  texNode.append_attribute("addressing_mode_v").set_value("repeat");
  hy.WriteMatrix4x4(texNode, "matrix", samplerMatrix); 
  hy.hrMaterialClose(matChecker)
  
  
  hy.hrMaterialOpen(matRefl, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matRefl)
  refl = matNode.append_child("reflectivity")
  refl.append_attribute("brdf_type").set_value("torranse_sparrow")
  refl.append_child("color").text().set("0.8 0.8 0.8")
  refl.append_child("glossiness").text().set("0.98")
  refl.append_child("extrusion").append_attribute("val").set_value("maxcolor")
  refl.append_child("fresnel_IOR").text().set("8");
  refl.append_child("fresnel").text().set("1");
  hy.hrMaterialClose(matRefl)
  

  cubeRef = createCubeRef("cube", 2.0, matGray.id);
  planeRef = createPlaneRef("plane", 15.0, matChecker.id)
  teapotRef = hy.hrMeshCreateFromFileDL("../../main/data/meshes/teapot.vsgf")
  
  hy.hrMeshOpen(teapotRef, hy.HR_TRIANGLE_IND3, hy.HR_OPEN_EXISTING)
  hy.hrMeshMaterialId(teapotRef, matRefl.id)
  hy.hrMeshClose(teapotRef)

  camRef = hy.hrCameraCreate("my camera")
  hy.hrCameraOpen(camRef, hy.HR_WRITE_DISCARD)
  camNode = hy.hrCameraParamNode(camRef)
  camNode.append_child("fov").text().set("45")
  camNode.append_child("nearClipPlane").text().set("0.01")
  camNode.append_child("farClipPlane").text().set("100.0")
  camNode.append_child("up").text().set("0 1 0")
  camNode.append_child("position").text().set("0 13 16")
  camNode.append_child("look_at").text().set("0 0 0")
  hy.hrCameraClose(camRef)


  light = hy.hrLightCreate("sky")

  hy.hrLightOpen(light, hy.HR_WRITE_DISCARD);
  lightNode = hy.hrLightParamNode(light);
  lightNode.attribute("type").set_value("sky");
  lightNode.attribute("distribution").set_value("map");

  intensityNode = lightNode.append_child("intensity");
  intensityNode.append_child("color").text().set("1 1 1");
  intensityNode.append_child("multiplier").text().set("1.0");
  texNode = hy.hrTextureBind(texEnv, intensityNode.child("color"));
  texNode.append_attribute("matrix");
  samplerMatrix2 = np.array([1, 0, 0, -0.4,
                            0, 1, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1], dtype = np.float32)

  texNode.append_attribute("addressing_mode_u").set_value("wrap");
  texNode.append_attribute("addressing_mode_v").set_value("wrap");
  hy.WriteMatrix4x4(texNode, "matrix", samplerMatrix2); 
  hy.hrLightClose(light);

  renderRef = hy.hrRenderCreate("HydraModern")
  hy.hrRenderEnableDevice(renderRef, 0, True);
  hy.hrRenderOpen(renderRef, hy.HR_WRITE_DISCARD)
  node = hy.hrRenderParamNode(renderRef)
  node.force_child("width").text().set(1024)
  node.force_child("height").text().set(768)
  node.force_child("method_primary").text().set("pathtracing")
  node.force_child("trace_depth").text().set(5)
  node.force_child("diff_trace_depth").text().set(3)
  node.force_child("maxRaysPerPixel").text().set(1024)
  hy.hrRenderClose(renderRef)


  scnRef = hy.hrSceneCreate("my scene")
  matrixT_Cube = np.dot(translateM4x4(np.array([-4.75, 1.0, 5.0])), rotateYM4x4(DEG_TO_RAD * 60.0))
  matrixT_RotScale1 = np.dot(scaleM4x4(np.array([3.0, 3.0, 3.0])),rotateYM4x4(DEG_TO_RAD * (-60.0)))
  matrixT_RotScale2 = np.dot(scaleM4x4(np.array([4.0, 4.0, 4.0])),rotateYM4x4(DEG_TO_RAD * (45.0)))
  matrixT_Teapot1 = np.dot(translateM4x4(np.array([4.0, 1.0, 5.0])), matrixT_RotScale1)
  matrixT_Teapot2 = np.dot(translateM4x4(np.array([0.0, 1.0, 0.0])), matrixT_RotScale2)
  matrixT_Plane = translateM4x4(np.array([0.0, -1.0, 0.0]))
  matrixT_light = translateM4x4(np.array([0.0, 3.85, 0.0]))
  mI = identityM4x4()

  hy.hrSceneOpen(scnRef, hy.HR_WRITE_DISCARD)
  hy.hrMeshInstance(scnRef, teapotRef, matrixT_Teapot1.flatten())
  hy.hrMeshInstance(scnRef, teapotRef, matrixT_Teapot2.flatten())
  hy.hrMeshInstance(scnRef, cubeRef, matrixT_Cube.flatten())
  hy.hrMeshInstance(scnRef, planeRef, matrixT_Plane.flatten())
  hy.hrLightInstance(scnRef, light, matrixT_light.flatten());
  hy.hrSceneClose(scnRef)
  hy.hrFlush(scnRef, renderRef, camRef)
  

  if(inBG):
    runRenderInBG(renderRef, 1024, 768, test_name)
    (res, mse) = check_images(test_name, 1, 80.0)
    if(res):
      report_file.write(test_name + " PASSED, MSE : {}\n".format(mse))
    else:
      report_file.write(test_name + " FAILED, MSE : {}\n".format(mse))   
  else:
    initAndStartOpenGL(renderRef, 1024, 768, test_name)
  return
  
def test08_shadow_catcher(report_file, inBG):
  test_name = "test08_shadow_catcher"
  hy.hrSceneLibraryOpen("tests/" + test_name, hy.HR_WRITE_DISCARD)
  
  matGray = hy.hrMaterialCreate("matGray");
  matChecker = hy.hrMaterialCreate("matChecker");
  matCatcher = hy.hrMaterialCreate("matCatcher");
  texEnv = hy.hrTexture2DCreateFromFile("../../main/data/textures/23_antwerp_night.hdr");
  texCheck = hy.hrTexture2DCreateFromFile("../../main/data/textures/chess_white.bmp");
  
  hy.hrMaterialOpen(matGray, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matGray)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("0.5 0.5 0.5")
  hy.hrMaterialClose(matGray)
  
  hy.hrMaterialOpen(matChecker, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matChecker)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("0.5 0.5 0.5")
  texNode = hy.hrTextureBind(texCheck, diff.child("color"));
  texNode.append_attribute("matrix");
  samplerMatrix = np.array([2, 0, 0, 0,
                            0, 2, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1], dtype = np.float32)

  texNode.append_attribute("addressing_mode_u").set_value("repeat");
  texNode.append_attribute("addressing_mode_v").set_value("repeat");
  hy.WriteMatrix4x4(texNode, "matrix", samplerMatrix); 
  hy.hrMaterialClose(matChecker)
  
  
  hy.hrMaterialOpen(matCatcher, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matCatcher)
  matNode.attribute("type").set_value("shadow_catcher");
  hy.hrMaterialClose(matCatcher)
  

  cubeRef = createCubeRef("cube", 2.0, matGray.id);
  planeRef = createPlaneRef("plane", 15.0, matCatcher.id)
  teapotRef = hy.hrMeshCreateFromFileDL("../../main/data/meshes/teapot.vsgf")
  
  hy.hrMeshOpen(teapotRef, hy.HR_TRIANGLE_IND3, hy.HR_OPEN_EXISTING)
  hy.hrMeshMaterialId(teapotRef, matChecker.id)
  hy.hrMeshClose(teapotRef)

  camRef = hy.hrCameraCreate("my camera")
  hy.hrCameraOpen(camRef, hy.HR_WRITE_DISCARD)
  camNode = hy.hrCameraParamNode(camRef)
  camNode.append_child("fov").text().set("45")
  camNode.append_child("nearClipPlane").text().set("0.01")
  camNode.append_child("farClipPlane").text().set("100.0")
  camNode.append_child("up").text().set("0 1 0")
  camNode.append_child("position").text().set("0 13 16")
  camNode.append_child("look_at").text().set("0 0 0")
  hy.hrCameraClose(camRef)


  light = hy.hrLightCreate("sky")

  hy.hrLightOpen(light, hy.HR_WRITE_DISCARD);
  lightNode = hy.hrLightParamNode(light);
  lightNode.attribute("type").set_value("sky");
  lightNode.attribute("distribution").set_value("map");

  intensityNode = lightNode.append_child("intensity");
  intensityNode.append_child("color").text().set("1 1 1");
  intensityNode.append_child("multiplier").text().set("1.0");
  texNode = hy.hrTextureBind(texEnv, intensityNode.child("color"));
  hy.hrLightClose(light);

  renderRef = hy.hrRenderCreate("HydraModern")
  hy.hrRenderEnableDevice(renderRef, 0, True);
  hy.hrRenderOpen(renderRef, hy.HR_WRITE_DISCARD)
  node = hy.hrRenderParamNode(renderRef)
  node.force_child("width").text().set(1024)
  node.force_child("height").text().set(768)
  node.force_child("method_primary").text().set("pathtracing")
  node.force_child("method_caustic").text().set("none")
  node.force_child("trace_depth").text().set(5)
  node.force_child("diff_trace_depth").text().set(3)
  node.force_child("maxRaysPerPixel").text().set(1024)
  hy.hrRenderClose(renderRef)


  scnRef = hy.hrSceneCreate("my scene")
  matrixT_Cube = np.dot(translateM4x4(np.array([2, 4.0, 5.0])), rotateYM4x4(DEG_TO_RAD * 60.0))
  matrixT_RotScale = np.dot(scaleM4x4(np.array([4.0, 4.0, 4.0])),rotateYM4x4(DEG_TO_RAD * (-45.0)))
  matrixT_Teapot = np.dot(translateM4x4(np.array([-8.0, -3.0, -2.0])), matrixT_RotScale)
  matrixT_Plane = translateM4x4(np.array([0.0, -1.0, 0.0]))
  matrixT_light = translateM4x4(np.array([0.0, 3.85, 0.0]))
  mI = identityM4x4()

  hy.hrSceneOpen(scnRef, hy.HR_WRITE_DISCARD)
  hy.hrMeshInstance(scnRef, teapotRef, matrixT_Teapot.flatten())
  hy.hrMeshInstance(scnRef, cubeRef, matrixT_Cube.flatten())
  hy.hrMeshInstance(scnRef, planeRef, matrixT_Plane.flatten())
  hy.hrLightInstance(scnRef, light, matrixT_light.flatten());
  hy.hrSceneClose(scnRef)
  hy.hrFlush(scnRef, renderRef, camRef)
  

  if(inBG):
    runRenderInBG(renderRef, 1024, 768, test_name)
    (res, mse) = check_images(test_name, 1, 80.0)
    if(res):
      report_file.write(test_name + " PASSED, MSE : {}\n".format(mse))
    else:
      report_file.write(test_name + " FAILED, MSE : {}\n".format(mse))   
  else:
    initAndStartOpenGL(renderRef, 1024, 768, test_name)
  return
  
def test09_load_car(library_path, report_file, inBG):
  test_name = "test09_load_car"

  hy.hrSceneLibraryOpen(library_path, hy.HR_OPEN_EXISTING)
   
  renderRef = hy.HRRenderRef()
  renderRef.id = 0

  scnRef = hy.HRSceneInstRef()
  scnRef.id = 0
  
  camRef = hy.hrFindCameraByName("PhysCamera001")
  
  hy.hrRenderEnableDevice(renderRef, 1, True)
  #hy.hrFlush(scnRef, renderRef, camRef)
  hy.hrCommit(scnRef, renderRef, camRef)
  hy.hrRenderCommand(renderRef, "resume", "")
  
  if(inBG):
    runRenderInBG(renderRef, 1920, 1080, test_name)
    (res, mse) = check_images(test_name, 1, 50.0)
    if(res):
      report_file.write(test_name + " PASSED, MSE : {}\n".format(mse))
    else:
      report_file.write(test_name + " FAILED, MSE : {}\n".format(mse))   
  else:
    initAndStartOpenGL(renderRef, 1920, 1080, test_name)
  
def test10_sky_sun_physical(report_file, inBG):
  test_name = "test10_sky_sun_physical"
  hy.hrSceneLibraryOpen("tests/" + test_name, hy.HR_WRITE_DISCARD)
  
  matGray = hy.hrMaterialCreate("matGray");
  matRefl = hy.hrMaterialCreate("matRefl");
  
  hy.hrMaterialOpen(matGray, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matGray)
  diff = matNode.append_child("diffuse")
  diff.append_attribute("brdf_type").set_value("lambert")
  diff.append_child("color").text().set("0.5 0.5 0.5")
  hy.hrMaterialClose(matGray)
  
  hy.hrMaterialOpen(matRefl, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matRefl)
  refl = matNode.append_child("reflectivity")
  refl.append_attribute("brdf_type").set_value("torranse_sparrow")
  refl.append_child("color").text().set("0.8 0.8 0.8")
  refl.append_child("glossiness").text().set("0.98")
  refl.append_child("extrusion").append_attribute("val").set_value("maxcolor")
  refl.append_child("fresnel_IOR").text().set("8");
  refl.append_child("fresnel").text().set("1");
  hy.hrMaterialClose(matRefl)
  

  cubeRef = createCubeRef("cube", 2.0, matGray.id);
  pillarRef = createCubeRef("pillar", 1.0, matGray.id);
  planeRef = createPlaneRef("plane", 150.0, matGray.id)
  teapotRef = hy.hrMeshCreateFromFileDL("../../main/data/meshes/teapot.vsgf")
  
  hy.hrMeshOpen(teapotRef, hy.HR_TRIANGLE_IND3, hy.HR_OPEN_EXISTING)
  hy.hrMeshMaterialId(teapotRef, matRefl.id)
  hy.hrMeshClose(teapotRef)

  camRef = hy.hrCameraCreate("my camera")
  hy.hrCameraOpen(camRef, hy.HR_WRITE_DISCARD)
  camNode = hy.hrCameraParamNode(camRef)
  camNode.append_child("fov").text().set("45")
  camNode.append_child("nearClipPlane").text().set("0.01")
  camNode.append_child("farClipPlane").text().set("100.0")
  camNode.append_child("up").text().set("0 1 0")
  camNode.append_child("position").text().set("0 7 25")
  camNode.append_child("look_at").text().set("0 3 0")
  hy.hrCameraClose(camRef)


  sky = hy.hrLightCreate("sky")
  sun = hy.hrLightCreate("sun")

  hy.hrLightOpen(sky, hy.HR_WRITE_DISCARD);
  lightNode = hy.hrLightParamNode(sky);
  lightNode.attribute("type").set_value("sky");
  lightNode.attribute("distribution").set_value("perez");

  intensityNode = lightNode.append_child("intensity");
  intensityNode.append_child("color").text().set("1 1 1");
  intensityNode.append_child("multiplier").text().set("1.0");
  
  sunModel = lightNode.append_child("perez")
  sunModel.append_attribute("sun_id").set_value(sun.id)
  sunModel.append_attribute("turbidity").set_value(2.0)
  
  hy.hrLightClose(sky)
  
  hy.hrLightOpen(sun, hy.HR_WRITE_DISCARD);
  lightNode = hy.hrLightParamNode(sun);
  lightNode.attribute("type").set_value("directional")
  lightNode.attribute("shape").set_value("point")
  lightNode.attribute("distribution").set_value("directional")
  
  sizeNode = lightNode.append_child("size")
  sizeNode.append_attribute("inner_radius").set_value(0.0);
  sizeNode.append_attribute("outer_radius").set_value(1000.0);

  intensityNode = lightNode.append_child("intensity");

  intensityNode.append_child("color").text().set("1.0 0.85 0.64");
  intensityNode.append_child("multiplier").text().set("3.0");
  
  lightNode.append_child("angle_radius").append_attribute("val").set_value(0.5)
  hy.hrLightClose(sky);

  renderRef = hy.hrRenderCreate("HydraModern")
  hy.hrRenderEnableDevice(renderRef, 0, True);
  hy.hrRenderOpen(renderRef, hy.HR_WRITE_DISCARD)
  node = hy.hrRenderParamNode(renderRef)
  node.force_child("width").text().set(1024)
  node.force_child("height").text().set(768)
  node.force_child("method_primary").text().set("pathtracing")
  node.force_child("trace_depth").text().set(5)
  node.force_child("diff_trace_depth").text().set(3)
  node.force_child("maxRaysPerPixel").text().set(1024)
  hy.hrRenderClose(renderRef)


  scnRef = hy.hrSceneCreate("my scene")
  matrixT_Cube = np.dot(translateM4x4(np.array([-4.75, 1.0, 5.0])), rotateYM4x4(DEG_TO_RAD * 60.0))
  matrixT_Pillar1 = np.dot(translateM4x4(np.array([15.0, 7.0, -15.0])),scaleM4x4(np.array([2.0, 8.0, 2.0])))
  matrixT_Pillar2 = np.dot(translateM4x4(np.array([-15.0, 7.0, -15.0])),scaleM4x4(np.array([2.0, 8.0, 2.0])))
  matrixT_Teapot1 = np.dot(translateM4x4(np.array([4.0, 1.0, 5.5])),scaleM4x4(np.array([3.0, 3.0, 3.0])))
  matrixT_Teapot2 = np.dot(translateM4x4(np.array([0.0, 3.0, -1.0])),scaleM4x4(np.array([1.8, 1.8, 1.8])))
  matrixT_Plane = translateM4x4(np.array([0.0, -1.0, 0.0]))
  matRot = np.dot(rotateZM4x4(DEG_TO_RAD * (15.0)), rotateXM4x4(DEG_TO_RAD * (30.0)))
  matrixT_light =  np.dot(translateM4x4(np.array([200.0, 20.0, -50.0])), matRot)
  mI = identityM4x4()

  hy.hrSceneOpen(scnRef, hy.HR_WRITE_DISCARD)
  hy.hrMeshInstance(scnRef, pillarRef, matrixT_Pillar1.flatten())
  hy.hrMeshInstance(scnRef, pillarRef, matrixT_Pillar2.flatten())
  hy.hrMeshInstance(scnRef, teapotRef, matrixT_Teapot1.flatten())
  hy.hrMeshInstance(scnRef, teapotRef, matrixT_Teapot2.flatten())
  hy.hrMeshInstance(scnRef, cubeRef, matrixT_Cube.flatten())
  hy.hrMeshInstance(scnRef, planeRef, matrixT_Plane.flatten())
  hy.hrLightInstance(scnRef, sun, matrixT_light.flatten());
  hy.hrLightInstance(scnRef, sky, mI.flatten());
  hy.hrSceneClose(scnRef)
  hy.hrFlush(scnRef, renderRef, camRef)
  

  if(inBG):
    runRenderInBG(renderRef, 1024, 768, test_name)
    (res, mse) = check_images(test_name, 1, 80.0)
    if(res):
      report_file.write(test_name + " PASSED, MSE : {}\n".format(mse))
    else:
      report_file.write(test_name + " FAILED, MSE : {}\n".format(mse))   
  else:
    initAndStartOpenGL(renderRef, 1024, 768, test_name)
  return
 
def test11_load_car_and_change_env(library_path, report_file, inBG):
  test_name = "test11_load_car_and_change_env"

  hy.hrSceneLibraryOpen(library_path, hy.HR_OPEN_EXISTING)
    
  renderRef = hy.HRRenderRef()
  renderRef.id = 0

  scnRef = hy.HRSceneInstRef()
  scnRef.id = 0
  
  camRef = hy.hrFindCameraByName("PhysCamera001")
  
  hy.hrRenderEnableDevice(renderRef, 1, True)
  hy.hrCommit(scnRef, renderRef);
  hy.hrRenderCommand(renderRef, "pause", "");
  time.sleep(0.25) #let render rip
  
  texEnv = hy.hrTexture2DCreateFromFile("../../main/data/textures/Factory_Catwalk_2k_BLUR.exr");
  
  matBGRef = hy.hrFindMaterialByName("ground")
  print("matBGRef.id = {}".format(matBGRef.id))
  hy.hrMaterialOpen(matBGRef, hy.HR_WRITE_DISCARD)
  matNode = hy.hrMaterialParamNode(matBGRef)
  matNode.attribute("type").set_value("shadow_catcher");
  hy.hrMaterialClose(matBGRef)
  
  
  light = hy.hrFindLightByName("environment")
  
  hy.hrLightOpen(light, hy.HR_WRITE_DISCARD);
  lightNode = hy.hrLightParamNode(light);
  lightNode.attribute("type").set_value("sky");
  lightNode.attribute("distribution").set_value("map");

  intensityNode = lightNode.append_child("intensity");
  intensityNode.append_child("color").text().set("1 1 1");
  intensityNode.append_child("multiplier").text().set("1.0");
  texNode = hy.hrTextureBind(texEnv, intensityNode.child("color"));
  texNode.append_attribute("matrix");
  samplerMatrix2 = np.array([1, 0, 0, -0.4,
                            0, 1, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1], dtype = np.float32)

  texNode.append_attribute("addressing_mode_u").set_value("wrap");
  texNode.append_attribute("addressing_mode_v").set_value("wrap");
  hy.WriteMatrix4x4(texNode, "matrix", samplerMatrix2); 
  hy.hrLightClose(light);
  

  hy.hrFlush(scnRef, renderRef, camRef)
  
  
  if(inBG):
    runRenderInBG(renderRef, 1920, 1080, test_name, [2])
    (res, mse) = check_images(test_name, 1, 50.0)
    if(res):
      report_file.write(test_name + " PASSED, MSE : {}\n".format(mse))
    else:
      report_file.write(test_name + " FAILED, MSE : {}\n".format(mse))   
  else:
    initAndStartOpenGL(renderRef, 1920, 1080, test_name, [2])
 
def test12_cornell_box_gbuffer(report_file, inBG):

  test_name = "test12_cornell_box_gbuffer"
  
  hy.hrSceneLibraryOpen("tests/" + test_name, hy.HR_OPEN_EXISTING)
    
  renderRef = hy.HRRenderRef()
  renderRef.id = 0

  scnRef = hy.HRSceneInstRef()
  scnRef.id = 0
  
  camRef = hy.hrFindCameraByName("PhysCamera001")
  
  hy.hrRenderEnableDevice(renderRef, 0, True)
  hy.hrCommit(scnRef, renderRef);
  hy.hrRenderCommand(renderRef, "pause", "");
  time.sleep(0.5) #let render rip
  

  hy.hrFlush(scnRef, renderRef, camRef)
  
  
  if(inBG):
    runRenderInBG(renderRef, 1024, 768, test_name, [2])
    saveAllGBuffferLayers(renderRef, test_name)
    (res, mse) = check_images(test_name, 10, 50.0) #check rendered image + 9 gbuffer layers
    if(res):
      report_file.write(test_name + " PASSED, MSE : {}\n".format(mse))
    else:
      report_file.write(test_name + " FAILED, MSE : {}\n".format(mse))   
  else:
    initAndStartOpenGL(renderRef, 1024, 768, test_name, [2])
    saveAllGBuffferLayers(renderRef, test_name)

  return
  
def test13_transform_instances(report_file, inBG):
  test_name = "test13_transform_instances"

  hy.hrSceneLibraryOpen("tests/" + test_name, hy.HR_OPEN_EXISTING)
   
  renderRef = hy.HRRenderRef()
  renderRef.id = 0

  scnRef = hy.HRSceneInstRef()
  scnRef.id = 0
  
  camRef = hy.hrFindCameraByName("PhysCamera001")
  
  hy.hrRenderEnableDevice(renderRef, 0, True)
  hy.hrCommit(scnRef, renderRef);
  hy.hrRenderCommand(renderRef, "pause", "");
  time.sleep(0.5) #let render rip
  
  matrix = rotateYM4x4(45 * DEG_TO_RAD)
  
  hy.TransformAllInstances(scnRef, matrix.flatten(), False)

  hy.hrFlush(scnRef, renderRef, camRef)
  
  if(inBG):
    runRenderInBG(renderRef, 1024, 768, test_name, [2])
    (res, mse) = check_images(test_name, 1, 50.0)
    if(res):
      report_file.write(test_name + " PASSED, MSE : {}\n".format(mse))
    else:
      report_file.write(test_name + " FAILED, MSE : {}\n".format(mse))   
  else:
    initAndStartOpenGL(renderRef, 1024, 768, test_name, [2])
  
def run_tests():
  hy.hrInit("-copy_textures_to_local_folder 1 -local_data_path 1 ")

  with open("test_report.txt", "w") as report_file:
#    test01_render_cubes(report_file, False)
#    test02_mesh_from_vsgf(report_file, False)    
#    test03_cornell_box(report_file, False)    
#    test04_instancing(report_file, False) 
#    test05_load_existing(report_file, False)
#    test06_blend_simple(report_file, False)
#    test07_sky_hdr_rotate(report_file, False)
#    test08_shadow_catcher(report_file, False)
#    test09_load_car("tests/test09_load_car", report_file, False)
#    test10_sky_sun_physical(report_file, False)
#    test11_load_car_and_change_env("tests/test11_load_car_and_change_env", report_file, False)
#    test12_cornell_box_gbuffer(report_file, False)
#    test13_transform_instances(report_file, False)
#    render_scene("tests/test04_instancing")

run_tests()
