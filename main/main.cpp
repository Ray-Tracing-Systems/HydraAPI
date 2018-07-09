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
      std::wcout << L"WARNING: " << callerPlace << L": " << message << std::endl;
    else      
      std::wcout << L"ERROR  : " << callerPlace << L": " << message << std::endl;
  }
}


void destroy()
{
  hrDestroy();
}

#if defined WIN32
BOOL WINAPI HandlerExit(_In_ DWORD fdwControl)
{
  exit(0);
  return TRUE;
}
#endif

extern float g_MSEOutput;
void test02_draw(void);
void test02_init();

void test_gl32_001_init(void);
void test_gl32_001_draw(void);

void test_gl32_002_init(void);
void test_gl32_002_draw(void);

void _hrDebugPrintVSGF(const wchar_t* a_fileNameIn, const wchar_t* a_fileNameOut);

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

  std::cout << sizeof(size_t) <<std::endl;

#endif

  try
  {
    // run_all_api_tests();
	  // run_all_geo_tests();
    // run_all_lgt_tests();
    // run_all_mtl_tests();
    // run_all_ipp_tests();
  
    //test77_save_gbuffer_layers();
    //test38_licence_plate();
    test39_mesh_from_vsgf();
    //test42_load_library_basic();
    //std::cout << test98_motion_blur() << std::endl;
    
    //std::cout << PP_TESTS::test303_median_in_place() << std::endl;
    
    //test38_licence_plate();
    //std::cout << test39_mesh_from_vsgf() << std::endl;

    //std::cout << test37_cornell_with_light_different_image_layers() << std::endl;
    //std::cout << test77_save_gbuffer_layers() << std::endl;
    //std::cout << test70_area_lights16() << std::endl;
    //std::cout << test79_material_remap_list2() << std::endl;
    //std::cout << test77_save_gbuffer_layers() << std::endl;

    //std::cout << test97_camera_from_matrices() << std::endl;

    //test38_licence_plate();
    //test82_proc_texture();

    //std::cout << test72_load_library_sigle_teapot_with_opacity() << std::endl;
    //GEO_TESTS::test_003_compute_normals();
    //std::cout << MTL_TESTS::test_163_diffuse_texture_recommended_res() << std::endl;
    //std::cout << MTL_TESTS::test_168_diffuse_texture_recommended_res2() << std::endl;

    //std::cout <<  MTL_TESTS::test_161_simple_displacement() << std::endl;
    //std::cout <<  MTL_TESTS::test_164_simple_displacement_proctex() << std::endl;
    //std::cout <<  MTL_TESTS::test_165_simple_displacement_mesh()  << std::endl;
    //MTL_TESTS::test_166_displace_by_noise();
    //std::cout <<  MTL_TESTS::test_167_subdiv()  << std::endl;

    //window_main_free_look(L"tests_f/test_167", L"opengl1Debug");

    //MTL_TESTS::test_162_shadow_matte_back1();
    //MTL_TESTS::test_154_proc_checker_precomp();
    //LGHT_TESTS::test_223_rotated_area_light();
    
    //PP_TESTS::test305_fbi_from_render();    
    //PP_TESTS::test312_2_post_process_hydra1_whitePointColor();

    //std::cout << "g_mse = " << g_MSEOutput << std::endl;

    //window_main_free_look(L"tests_f/test_241", L"opengl1Debug");

    //std::cout << PP_TESTS::test301_resample() << std::endl;

    //test100_dummy_hydra_exec();

    //window_main_free_look(L"tests/test_gl32_002_", L"opengl32Deferred", &test_gl32_002_init, &test_gl32_002_draw);
    //window_main_free_look(L"tests/lucy_deferred", L"opengl32Deferred", &test_gl32_001_init, &test_gl32_001_draw);
    //window_main_free_look(L"tests/lucy_deferred", L"opengl3Utility", &test_gl32_001_init, &test_gl32_001_draw);
    //window_main_free_look(L"tests/test_gl32_002_", L"opengl3Utility", &test_gl32_002_init, &test_gl32_002_draw);
    //window_main_free_look(L"C:/[Hydra]/pluginFiles/scenelib", L"opengl1Debug", nullptr, &test02_draw);
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

