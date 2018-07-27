#include <stdio.h>
#include <windows.h>  
                      
BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{

  switch (fdwReason)     
  {
  case DLL_PROCESS_ATTACH: 
    
    return 1; 

  case DLL_PROCESS_DETACH: 
                             
    break;

  case DLL_THREAD_ATTACH: 
    
    break;

  case DLL_THREAD_DETACH:
  
    break;

  }

  return TRUE;   // this is actually ignored by windows

}