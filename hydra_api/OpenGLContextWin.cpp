#include <cstdint>
#include "windows.h"

#ifndef WGL_CONTEXT_DEBUG_BIT_ARB
#define WGL_CONTEXT_DEBUG_BIT_ARB      0x0001
#endif
#ifndef WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002
#endif
#ifndef WGL_CONTEXT_MAJOR_VERSION_ARB
#define WGL_CONTEXT_MAJOR_VERSION_ARB  0x2091
#endif
#ifndef WGL_CONTEXT_MINOR_VERSION_ARB
#define WGL_CONTEXT_MINOR_VERSION_ARB  0x2092
#endif
#ifndef WGL_CONTEXT_LAYER_PLANE_ARB
#define WGL_CONTEXT_LAYER_PLANE_ARB    0x2093
#endif
#ifndef WGL_CONTEXT_FLAGS_ARB
#define WGL_CONTEXT_FLAGS_ARB          0x2094
#endif
#ifndef ERROR_INVALID_VERSION_ARB
#define ERROR_INVALID_VERSION_ARB      0x2095
#endif

static HWND		    g_hWnd      = NULL;	  // Holds Our Window Handle
static HINSTANCE	g_hInstance = NULL;		// Holds The Instance Of The Application
static HDC			  g_hDC       = NULL;	  // Private GDI Device Context
static HGLRC		  g_hRC       = NULL;	  // Permanent Rendering Context

typedef HGLRC(WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROCTEMP) (HDC hDC, HGLRC hShareContext, const int *attribList);


LRESULT CALLBACK WndProc(HWND	hWnd,			    // Handle For Window
                        UINT	uMsg,			    // Message For Window
                        WPARAM	wParam,			// Additional Message Information
                        LPARAM	lParam)			// Additional Message Information
{


  // Pass All Unhandled Messages To DefWindowProc
  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


void HydraDestroyHiddenWindow()
{
	if (g_hRC)											// Do We Have A Rendering Context?
	{
		wglMakeCurrent(NULL, NULL);		// Release The DC And RC Contexts
		wglDeleteContext(g_hRC);		  // Delete The RC
		g_hRC = NULL;									// Set RC To NULL
	}

	if (g_hDC)
	{
		ReleaseDC(g_hWnd, g_hDC);		  // Release The DC
		g_hDC = NULL;									// Set DC To NULL
	}

	if (g_hWnd)
	{
		DestroyWindow(g_hWnd);				// Are We Able To Destroy The Window?
		g_hWnd = NULL;								// Set hWnd To NULL
	}

	if (g_hInstance)
	{
		UnregisterClass(L"HydraHiddenOpenGLWindow", g_hInstance);			// Unregister Class
		g_hInstance = NULL;								                            // Set hInstance To NULL
	}
}


bool HydraCreateHiddenWindow(int width, int height, int a_major, int a_minor, int a_flags)
{
	int32_t attribList[] =
	{
	    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
      WGL_CONTEXT_MINOR_VERSION_ARB, 0,
      WGL_CONTEXT_FLAGS_ARB, 0,
      0
	};

	static PIXELFORMATDESCRIPTOR pfd=  // pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),   // Size Of This Pixel Format Descriptor
		1,											         // Version Number
		PFD_DRAW_TO_WINDOW |						 // Format Must Support Window
		PFD_SUPPORT_OPENGL |						 // Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							   // Must Support Double Buffering
		PFD_TYPE_RGBA,								   // Request An RGBA Format
		32,											         // Select Our Color Depth
		0, 0, 0, 0, 0, 0,							   // Color Bits Ignored
		0,											         // No Alpha Buffer
		0,											         // Shift Bit Ignored
		0,											         // No Accumulation Buffer
		0, 0, 0, 0,									     // Accumulation Bits Ignored
		24,											         // 24Bit Z-Buffer (Depth Buffer)  
		0,											         // No Stencil Buffer
		0,											         // No Auxiliary Buffer
		PFD_MAIN_PLANE,								   // Main Drawing Layer
		0,											         // Reserved
		0, 0, 0										       // Layer Masks Ignored
	};

	uint32_t	PixelFormat;		         // Holds The Results After Searching For A Match
	WNDCLASS	wc;						           // Windows Class Structure
	DWORD		dwExStyle;				         // Window Extended Style
	DWORD		dwStyle;				           // Window Style
	RECT		WindowRect;				         // Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			     // Set Left Value To 0
	WindowRect.right=(long)width;		   // Set Right Value To Requested Width
	WindowRect.top=(long)0;				     // Set Top Value To 0
	WindowRect.bottom=(long)height;		 // Set Bottom Value To Requested Height

	g_hInstance			  = GetModuleHandle(NULL);				       // Grab An Instance For Our Window
	wc.style			    = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	 // Redraw On Size, And Own DC For Window.
  wc.lpfnWndProc    = (WNDPROC)WndProc;					           // WndProc Handles Messages
	wc.cbClsExtra		  = 0;									                 // No Extra Window Data
	wc.cbWndExtra		  = 0;									                 // No Extra Window Data
	wc.hInstance		  = g_hInstance;							           // Set The Instance
	wc.hIcon			    = NULL; //LoadIcon(NULL, IDI_WINLOGO); // Load The Default Icon
	wc.hCursor			  = NULL; //LoadCursor(NULL, IDC_ARROW); // Load The Arrow Pointer
	wc.hbrBackground	= NULL;									               // No Background Required For GL
	wc.lpszMenuName		= NULL;									               // We Don't Want A Menu
	wc.lpszClassName	= L"HydraHiddenOpenGLWindow";		       // Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
		return false;											

  dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
  dwStyle = WS_OVERLAPPEDWINDOW;							       // Windows Style

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(g_hWnd = CreateWindowEx(dwExStyle,						// Extended Style For The Window
								L"HydraHiddenOpenGLWindow",						// Class Name
								L"HydraHiddenOpenGLWindow",						// Window Title
								dwStyle |							                // Defined Window Style
								WS_CLIPSIBLINGS |					            // Required Window Style
								WS_CLIPCHILDREN,					            // Required Window Style
								0, 0,								                  // Window Position
								WindowRect.right-WindowRect.left,	    // Calculate Window Width
								WindowRect.bottom-WindowRect.top,	    // Calculate Window Height
								NULL,								                  // No Parent Window
								NULL,								                  // No Menu
								g_hInstance,						              // Instance
								NULL)))								                // Dont Pass Anything To WM_CREATE
	{
		return false;
	}
	
	if (!(g_hDC = GetDC(g_hWnd)))					// Did We Get A Device Context?
	{
    HydraDestroyHiddenWindow();
    return false;
	}

	if (!(PixelFormat = ChoosePixelFormat(g_hDC, &pfd)))	// Did Windows Find A Matching Pixel Format?
	{
    HydraDestroyHiddenWindow();
    return false;
	}

	if(!SetPixelFormat(g_hDC, PixelFormat, &pfd))	// Are We Able To Set The Pixel Format?
	{
    HydraDestroyHiddenWindow();
    return false;
	}

	if (!(g_hRC=wglCreateContext(g_hDC)))			// Are We Able To Get A Rendering Context?
	{
    HydraDestroyHiddenWindow();
    return false;
	}

	if(!wglMakeCurrent(g_hDC, g_hRC))				// Try To Activate The Rendering Context
	{
    HydraDestroyHiddenWindow();
    return false;
	}

	if (a_major >= 3)
	{
		PFNWGLCREATECONTEXTATTRIBSARBPROCTEMP wglCreateContextAttribsARBTemp = NULL;
		HGLRC hRCTemp = NULL;

		int32_t attribList[] =
		{
		  WGL_CONTEXT_MAJOR_VERSION_ARB, 1,
		  WGL_CONTEXT_MINOR_VERSION_ARB, 0,
		  WGL_CONTEXT_FLAGS_ARB,         0,
		  0
		};

		attribList[1] = a_major;
		attribList[3] = a_minor;
		attribList[5] = a_flags;

		if (!(wglCreateContextAttribsARBTemp = (PFNWGLCREATECONTEXTATTRIBSARBPROCTEMP)wglGetProcAddress("wglCreateContextAttribsARB")))
		{
      HydraDestroyHiddenWindow();
      return false;
		}

		if (!(hRCTemp = wglCreateContextAttribsARBTemp(g_hDC, 0, attribList)))
		{
      HydraDestroyHiddenWindow();
      return false;
		}

		if(!wglMakeCurrent(NULL, NULL))
		{
			wglDeleteContext(hRCTemp);
      HydraDestroyHiddenWindow();
      return false;
		}

		if (!wglDeleteContext(g_hRC))
		{
			wglDeleteContext(hRCTemp);
      HydraDestroyHiddenWindow();
      return false;
		}

		g_hRC = hRCTemp;

		if(!wglMakeCurrent(g_hDC, g_hRC))
		{
      HydraDestroyHiddenWindow();
      return false;
		}
	}

	ShowWindow(g_hWnd, SW_HIDE);					// Hide The Window
	//SetForegroundWindow(g_hWnd);		   // Slightly Higher Priority
	//SetFocus(g_hWnd);								   // Sets Keyboard Focus To The Window

	return true;								          // Success
}