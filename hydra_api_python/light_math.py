# -*- coding: utf-8 -*-
"""
Created on Mon Feb  5 16:00:41 2018

@author: vsan
"""

import numpy as np
from math import cos, sin


DEG_TO_RAD = 0.01745329251; # 3.14159265358979323846 / 180.0;

def identityM4x4():
    mat = np.array([[1.0, 0.0, 0.0, 0.0],
                    [0.0, 1.0, 0.0, 0.0],
                    [0.0, 0.0, 1.0, 0.0],
                    [0.0, 0.0, 0.0, 1.0]], dtype=np.float32)
                        
    return mat
    
def translateM4x4(t):
    assert(t.size == 3)
    
    mat = np.array([[1.0, 0.0, 0.0, t[0]],
                    [0.0, 1.0, 0.0, t[1]],
                    [0.0, 0.0, 1.0, t[2]],
                    [0.0, 0.0, 0.0,  1.0]], dtype=np.float32)
                        
    return mat

def scaleM4x4(t):
    assert(t.size == 3)
    
    mat = np.array([[t[0],  0.0,  0.0, 0.0],
                    [ 0.0, t[1],  0.0, 0.0],
                    [ 0.0,  0.0, t[2], 0.0],
                    [ 0.0,  0.0,  0.0, 1.0]], dtype=np.float32)
                        
    return mat
    
def rotateXM4x4(phi):
    
    mat = np.array([[1.0,       0.0,       0.0, 0.0],
                    [0.0, +cos(phi), +sin(phi), 0.0],
                    [0.0, -sin(phi), +cos(phi), 0.0],
                    [0.0,       0.0,       0.0, 1.0]], dtype=np.float32)
                        
    return mat
    
def rotateYM4x4(phi):
    
    mat = np.array([[+cos(phi), 0.0, -sin(phi), 0.0],
                    [      0.0, 1.0,       0.0, 0.0],
                    [+sin(phi), 0.0, +cos(phi), 0.0],
                    [      0.0, 0.0,       0.0, 1.0]], dtype=np.float32)
                        
    return mat
    
    
def rotateZM4x4(phi):
    
    mat = np.array([[+cos(phi), +sin(phi), 0.0, 0.0],
                    [-sin(phi), +cos(phi), 0.0, 0.0],
                    [      0.0,       0.0, 1.0, 0.0],
                    [      0.0,       0.0, 0.0, 1.0]], dtype=np.float32)
                        
    return mat
    
    
#A = identityM4x4()
#B = rotateYM4x4(45 * DEG_TO_RAD)
#C = np.dot(B, A)


