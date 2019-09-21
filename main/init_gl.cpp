//
// Created by frol on 8/7/19.
//

#include <iostream>
#include <cstdio>


#include <GLFW/glfw3.h>
#if defined(WIN32)
#pragma comment(lib, "glfw3dll.lib")
#else
#endif

int g_width  = 1024;
int g_height = 1024;
GLFWwindow* g_window = nullptr;

void initGLIfNeeded(int a_width, int a_height, const char* a_name)
{
  static bool firstCall = true;
  
  if (firstCall)
  {
    g_width  = a_width;
    g_height = a_height;
    
    if (!glfwInit())
    {
      fprintf(stderr, "Failed to initialize GLFW\n");
      exit(EXIT_FAILURE);
    }
    
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    
    g_window = glfwCreateWindow(g_width, g_height, a_name, NULL, NULL);
    if (!g_window)
    {
      fprintf(stderr, "Failed to open GLFW window\n");
      glfwTerminate();
      exit(EXIT_FAILURE);
    }
    
    // Set callback functions
    //glfwSetFramebufferSizeCallback(g_window, reshape);
    //glfwSetKeyCallback(g_window, key);
    
    glfwMakeContextCurrent(g_window);
    //gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);
    
    glfwGetFramebufferSize(g_window, &g_width, &g_height);
    //reshape(g_window, g_width, g_height);
    
    firstCall = false;
  }
  else
  {
    g_width  = a_width;
    g_height = a_height;
    
    glfwSetWindowSize(g_window, g_width, g_height);
    glfwSetWindowTitle(g_window, a_name);
  }
}

void terminateGL()
{
  glfwTerminate();
}
