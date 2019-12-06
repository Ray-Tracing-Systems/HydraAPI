#!/usr/bin python3.5
# -*- coding: utf-8 -*-


import hydraPy as hy
import numpy as np
import time

from mesh_utils import *
from light_math import *
from test_utils import *


def run_render_bg(render_ref, path):
    while True:
        time.sleep(0.5)
        info = hy.hrRenderHaveUpdate(render_ref)
        if info.finalUpdate is True:
            hy.hrRenderSaveFrameBufferLDR(render_ref, path + "/z_out.png")
            hy.hrRenderCommand(render_ref, "exitnow", "")
            break


def cornell_box():
    demo_name = "cornell_box"
    init_info = hy.HRInitInfo()
    hy.hrSceneLibraryOpen("demos/scenes/" + demo_name, hy.HR_WRITE_DISCARD, init_info)

    matW = hy.hrMaterialCreate("mWhite")
    hy.hrMaterialOpen(matW, hy.HR_WRITE_DISCARD)
    matNode = hy.hrMaterialParamNode(matW)
    diff = matNode.append_child("diffuse")
    diff.append_attribute("brdf_type").set_value("lambert")
    diff.append_child("color").text().set("0.5 0.5 0.5")
    hy.hrMaterialClose(matW)

    matMetallic = hy.hrMaterialCreate("metallic")
    hy.hrMaterialOpen(matMetallic, hy.HR_WRITE_DISCARD)
    matNode = hy.hrMaterialParamNode(matMetallic)
    diff = matNode.append_child("diffuse")
    diff.append_attribute("brdf_type").set_value("lambert")
    diff.append_child("color").text().set("0.207843 0.188235 0.0")
    refl = matNode.append_child("reflectivity")
    refl.append_attribute("brdf_type").set_value("phong")
    refl.append_child("color").text().set("0.367059 0.345882 0")
    refl.append_child("glossiness").text().set("0.5")
    refl.append_child("fresnel_IOR").text().set("14")
    refl.append_child("fresnel").text().set("1")
    hy.hrMaterialClose(matMetallic)

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

    cubeOpenRef = createCornellCubeOpenRef("cornellBox", 4.0, matR.id, matG.id, matW.id)
    cubeRef = createCubeRef("cube", 0.5, matMetallic.id)

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
    hy.hrLightOpen(light, hy.HR_WRITE_DISCARD)
    lightNode = hy.hrLightParamNode(light)
    lightNode.attribute("type").set_value("area")
    lightNode.attribute("shape").set_value("rect")
    lightNode.attribute("distribution").set_value("diffuse")

    sizeNode = lightNode.append_child("size")
    sizeNode.append_attribute("half_length").set_value(1.0)
    sizeNode.append_attribute("half_width").set_value(1.0)

    intensityNode = lightNode.append_child("intensity")
    intensityNode.append_child("color").append_attribute("val").set_value("1 1 1")
    intensityNode.append_child("multiplier").append_attribute("val").set_value("10.0")
    hy.hrLightClose(light)

    render_w = 512
    render_h = 512

    renderRef = hy.hrRenderCreate("HydraModern")
    hy.hrRenderEnableDevice(renderRef, 0, True)
    hy.hrRenderOpen(renderRef, hy.HR_WRITE_DISCARD)
    node = hy.hrRenderParamNode(renderRef)
    node.force_child("width").text().set(render_w)
    node.force_child("height").text().set(render_h)
    node.force_child("method_primary").text().set("pathtracing")
    node.force_child("method_caustic").text().set("pathtracing")
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
    hy.hrMeshInstance(scnRef, cubeRef, matrixT_1.flatten())
    hy.hrMeshInstance(scnRef, cubeOpenRef, matrixT_2.flatten())
    hy.hrLightInstance(scnRef, light, matrixT_light.flatten())
    hy.hrSceneClose(scnRef)
    hy.hrFlush(scnRef, renderRef, camRef)

    run_render_bg(renderRef, "demos/images/" + demo_name)

    return

def blend_simple():

    demo_name = "blend_simple"
    init_info = hy.HRInitInfo()
    hy.hrSceneLibraryOpen("demos/scenes/" + demo_name, hy.HR_WRITE_DISCARD, init_info)

    matGold    = hy.hrMaterialCreate("matGold")
    matSilver  = hy.hrMaterialCreate("matSilver")
    matLacquer = hy.hrMaterialCreate("matLacquer")
    matGlass   = hy.hrMaterialCreate("matGlass")
    matBricks1 = hy.hrMaterialCreate("matBricks1")
    matBricks2 = hy.hrMaterialCreate("matBricks2")
    matGray = hy.hrMaterialCreate("matGray")

    texChecker = hy.hrTexture2DCreateFromFile("data/textures/chess_white.bmp")
    texYinYang = hy.hrTexture2DCreateFromFile("data/textures/yinyang.png")

    hy.hrMaterialOpen(matGold, hy.HR_WRITE_DISCARD)
    matNode = hy.hrMaterialParamNode(matGold)
    diff = matNode.append_child("diffuse")
    diff.append_attribute("brdf_type").set_value("lambert")
    diff.append_child("color").append_attribute("val").set_value("0.88 0.61 0.05")
    refl = matNode.append_child("reflectivity")
    refl.append_attribute("brdf_type").set_value("torranse_sparrow")
    refl.append_child("color").append_attribute("val").set_value("0.88 0.61 0.05")
    refl.append_child("glossiness").append_attribute("val").set_value("0.98")
    refl.append_child("extrusion").append_attribute("val").set_value("maxcolor")
    refl.append_child("fresnel").append_attribute("val").set_value(1)
    refl.append_child("fresnel_IOR").append_attribute("val").set_value(8.0)
    hy.hrMaterialClose(matGold)

    hy.hrMaterialOpen(matSilver, hy.HR_WRITE_DISCARD)
    matNode = hy.hrMaterialParamNode(matSilver)
    diff = matNode.append_child("diffuse")
    diff.append_attribute("brdf_type").set_value("lambert")
    diff.append_child("color").append_attribute("val").set_value("0.8 0.8 0.8")
    refl = matNode.append_child("reflectivity")
    refl.append_attribute("brdf_type").set_value("torranse_sparrow")
    refl.append_child("color").append_attribute("val").set_value("0.8 0.8 0.8")
    refl.append_child("glossiness").append_attribute("val").set_value("0.98")
    refl.append_child("extrusion").append_attribute("val").set_value("maxcolor")
    refl.append_child("fresnel").append_attribute("val").set_value(1)
    refl.append_child("fresnel_IOR").append_attribute("val").set_value(8.0)
    hy.hrMaterialClose(matSilver)

    hy.hrMaterialOpen(matLacquer, hy.HR_WRITE_DISCARD)
    matNode = hy.hrMaterialParamNode(matLacquer)
    diff = matNode.append_child("diffuse")
    diff.append_attribute("brdf_type").set_value("lambert")
    diff.append_child("color").append_attribute("val").set_value("0.05 0.05 0.05")
    refl = matNode.append_child("reflectivity")
    refl.append_attribute("brdf_type").set_value("phong")
    refl.append_child("color").append_attribute("val").set_value("0.5 0.5 0.5")
    refl.append_child("glossiness").append_attribute("val").set_value("1.0")
    refl.append_child("extrusion").append_attribute("val").set_value("maxcolor")
    refl.append_child("fresnel").append_attribute("val").set_value(1)
    refl.append_child("fresnel_IOR").append_attribute("val").set_value(1.5)
    hy.hrMaterialClose(matLacquer)

    hy.hrMaterialOpen(matGlass, hy.HR_WRITE_DISCARD)
    matNode = hy.hrMaterialParamNode(matGlass)
    refl = matNode.append_child("reflectivity")
    refl.append_attribute("brdf_type").set_value("phong")
    refl.append_child("color").append_attribute("val").set_value("0.1 0.1 0.1 ")
    refl.append_child("glossiness").append_attribute("val").set_value(1.0)
    refl.append_child("extrusion").append_attribute("val").set_value("maxcolor")
    refl.append_child("fresnel").append_attribute("val").set_value(1)
    refl.append_child("fresnel_IOR").append_attribute("val").set_value(1.5)
    transp = matNode.append_child("transparency")
    transp.append_attribute("brdf_type").set_value("phong")
    transp.append_child("color").append_attribute("val").set_value("1.0 1.0 1.0")
    transp.append_child("glossiness").append_attribute("val").set_value(1.0)
    transp.append_child("thin_walled").append_attribute("val").set_value(0)
    transp.append_child("fog_color").append_attribute("val").set_value("0.9 0.9 1.0")
    transp.append_child("fog_multiplier").append_attribute("val").set_value(1.0)
    transp.append_child("IOR").append_attribute("val").set_value(1.5)
    hy.hrMaterialClose(matGlass)

    hy.hrMaterialOpen(matBricks1, hy.HR_WRITE_DISCARD)
    matNode = hy.hrMaterialParamNode(matBricks1)
    diff = matNode.append_child("diffuse")
    diff.append_attribute("brdf_type").set_value("lambert")
    color = diff.append_child("color")
    color.append_attribute("val").set_value("0.2 0.2 0.75")
    color.append_attribute("tex_apply_mode ").set_value("multiply")
    texNode = hy.hrTextureBind(texChecker, diff.child("color"))
    texNode.append_attribute("matrix")
    samplerMatrix = np.array([ 16,  0, 0, 0,
                               0, 16, 0, 0,
                               0,  0, 1, 0,
                               0,  0, 0, 1], dtype = np.float32)
    texNode.append_attribute("addressing_mode_u").set_value("wrap")
    texNode.append_attribute("addressing_mode_v").set_value("wrap")
    texNode.append_attribute("input_gamma").set_value(2.2)
    texNode.append_attribute("input_alpha").set_value("rgb")
    hy.WriteMatrix4x4(texNode, "matrix", samplerMatrix)
    hy.hrMaterialClose(matBricks1)


    hy.hrMaterialOpen(matBricks2, hy.HR_WRITE_DISCARD)
    matNode = hy.hrMaterialParamNode(matBricks2)
    diff = matNode.append_child("diffuse")
    diff.append_attribute("brdf_type").set_value("lambert")
    color = diff.append_child("color")
    color.append_attribute("val").set_value("0.1 0.1 0.1")
    color.append_attribute("tex_apply_mode ").set_value("multiply")
    texNode = hy.hrTextureBind(texChecker, diff.child("color"))
    texNode.append_attribute("matrix")
    samplerMatrix2 = np.array([ 8,  0, 0, 0,
                                0,  8, 0, 0,
                                0,  0, 1, 0,
                                0,  0, 0, 1], dtype = np.float32)
    texNode.append_attribute("addressing_mode_u").set_value("wrap")
    texNode.append_attribute("addressing_mode_v").set_value("wrap")
    texNode.append_attribute("input_gamma").set_value(2.2)
    texNode.append_attribute("input_alpha").set_value("rgb")
    hy.WriteMatrix4x4(texNode, "matrix", samplerMatrix2)
    refl = matNode.append_child("reflectivity")
    refl.append_attribute("brdf_type").set_value("phong")
    refl.append_child("color").append_attribute("val").set_value("0.9 0.9 0.9")
    refl.append_child("glossiness").append_attribute("val").set_value("1.0")
    refl.append_child("extrusion").append_attribute("val").set_value("maxcolor")
    refl.append_child("fresnel").append_attribute("val").set_value(1)
    refl.append_child("fresnel_IOR").append_attribute("val").set_value(1.5)
    hy.hrMaterialClose(matBricks2)

    hy.hrMaterialOpen(matGray, hy.HR_WRITE_DISCARD)
    matNode = hy.hrMaterialParamNode(matGray)
    diff = matNode.append_child("diffuse")
    diff.append_attribute("brdf_type").set_value("lambert")
    diff.append_child("color").append_attribute("val").set_value("0.5 0.5 0.5")
    hy.hrMaterialClose(matGray)

    matBlend1 = hy.hrMaterialCreateBlend("matBlend1", matGold, matSilver)
    matBlend2 = hy.hrMaterialCreateBlend("matBlend2", matLacquer, matGlass)
    matBlend3 = hy.hrMaterialCreateBlend("matBlend3", matBricks1, matBricks2)


    hy.hrMaterialOpen(matBlend1, hy.HR_WRITE_DISCARD)
    matNode = hy.hrMaterialParamNode(matBlend1)
    blend = matNode.append_child("blend")
    blend.append_attribute("type").set_value("mask_blend")
    mask = blend.append_child("mask")
    mask.append_attribute("val").set_value(1.0)
    texNode = mask.append_child("texture")
    texNode.append_attribute("matrix")
    samplerMatrix3 = np.array([ 4,  0, 0, 0,
                                0,  4, 0, 0,
                                0,  0, 1, 0,
                                0,  0, 0, 1], dtype = np.float32)
    texNode.append_attribute("addressing_mode_u").set_value("wrap")
    texNode.append_attribute("addressing_mode_v").set_value("wrap")
    texNode.append_attribute("input_gamma").set_value(2.2)
    texNode.append_attribute("input_alpha").set_value("rgb")
    hy.WriteMatrix4x4(texNode, "matrix", samplerMatrix3)
    hy.hrTextureBind(texChecker, mask)
    hy.hrMaterialClose(matBlend1)

    hy.hrMaterialOpen(matBlend2, hy.HR_WRITE_DISCARD)
    matNode = hy.hrMaterialParamNode(matBlend2)
    blend = matNode.append_child("blend")
    blend.append_attribute("type").set_value("mask_blend")
    mask = blend.append_child("mask")
    mask.append_attribute("val").set_value(1.0)
    texNode = mask.append_child("texture")
    texNode.append_attribute("matrix")
    texNode.append_attribute("addressing_mode_u").set_value("wrap")
    texNode.append_attribute("addressing_mode_v").set_value("wrap")
    texNode.append_attribute("input_gamma").set_value(2.2)
    texNode.append_attribute("input_alpha").set_value("rgb")
    hy.WriteMatrix4x4(texNode, "matrix", samplerMatrix3)
    hy.hrTextureBind(texYinYang, mask)
    hy.hrMaterialClose(matBlend2)

    hy.hrMaterialOpen(matBlend3, hy.HR_WRITE_DISCARD)
    matNode = hy.hrMaterialParamNode(matBlend3)
    blend = matNode.append_child("blend")
    blend.append_attribute("type").set_value("mask_blend")
    mask = blend.append_child("mask")
    mask.append_attribute("val").set_value(1.0)
    texNode = mask.append_child("texture")
    texNode.append_attribute("matrix")
    texNode.append_attribute("addressing_mode_u").set_value("wrap")
    texNode.append_attribute("addressing_mode_v").set_value("wrap")
    texNode.append_attribute("input_gamma").set_value(2.2)
    texNode.append_attribute("input_alpha").set_value("rgb")
    hy.WriteMatrix4x4(texNode, "matrix", samplerMatrix3)
    hy.hrTextureBind(texChecker, mask)
    hy.hrMaterialClose(matBlend3)

    cubeOpenRef = createCubeOpenRef("box", 18.0, matGray.id)
    cubeRef     = createCubeRef("cube", 0.5, 1)


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

    hy.hrLightOpen(light, hy.HR_WRITE_DISCARD)
    lightNode = hy.hrLightParamNode(light)
    lightNode.attribute("type").set_value("area")
    lightNode.attribute("shape").set_value("rect")
    lightNode.attribute("distribution").set_value("diffuse")

    sizeNode = lightNode.append_child("size")
    sizeNode.append_attribute("half_length").set_value(1.0)
    sizeNode.append_attribute("half_width").set_value(1.0)

    intensityNode = lightNode.append_child("intensity")
    intensityNode.append_child("color").append_attribute("val").set_value("1 1 1")
    intensityNode.append_child("multiplier").append_attribute("val").set_value("50.0")
    hy.hrLightClose(light)

    render_w = 512
    render_h = 512

    renderRef = hy.hrRenderCreate("HydraModern")
    hy.hrRenderEnableDevice(renderRef, 0, True)
    hy.hrRenderOpen(renderRef, hy.HR_WRITE_DISCARD)
    node = hy.hrRenderParamNode(renderRef)
    node.force_child("width").text().set(render_w)
    node.force_child("height").text().set(render_h)
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

    remap_1 = np.array([1, matBlend1.id], dtype=np.int32)
    remap_2 = np.array([1, matBlend2.id], dtype=np.int32)
    remap_3 = np.array([1, matBlend3.id], dtype=np.int32)

    hy.hrSceneOpen(scnRef, hy.HR_WRITE_DISCARD)
    hy.hrMeshInstanceRemap(scnRef, cubeRef, matrixTeapot_1.flatten(), remap_1, 2)
    hy.hrMeshInstanceRemap(scnRef, cubeRef, matrixTeapot_2.flatten(), remap_2, 2)
    hy.hrMeshInstanceRemap(scnRef, cubeRef, matrixTeapot_3.flatten(), remap_3, 2)
    hy.hrMeshInstance(scnRef, cubeOpenRef, matrixT_2.flatten())
    hy.hrLightInstance(scnRef, light, matrixT_light.flatten())
    hy.hrSceneClose(scnRef)
    hy.hrFlush(scnRef, renderRef, camRef)

    run_render_bg(renderRef, "demos/images/" + demo_name)

    return

if __name__ == "__main__":
    cornell_box()
    blend_simple()