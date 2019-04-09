#include <iostream>
#include <vector>
//#include <zconf.h>

#include "../hydra_api/HydraAPI.h"
#include "tests.h"

using pugi::xml_node;

///////////////////////////////////////////////////////////////////////////////////////////////////////////// just leave this it as it is :)
#include "../hydra_api/HydraRenderDriverAPI.h"
IHRRenderDriver* CreateDriverRTE(const wchar_t* a_cfg) { return nullptr; }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined WIN32
#include <windows.h> // for SetConsoleCtrlHandler
#else
#include <unistd.h>
#include <signal.h>
#endif


void ErrorCallBack(const wchar_t* message, const wchar_t* callerPlace)
{
  std::wcout << callerPlace << L":\t" << message << std::endl;
}

void InfoCallBack(const wchar_t* message, const wchar_t* callerPlace, HR_SEVERITY_LEVEL a_level)
{
  if (a_level >= HR_SEVERITY_WARNING)
  {
    if (a_level == HR_SEVERITY_WARNING)
      std::wcerr << L"WARNING: " << callerPlace << L": " << message; // << std::endl;
    else      
      std::wcerr << L"ERROR  : " << callerPlace << L": " << message; // << std::endl;
  }
}


void destroy()
{
  std::cout << "call destroy() --> hrDestroy()" << std::endl;
  hrDestroy();
}

#ifdef WIN32
BOOL WINAPI HandlerExit(_In_ DWORD fdwControl)
{
  exit(0);
  return TRUE;
}
#else
bool destroyedBySig = false;
void sig_handler(int signo)
{
  if(destroyedBySig)
    return;
  switch(signo)
  {
    case SIGINT : std::cerr << "\nmain_app, SIGINT";      break;
    case SIGABRT: std::cerr << "\nmain_app, SIGABRT";     break;
    case SIGILL : std::cerr << "\nmain_app, SIGINT";      break;
    case SIGTERM: std::cerr << "\nmain_app, SIGILL";      break;
    case SIGSEGV: std::cerr << "\nmain_app, SIGSEGV";     break;
    case SIGFPE : std::cerr << "\nmain_app, SIGFPE";      break;
    default     : std::cerr << "\nmain_app, SIG_UNKNOWN"; break;
    break;
  }
  std::cerr << " --> hrDestroy()" << std::endl;
  hrDestroy();
  destroyedBySig = true;
}
#endif

extern float g_MSEOutput;
void test02_draw();
void test02_init();

void test_gl32_001_init(void);
void test_gl32_001_draw(void);

void test_gl32_002_init(void);
void test_gl32_002_draw(void);

void _hrDebugPrintVSGF(const wchar_t* a_fileNameIn, const wchar_t* a_fileNameOut);
void _hrConvertOldVSGFMesh(const std::wstring& a_path, const std::wstring& a_newPath);

void _hrCompressMesh(const std::wstring& a_inPath, const std::wstring& a_outPath);
void _hrDecompressMesh(const std::wstring& a_path, const std::wstring& a_newPath);


void demo_01_plane_box();

int main(int argc, const char** argv)
{
  hrInit(L"-copy_textures_to_local_folder 0 -local_data_path 1 -sort_indices 1 -compute_bboxes 1");
  hrInfoCallback(&InfoCallBack);

  hrErrorCallerPlace(L"main");  // for debug needs only

  atexit(&destroy);                           // if application will terminated you have to call hrDestroy to free all connections with hydra.exe
#if defined WIN32
  SetConsoleCtrlHandler(&HandlerExit, TRUE);  // if some one kill console :)
  wchar_t NPath[512];
  GetCurrentDirectoryW(512, NPath);
#ifdef NEED_DIR_CHANGE
  SetCurrentDirectoryW(L"../../main");
#endif
  std::wcout << L"[main]: curr_dir = " << NPath << std::endl;
#else
  std::string workingDir = "../main";
  chdir(workingDir.c_str());
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != nullptr)
    std::cout << "[main]: curr_dir = " << cwd <<std::endl;
  else
    std::cout << "getcwd() error" <<std::endl;
  
  {
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sig_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = SA_RESETHAND;
    sigaction(SIGINT,  &sigIntHandler, NULL);
    sigaction(SIGABRT, &sigIntHandler, NULL);
    sigaction(SIGILL,  &sigIntHandler, NULL);
    sigaction(SIGTERM, &sigIntHandler, NULL);
    sigaction(SIGSEGV, &sigIntHandler, NULL);
    sigaction(SIGFPE,  &sigIntHandler, NULL);
  }
#endif
  
  std::cout << "sizeof(size_t) = " << sizeof(size_t) <<std::endl;
  
  try
  {
    //GEO_TESTS::test_001_mesh_from_memory();
    //demo_01_plane_box();
    //window_main_free_look(L"/home/frol/PROG/HydraAPI/main/tests/test_46", L"opengl1Debug");
    
    //run_all_api_tests();
    //run_all_geo_tests();
    //run_all_mtl_tests();
    //run_all_lgt_tests();
	  //run_all_alg_tests();
	  //run_all_ipp_tests();

    //test40_several_changes();
    //test44_four_lights();

    //hrSceneLibraryOpen(L"/home/frol/temp", HR_WRITE_DISCARD);
    //_hrConvertOldVSGFMesh(L"data/meshes/teapot.vsgf", L"data/meshes/teapot2.vsgf");
    ////_hrDebugPrintVSGF(L"data/meshes/teapot.vsgf",    L"/home/frol/temp/teapot1.txt");
    ////_hrDebugPrintVSGF(L"data/meshes/teapot2.vsgf",   L"/home/frol/temp/teapot2.txt");

    //std::cout << test38_save_mesh_and_delayed_load() << std::endl;
    std::cout << test49_light_geom_disk() << std::endl;
    
    //test46_light_geom_rect();
    //window_main_free_look(L"/home/frol/PROG/HydraAPI/main/tests/test_49", L"opengl1"); // &test02_draw
    //window_main_free_look(L"/home/frol/PROG/HydraAPI/main/tests/test_38", L"opengl1Debug"); // &test02_draw
  
    //_hrCompressMesh  (L"/home/frol/temp/original.vsgf",    L"/home/frol/temp/compressed.vsgfc");
    //_hrDecompressMesh(L"/home/frol/temp/compressed.vsgfc", L"/home/frol/temp/decompressed.vsgf");
    //_hrDebugPrintVSGF(L"/home/frol/temp/original.vsgf",    L"/home/frol/temp/original.txt");
    //_hrDebugPrintVSGF(L"/home/frol/temp/decompressed.vsgf",L"/home/frol/temp/decompressed.txt");

    //test82_proc_texture();
    //test93_proc_tex_recursive();
    //test39_mmlt_or_ibpt();
    //test42_load_library_basic();
    //std::cout << test37_cornell_with_light_different_image_layers() << std::endl;
    
    //std::cout << ALGR_TESTS::test_404_cornell_glossy() << std::endl;
    //std::cout << ALGR_TESTS::test_405_cornell_with_mirror() << std::endl;
    //std::cout << ALGR_TESTS::test_406_env_glass_ball_caustic() << std::endl;
    //std::cout << std::endl << "g_MSEOutput = " << g_MSEOutput << std::endl;
    
    //std::cout << "g_mse = " << g_MSEOutput << std::endl;
    //window_main_free_look(L"tests_f/test_241", L"opengl1Debug");

    //window_main_free_look(L"tests/test_gl32_002_", L"opengl32Deferred", &test_gl32_002_init, &test_gl32_002_draw);
    //window_main_free_look(L"tests/lucy_deferred", L"opengl3Utility", &test_gl32_001_init, &test_gl32_001_draw);
    //window_main_free_look(L"tests/test_gl32_002_", L"opengl3Utility", &test_gl32_002_init, &test_gl32_002_draw);
    //window_main_free_look(L"/home/frol/PROG/HydraAPI/main/tests/test_39", L"opengl1", nullptr, &test02_draw);

    //window_main_free_look(L"tests/zgl1_test_cube", L"opengl32Forward", &test_gl32_001_init, &test_gl32_001_draw);
    //window_main_free_look(L"D:/PROG/HydraCore/hydra_app/tests/test_42", L"opengl1DrawRays");
    //test_gl32_002();
    //window_main_free_look(L"tests/test_gl32_002", L"opengl32Deferred");

    //_hrDebugPrintVSGF(L"D:/temp/TestRenderFromPhil/data/chunk_00022.vsgf",   L"z_mesh_phil.txt");
    //_hrDebugPrintVSGF(L"D:/temp/TestRenderFromSergey/data/chunk_00022.vsgf", L"z_mesh_serg.txt");

	  //test_console_render(L"D:/Downloads/test(1)/test", L"D:/Downloads/test(1)/test/zzz.bmp");
	  //test02_simple_gl1_render(L"opengl1Debug");
    
    //image_p_sandbox();

	  terminate_opengl();
  }
  catch (std::runtime_error& e)
  {
    std::cout << "std::runtime_error: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cout << "unknown exception" << std::endl;
  }

  hrErrorCallerPlace(L"main"); // for debug needs only

  hrDestroy();

  return 0;
}
