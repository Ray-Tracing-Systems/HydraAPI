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
  hrInit(L"-copy_textures_to_local_folder 0 -local_data_path 1 -sort_indices 1");
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
	  run_all_geo_tests();
    run_all_mtl_tests();
    run_all_lgt_tests();
    run_all_api_tests();
    run_all_ipp_tests();

    //test42_load_library_basic();

    //test1000_loadlibrary_and_edit();
    //test1001_loadlibrary_and_add_textures();
    //test1002_get_material_by_name_and_edit();
    //test1003_get_light_by_name_and_edit();
    //test1004_get_camera_by_name_and_edit();

    //test1005_transform_all_instances();
    //test1006_transform_all_instances_origin();
    //test1007_merge_library();
    //test1008_merge_one_texture();
    //test1009_merge_one_material();
    //test1010_merge_one_mesh();
    //test1011_merge_scene();
    //test1012_merge_one_light();
    //test1013_commit_without_render();
    //std::cout << test1014_print_matlib_map() << std::endl;

    //std::cout << test1015_merge_scene_with_remaps() << std::endl;
    //std::cout << test1016_merge_scene_remap_override() << std::endl;

    //std::cout << test39_mesh_from_vsgf() << std::endl;
    //std::cout << test37_cornell_with_light_different_image_layers() << std::endl;
    //std::cout << test77_save_gbuffer_layers() << std::endl;
    //std::cout << test59_cornell_water_mlt() << std::endl;
    //std::cout << test70_area_lights16() << std::endl;
    //std::cout << test78_material_remap_list1() << std::endl;
    //std::cout << test79_material_remap_list2() << std::endl;

    //std::cout << test72_load_library_sigle_teapot_with_opacity() << std::endl;
    //GEO_TESTS::test_005_instancing();
    //MTL_TESTS::test_120_opacity_shadow_matte();
    //LGHT_TESTS::test_211_sky_and_sun_perez();
    
    //PP_TESTS::test305_fbi_from_render();    
    //PP_TESTS::test312_2_post_process_hydra1_whitePointColor();

    //std::cout << "g_mse = " << g_MSEOutput << std::endl;

    //window_main_free_look(L"tests_f/test_241", L"opengl1Debug");

    //std::cout << PP_TESTS::test301_resample() << std::endl;
   
    //test39_mesh_from_vsgf();

    //test100_dummy_hydra_exec();

    //window_main_free_look(L"tests/test_gl32_002_", L"opengl32Deferred", &test_gl32_002_init, &test_gl32_002_draw);
    //window_main_free_look(L"tests/lucy_deferred", L"opengl32Deferred", &test_gl32_001_init, &test_gl32_001_draw);
    //window_main_free_look(L"tests/zgl1_test_cube", L"opengl32Deferred", &test02_init, &test02_draw);
    //window_main_free_look(L"tests/zgl1_test_cube", L"opengl1Debug", &test_gl32_001_init, &test_gl32_001_draw);
    //window_main_free_look(L"tests/zgl1_test_cube", L"opengl32Forward", &test_gl32_001_init, &test_gl32_001_draw);
    //window_main_free_look(L"D:/PROG/HydraCore/hydra_app/tests/test_42", L"opengl1DrawRays");
    //test_gl32_002();
    //window_main_free_look(L"tests/test_gl32_002", L"opengl32Deferred");
    //window_main_free_look(L"tests_f/test_235", L"opengl1");

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

