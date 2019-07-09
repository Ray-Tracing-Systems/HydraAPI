# -*- coding: utf-8 -*-
"""
Created on Mon Feb  5 17:14:21 2018

@author: vsan
"""
import hydraPy as hy
import numpy as np
import time


def runRenderInBG(renderRef, w, h, path):
  while True:
    time.sleep(0.5)
    info = hy.hrRenderHaveUpdate(renderRef)
    if (info.finalUpdate is True):
      hy.hrRenderSaveFrameBufferLDR(renderRef, path + "/z_out.png")
      hy.hrRenderCommand(renderRef, "exitnow", "")
      break

