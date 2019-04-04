#include <iostream>
#include <vector>

#define _USE_MATH_DEFINES
#include "math.h"


#include "../hydra_api/HydraAPI.h"
#include "../hydra_api/HydraXMLHelpers.h"
#include "tests.h"

#include "input.h"
#include "Camera.h"
#include "Timer.h"
//#include "glad/glad.h"
#include "../hydra_api/OpenGLCoreProfileUtils.h"

#if defined(WIN32)
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3dll.lib")
#else
#include <GLFW/glfw3.h>
#endif

using pugi::xml_node;
using pugi::xml_attribute;

using namespace HydraXMLHelpers;

GLFWwindow* g_window = nullptr;
Input g_input;
int   g_width  = 1024;
int   g_height = 1024;
int   g_ssao = 1;
int   g_lightgeo = 0;
static int g_filling = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool   g_captureMouse         = false;
static bool   g_capturedMouseJustNow = false;
static double g_scrollY              = 0.0f;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Camera       g_cam;
HRCameraRef  camRef;
HRRenderRef  renderRef;

static HRSceneInstRef scnRef;


void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity,
                            GLsizei length, const GLchar *message, const void *userParam)
{

  // ignore non-significant error/warning codes
  if(id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 131076) return;

  std::cout << "---------------" << std::endl;
  std::cout << "Debug message (" << id << "): " <<  message << std::endl;

  switch (source)
  {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    default:                              std::cout << "Source: Unknown";  break;
  } std::cout << std::endl;

  switch (type)
  {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    default:                                std::cout << "Source: Unknown"; break;
  } std::cout << std::endl;

  switch (severity)
  {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    default:                             std::cout << "Severity: unknown"; break;
  } std::cout << std::endl;

  std::cout << std::endl;
}



static void Init()
{
  hrErrorCallerPlace(L"Init");

  hrSceneLibraryOpen(g_input.inputLibraryPath.c_str(), HR_OPEN_EXISTING);
  
  HRSceneLibraryInfo scnInfo = hrSceneLibraryInfo();

  if (scnInfo.renderDriversNum == 0) // create some default render driver
  {

  }

  if (scnInfo.camerasNum == 0) // create some default camera
    camRef = hrCameraCreate(L"defaultCam");

  renderRef.id = 0;
  camRef.id    = 0;
  scnRef.id    = 0;

  // TODO: set current camera parameters here
  //
  hrCameraOpen(camRef, HR_OPEN_READ_ONLY);
  {
    xml_node camNode = hrCameraParamNode(camRef);

    ReadFloat3(camNode.child(L"position"), &g_cam.pos.x);
    ReadFloat3(camNode.child(L"look_at"),  &g_cam.lookAt.x);
    ReadFloat3(camNode.child(L"up"),       &g_cam.up.x);
    g_cam.fov = ReadFloat(camNode.child(L"fov"));
  }
  hrCameraClose(camRef);

  if (g_input.enableOpenGL1)
  {
    renderRef = hrRenderCreate(g_input.inputRenderName.c_str()); // L"opengl1"
  }
  else
    renderRef = hrRenderCreate(L"HydraModern"); 
  
  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);
  
    node.append_child(L"width").text()          = g_width;
    node.append_child(L"height").text()         = g_height;
  }
  hrRenderClose(renderRef);


  auto pList = hrRenderGetDeviceList(renderRef);

  while (pList != nullptr)
  {
    std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
    pList = pList->next;
  }

  hrRenderEnableDevice(renderRef, 0, true);

  hrCommit(scnRef, renderRef, camRef);
}

static void Update(float secondsElapsed)
{
  //move position of camera based on WASD keys, and XZ keys for up and down
  if (glfwGetKey(g_window, 'S'))
    g_cam.offsetPosition(secondsElapsed * g_input.camMoveSpeed * -g_cam.forward());
  else if (glfwGetKey(g_window, 'W'))
    g_cam.offsetPosition(secondsElapsed * g_input.camMoveSpeed * g_cam.forward());
  
  if (glfwGetKey(g_window, 'A'))
    g_cam.offsetPosition(secondsElapsed * g_input.camMoveSpeed * -g_cam.right());
  else if (glfwGetKey(g_window, 'D'))
    g_cam.offsetPosition(secondsElapsed * g_input.camMoveSpeed * g_cam.right());
  
  if (glfwGetKey(g_window, 'F'))
    g_cam.offsetPosition(secondsElapsed * g_input.camMoveSpeed * -g_cam.up);
  else if (glfwGetKey(g_window, 'R'))
    g_cam.offsetPosition(secondsElapsed * g_input.camMoveSpeed * g_cam.up);
  
  //rotate camera based on mouse movement
  //
  if (g_captureMouse)
  {
    if(g_capturedMouseJustNow)
      glfwSetCursorPos(g_window, 0, 0);
  
    double mouseX, mouseY;
    glfwGetCursorPos(g_window, &mouseX, &mouseY);
    g_cam.offsetOrientation(g_input.mouseSensitivity * float(mouseY), g_input.mouseSensitivity * float(mouseX));
    glfwSetCursorPos(g_window, 0, 0); //reset the mouse, so it doesn't go out of the window
    g_capturedMouseJustNow = false;
  }
  
  //increase or decrease field of view based on mouse wheel
  //
  const float zoomSensitivity = -0.2f;
  float fieldOfView = g_cam.fov + zoomSensitivity * (float)g_scrollY;
  if(fieldOfView < 5.0f) fieldOfView = 5.0f;
  if(fieldOfView > 130.0f) fieldOfView = 130.0f;
  g_cam.fov = fieldOfView;
  g_scrollY = 0;
 
}


//
static void Draw(void)
{
  hrErrorCallerPlace(L"Draw");

  static GLfloat	rtri  = 25.0f; // Angle For The Triangle ( NEW )
  static GLfloat	rquad = 40.0f;
  static float    g_FPS = 60.0f;
  static int      frameCounter = 0;
#if defined WIN32
  static Timer    timer(true);
#endif
  //const float DEG_TO_RAD = float(M_PI) / 180.0f;

  hrCameraOpen(camRef, HR_OPEN_EXISTING);
  {
    xml_node camNode = hrCameraParamNode(camRef);

    WriteFloat3(camNode.child(L"position"), &g_cam.pos.x);
    WriteFloat3(camNode.child(L"look_at"),  &g_cam.lookAt.x);
    WriteFloat3(camNode.child(L"up"),       &g_cam.up.x);
    WriteFloat( camNode.child(L"fov"),      g_cam.fov); 
  }
  hrCameraClose(camRef);

  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  {
    xml_node settingsNode = hrRenderParamNode(renderRef);

    if(g_input.pathTracingEnabled)
      settingsNode.child(L"method_primary").text() = L"pathtracing";
    else
      settingsNode.child(L"method_primary").text() = L"raytracing";

    settingsNode.force_child(L"draw_solid").text()    = 1;
    settingsNode.force_child(L"draw_wire").text()     = 1;
    settingsNode.force_child(L"draw_normals").text()  = 1;
    settingsNode.force_child(L"draw_tangents").text() = 0;
    settingsNode.force_child(L"draw_axis").text()     = 0;
    settingsNode.force_child(L"draw_length").text()   = 1.0f;
  }
  hrRenderClose(renderRef);


  hrCommit(scnRef, renderRef, camRef);


  //HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);
  //
  //if (info.haveUpdateFB && !g_input.enableOpenGL1)
  //{
  //  glViewport(0, 0, 1024, 768);
  //  std::vector<int32_t> image(1024 * 768);
  //
  //  hrRenderGetFrameBufferLDR1i(renderRef, 1024, 768, &image[0]);
  //
  //  glFlush();
  //  glDisable(GL_TEXTURE_2D);
  //  glDrawPixels(1024, 768, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
  //}

  if (!g_input.enableOpenGL1)
  {
    glViewport(0, 0, g_width, g_height);
    std::vector<int32_t> image(g_width * g_height);
  
    hrRenderGetFrameBufferLDR1i(renderRef, g_width, g_height, &image[0]);
  
    glFlush();
    glDisable(GL_TEXTURE_2D);
    glDrawPixels(g_width, g_height, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
  }

  // count fps 
  //
  const float coeff = 100.0f / fmax(g_FPS, 1.0f);
  rtri += coeff*0.2f;
  rquad -= coeff*0.15f;

#if defined WIN32
  if (frameCounter % 10 == 0)
  {
    std::stringstream out; out.precision(4);
    g_FPS = (10.0f / timer.getElapsed());
    out << "FPS = " << g_FPS;
    glfwSetWindowTitle(g_window, out.str().c_str());
    timer.start();
  }
#else
  glfwSetWindowTitle(g_window, "test");
#endif
  frameCounter++;
}

extern int g_drawBVHIdInput;

// 
static void key(GLFWwindow* window, int k, int s, int action, int mods)
{
  if (action != GLFW_PRESS) 
    return;

  g_input.camMoveSpeed = 7.5f;
  switch (k) {
  case GLFW_KEY_Z:
    break;
  case GLFW_KEY_ESCAPE:
    glfwSetWindowShouldClose(window, GL_TRUE);
    break;
  case GLFW_KEY_UP:
    //view_rotx += 5.0;
    break;
  case GLFW_KEY_DOWN:
    //view_rotx -= 5.0;
    break;
  case GLFW_KEY_LEFT:
    //view_roty += 5.0;
    break;
  case GLFW_KEY_RIGHT:
    //view_roty -= 5.0;
    break;

  case GLFW_KEY_O:
    g_ssao = g_ssao == 1 ? 0 : 1;
    break;

  case GLFW_KEY_I:
    if (g_lightgeo == 2)
      g_lightgeo = 0;
    else
      g_lightgeo++;

  case GLFW_KEY_PAGE_UP:
    g_drawBVHIdInput--;
    if (g_drawBVHIdInput < 0)
      g_drawBVHIdInput = 0;
    std::cout << "draw item id = " << g_drawBVHIdInput << std::endl;
    break;

  case GLFW_KEY_PAGE_DOWN:
    g_drawBVHIdInput++;
    std::cout << "draw item id = " << g_drawBVHIdInput << std::endl;
    break;

  case GLFW_KEY_LEFT_SHIFT:
    g_input.camMoveSpeed = 15.0f;
    break;
  case GLFW_KEY_SPACE:
    if (action == GLFW_PRESS)
    {
      if (g_filling == 0)
      {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        g_filling = 1;
      }
      else
      {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        g_filling = 0;
      }
    }

  case GLFW_KEY_P:
    g_input.pathTracingEnabled = !g_input.pathTracingEnabled;
    g_input.cameraFreeze = !g_input.pathTracingEnabled;
    break;

  default:
    return;
  }


}


// new window size 
static void reshape(GLFWwindow* window, int width, int height)
{
  hrErrorCallerPlace(L"reshape");

  g_width  = width;
  g_height = height;

  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);

    wchar_t temp[256];
    swprintf(temp, sizeof(temp) / sizeof(*temp), L"%d", g_width);
   // wsprintf(temp, L"%d", g_width);
    node.child(L"width").text().set(temp);
    swprintf(temp, sizeof(temp) / sizeof(*temp), L"%d", g_height);
    //wsprintf(temp, L"%d", g_height);
    node.child(L"height").text().set(temp);
  }
  hrRenderClose(renderRef);

  hrCommit(scnRef, renderRef);
}


// records how far the y axis has been scrolled
void OnScroll(GLFWwindow* window, double deltaX, double deltaY) 
{
  g_scrollY += deltaY;
}

void OnMouseButtonClicked(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    g_captureMouse = !g_captureMouse;


  if (g_captureMouse)
  {
    glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    g_capturedMouseJustNow = true;
  }
  else
    glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

}

void OnError(int errorCode, const char* msg) 
{
  throw std::runtime_error(msg);
}


void window_main_free_look(const wchar_t* a_libPath, const wchar_t* a_renderName, InitFuncType a_pInitFunc, DrawFuncType a_pDrawFunc)
{
  g_input.inputLibraryPath = a_libPath;
  g_input.inputRenderName  = a_renderName;

  if (!glfwInit())
  {
    fprintf(stderr, "Failed to initialize GLFW\n");
    exit(EXIT_FAILURE);
  }

  glfwSetErrorCallback(OnError);

  glfwWindowHint(GLFW_DEPTH_BITS, 24);

  if(!wcscmp(a_renderName, L"opengl32Forward") || !wcscmp(a_renderName, L"opengl32Deferred"))
  {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
  }

  g_window = glfwCreateWindow(g_width, g_height, "Hydra GLFW Window", NULL, NULL);
  if (!g_window)
  {
    fprintf(stderr, "Failed to open GLFW window\n");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  // Set callback functions
  glfwSetFramebufferSizeCallback(g_window, reshape);
  glfwSetKeyCallback(g_window, key);
  glfwSetScrollCallback(g_window, OnScroll);
  glfwSetMouseButtonCallback(g_window, OnMouseButtonClicked);

  glfwMakeContextCurrent(g_window);
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  //glfwSwapInterval(0);

/*
 * comment this for better performance when not debugging
 * */
  if(!wcscmp(a_renderName, L"opengl32Forward") || !wcscmp(a_renderName, L"opengl32Deferred") || !wcscmp(a_renderName, L"opengl3Utility"))
  {
    GLint flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
      std::cout << "Initializing debug output" << std::endl;
      glEnable(GL_DEBUG_OUTPUT);
      glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
      glDebugMessageCallback(glDebugOutput, nullptr);
      glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
  }
/*
 *
 * */

  glfwGetFramebufferSize(g_window, &g_width, &g_height);
  glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  reshape(g_window, g_width, g_height);

  // Parse command-line options
  if (a_pInitFunc != nullptr)
    (*a_pInitFunc)();
  else
    Init();

  // Main loop
  //
  double lastTime = glfwGetTime();
  while (!glfwWindowShouldClose(g_window))
  {
    glfwPollEvents();

    double thisTime = glfwGetTime();
    const float diffTime = float(thisTime - lastTime);
    lastTime = thisTime;

    Update(diffTime);

    if (a_pDrawFunc != nullptr)
      (*a_pDrawFunc)();
    else
      Draw();

    // Swap buffers
    glfwSwapBuffers(g_window);

    //exit program if escape key is pressed
    if (glfwGetKey(g_window, GLFW_KEY_ESCAPE))
      glfwSetWindowShouldClose(g_window, GL_TRUE);
  }

  // Terminate GLFW
  glfwTerminate();

}

