# -*- coding: utf-8 -*-
"""
Created on Mon Feb  5 17:14:21 2018

@author: vsan
"""

from scipy import misc

def LoadImageFromFile(path) :
  img = misc.imread(path)
  
  (h, w, bpp) = img.shape
  
  return (img, w, h)
    
def MSE(img1, img2) :
  img1 = (img1.flatten() / 255.0).astype(float)
  img2 = (img2.flatten() / 255.0).astype(float)
  
  if (img1.size != img2.size):
    return 100000.0

  tmp = img1 - img2

  return np.sum(tmp * tmp) / tmp.size


def check_images(a_path, a_numImages, a_mse) :
  g_MSEOutput = 0.0
  result = True
  
  for i in range (0, a_numImages):
    path1 = "tests_images/" + a_path + "/z_out"
    path2 = "tests_images/" + a_path + "/z_ref"
    path3 = "tests_images/" + a_path + "/w_ref"

    if (i > 0) :
      path1 += str(i + 1)
      path2 += str(i + 1)
      path3 += str(i + 1)

    path1 += ".png";
    path2 += ".png";
    path3 += ".png";

    path2 = path3 #use w_ref by deafult

    (data1, w1, h1) = LoadImageFromFile(path1)
    (data2, w2, h2) = LoadImageFromFile(path2)

    if (w1 != w2 or h1 != h2) :
      return (False, 100000.0)

    mseVal = float(w1 * h1) * MSE(data1, data2)

    g_MSEOutput = max(g_MSEOutput, mseVal)

    result = result and (mseVal <= a_mse)

  return (result, g_MSEOutput)

import hydraPy as hy
import numpy as np
import time
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *


def saveAllGBuffferLayers(renderRef, testName):
  hy.hrRenderSaveGBufferLayerLDR(renderRef, "tests_images/" + testName + "/z_out2.png", "depth", 0, 0)
  hy.hrRenderSaveGBufferLayerLDR(renderRef, "tests_images/" + testName + "/z_out3.png", "normals", 0, 0)
  hy.hrRenderSaveGBufferLayerLDR(renderRef, "tests_images/" + testName + "/z_out4.png", "texcoord", 0, 0)
  hy.hrRenderSaveGBufferLayerLDR(renderRef, "tests_images/" + testName + "/z_out5.png", "diffcolor", 0, 0)
  hy.hrRenderSaveGBufferLayerLDR(renderRef, "tests_images/" + testName + "/z_out6.png", "alpha", 0, 0)
  hy.hrRenderSaveGBufferLayerLDR(renderRef, "tests_images/" + testName + "/z_out7.png", "shadow", 0, 0)
  hy.hrRenderSaveGBufferLayerLDR(renderRef, "tests_images/" + testName + "/z_out8.png", "matid", 0, 0)
  hy.hrRenderSaveGBufferLayerLDR(renderRef, "tests_images/" + testName + "/z_out9.png", "objid", 0, 0)
  hy.hrRenderSaveGBufferLayerLDR(renderRef, "tests_images/" + testName + "/z_out10.png", "instid", 0, 0)

def runRenderInBG(renderRef, w, h, testName, states_to_del = []):
  while True:
    time.sleep(0.5)
    info = hy.hrRenderHaveUpdate(renderRef);
    if (info.finalUpdate is True):
      for state in states_to_del:
        os.remove(("tests/" + testName + "/change_0000{}.xml").format(state - 1))    
        os.remove(("tests/" + testName + "/statex_0000{}.xml").format(state))    
      hy.hrRenderSaveFrameBufferLDR(renderRef, "tests_images/" + testName + "/z_out.png")
      hy.hrRenderCommand(renderRef, "exitnow", "")
      break

global firstUpdate

def initAndStartOpenGL(renderRef, w, h, testName, states_to_del = []):
  glutInit(sys.argv)
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH)
  glutInitWindowSize(w, h)
  window = glutCreateWindow('test')
  image = np.zeros(w * h, dtype=np.int32)
  glViewport(0, 0, w, h)
  
  stop = False 

  def close(): 
    stop = True 

  def display():
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    time.sleep(0.25)
    info = hy.hrRenderHaveUpdate(renderRef);
    
    if (info.haveUpdateFB):
      if(display.firstUpdate):
        for state in states_to_del:
          os.remove(("tests/" + testName + "/change_0000{}.xml").format(state - 1))    
          os.remove(("tests/" + testName + "/statex_0000{}.xml").format(state))          
        display.firstUpdate = False
        
      hy.hrRenderGetFrameBufferLDR1i(renderRef, w, h, image);
      glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, image.tostring());
      glutSwapBuffers()
    
    if (info.finalUpdate):
      hy.hrRenderSaveFrameBufferLDR(renderRef, "tests_images/" + testName + "/z_out.png")
      hy.hrRenderCommand(renderRef, "exitnow", "")
      glutDestroyWindow(window)
      
  def special(key, x , y):
    if key == GLUT_KEY_RIGHT:
      glutPostRedisplay()
      hy.hrRenderSaveFrameBufferLDR(renderRef, "tests_images/" + testName + "/z_out.png")
    if key == GLUT_KEY_LEFT:
      hy.hrRenderSaveFrameBufferLDR(renderRef, "tests_images/" + testName + "/z_out.png")
      hy.hrRenderCommand(renderRef, "exitnow", "")
      glutDestroyWindow(window)
          
  display.firstUpdate = True

  glutDisplayFunc(display)
  glutIdleFunc(display) 
  glutSpecialFunc(special)
  
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION)
  #glutSetOption(GLUT_ACTION_GLUTMAINLOOP_RETURNS, GLUT_ACTION_CONTINUE_EXECUTION) 

  glutMainLoop()
  #while not stop: 
  #  glutMainLoopEvent()

#print(check_images("test01_render_cubes", 1, 100.0))
