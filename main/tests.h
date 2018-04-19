#ifndef TESTS_H
#define TESTS_H

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include "../hydra_api/HydraAPI.h"
#include "../hydra_api/HydraXMLVerify.h"
#include "mesh_utils.h"

void ErrorCallBack(const wchar_t* message, const wchar_t* callerPlace);
void InfoCallBack(const wchar_t* message, const wchar_t* callerPlace, HR_SEVERITY_LEVEL a_level);

using pugi::xml_node;

inline xml_node force_child(xml_node a_parent, const wchar_t* a_name) ///< helper function
{
  xml_node child = a_parent.child(a_name);
  if (child != nullptr)
    return child;
  else
    return a_parent.append_child(a_name);
}

inline float rnd(float s, float e)
{
  float t = (float)rand() / (float)RAND_MAX;
  return s + t*(e - s);
}


bool check_images(const char* a_path, const int a_numImages = 1, const float a_mse = 10.0f);
void initGLIfNeeded();





namespace TEST_UTILS
{
  bool FileExists(const char* a_fileName);

  //images and textures
  void show_me_texture_ldr(const std::string& a_inFleName, const std::string& a_outFleName);
  void show_me_texture_hdr(const std::string& a_inFleName, const std::string& a_outFleName);
  void CreateTestBigTexturesFilesIfNeeded();
  HRTextureNodeRef AddRandomTextureFromMemory(size_t& memTotal);
  HRTextureNodeRef CreateRandomStrippedTextureFromMemory(size_t& a_byteSize);
  std::vector<unsigned int> CreateStripedImageData(unsigned int* a_colors, int a_stripsNum, int w, int h);
  void CreateStripedImageFile(const char* a_fileName, unsigned int* a_colors, int a_stripsNum, int w, int h);
  void procTexCheckerHDR(float* a_buffer, int w, int h, void* a_repeat);
  void procTexCheckerLDR(unsigned char* a_buffer, int w, int h, void* a_repeat);

  //geometry
  std::vector<HRMeshRef> CreateRandomMeshesArray(int a_size);
  HRMeshRef HRMeshFromSimpleMesh(const wchar_t* a_name, const SimpleMesh& a_mesh, int a_matId);

  //render
  HRRenderRef CreateBasicTestRenderPT(int deviceId, int w, int h, int minRays, int maxRays, const wchar_t* a_drvName = L"HydraModern");
  HRRenderRef CreateBasicTestRenderPTNoCaust(int deviceId, int w, int h, int minRays, int maxRays);
 
}

using DrawFuncType = void (*)();
using InitFuncType = void (*)();

void test02_simple_gl1_render(const wchar_t* a_driverName);
void window_main_free_look(const wchar_t* a_libPath, const wchar_t* a_renderName = L"opengl1", InitFuncType a_pInitFunc = nullptr, DrawFuncType a_pDrawFunc = nullptr);
void test_console_render(const wchar_t* a_libPath, const wchar_t* a_savePath);

void test_device_list();

bool check_all_duplicates(const std::wstring& a_fileName);

bool test01_materials_add();
bool test02_materials_changes_open_mode();
bool test03_lights_add();
bool test05_instances_write_discard();
bool test06_instances_open_existent();
bool test07_camera_add();
bool test08_camera_add_change();
bool test09_render_ogl();

bool test10_render_ogl_cube();
bool test11_render_ogl_some_figures();
bool test12_render_ogl_100_random_figures();
bool test13_render_ogl_some_figures_diff_mats_prom_ptr();
bool test14_bad_material_indices();

bool test15_main_scene_and_mat_editor();
bool test16_texture_add_change();
bool test17_falloff();
bool test18_camera_move();

bool test19_material_change();
bool test20_mesh_change();
bool test21_add_same_textures_from_file();
bool test22_can_not_load_texture();
bool test23_texture_from_memory();
bool test24_many_textures_big_data();
bool test25_many_textures_big_data();
bool test26_many_textures_big_data();
bool test27_many_textures_big_data_from_mem();
bool test28_compute_normals();
bool test29_many_textures_and_meshes();
bool test30_many_textures_and_meshes();
bool test31_procedural_texture_LDR(); //doesn't work because opengl1 render doesn't support utilityPrepass
bool test32_procedural_texture_HDR(); //doesn't work because opengl1 render doesn't support utilityPrepass
bool test33_update_from_file();
bool test34_delayed_textures_does_not_exists();
bool test35_cornell_with_light();
bool test36_update_from_memory();
bool test37_cornell_with_light_different_image_layers();
bool test38_test_for_mlt();
bool test39_mesh_from_vsgf();
bool test40_several_changes();

bool test41_load_library_basic();
bool test42_load_library_basic(); 

bool test43_test_direct_light();
bool test44_four_lights();
bool test45_mesh_from_vsgf_opengl_bug_teapot();

bool test46_light_geom_rect();
bool test47_light_geom_disk();
bool test48_light_geom_sphere();
bool test49_light_geom_disk();
bool test50_open_library_several_times();
bool test51_instance_many_trees_and_opacity();
bool test52_instance_perf_test();
bool test53_crysponza_perf();
bool test54_portalsroom_perf();
bool test55_clear_scene();
bool test56_mesh_change_open_existing();
bool test57_single_instance();
bool test58_crysponza_and_opacity1_perf();
bool test59_cornell_water_mlt();
bool test60_debug_print();
bool test61_cornell_with_light_near_wall_and_glossy_wall();

//bool test62_bad_textures();
bool test63_cornell_with_caustic_from_torus();
bool test64_several_changes_light_area();
bool test65_several_changes_light_rect();

bool test66_fast_render_no_final_update();
bool test67_fast_empty_scene();
bool test68_scene_library_file_info();
bool test69_pause_and_resume();                      //#TODO: implement image save an load;
bool test70_area_lights16();
bool test71_out_of_memory();
bool test72_load_library_single_teapot_with_opacity();
bool test73_big_resolution();
bool test74_frame_buffer_line();
bool test75_repeated_render();
bool test76_empty_mesh();
bool test77_save_gbuffer_layers();
bool test78_material_remap_list1();
bool test79_material_remap_list2();
bool test80_lt_rect_image();
bool test81_custom_attributes();
bool test82_proc_texture();
bool test83_proc_texture2();
bool test84_proc_texture2();

bool test90_check_xml_fail_lights();
bool test91_check_xml_fail_lights();
bool test92_check_xml_fail_lights();

bool test93_check_xml_fail_materials();
bool test94_check_xml_fail_camera();
bool test95_check_xml_fail_render();
bool test96_save_temp_renders();

bool test98_test_split_clipping();

bool test100_dummy_hydra_exec();

namespace GEO_TESTS
{
  bool test_001_mesh_from_memory();
  bool test_002_mesh_from_vsgf();
  bool test_003_compute_normals(); //bug - normals computation 
  bool test_004_dof();
  bool test_005_instancing();
}

namespace MTL_TESTS
{
  bool test_101_diffuse_lambert();
  bool test_102_diffuse_orennayar();
  bool test_103_diffuse_texture();

  bool test_104_reflect_phong();
  bool test_105_reflect_microfacet();
  bool test_106_reflect_fresnel_ior();
  bool test_107_reflect_extrusion();
  bool test_108_reflect_texture();
  bool test_109_reflect_glossiness_texture();

  bool test_110_texture_sampler();
  bool test_111_glossiness_texture_sampler();

  bool test_112_transparency();
  bool test_113_transparency_ior();
  bool test_114_transparency_fog();
  bool test_115_transparency_fog_mult();
  bool test_116_transparency_thin();
  bool test_117_transparency_texture();
  bool test_118_transparency_glossiness_texture();

  bool test_119_opacity_texture();
  bool test_120_opacity_shadow_matte(); // need to be redone with new material?

  bool test_121_translucency();
  bool test_122_translucency_texture(); // bug - samplers are ignored

  bool test_123_emission();
  bool test_124_emission_texture();
  bool test_125_emission_cast_gi();

  bool test_126_bump_amount();
  bool test_127_normal_map_height(); 
  bool test_128_bump_radius();
  bool test_129_parallax();            // #NOT_SUPPORTED_CURRENTLY; parallax is broken;
  bool test_130_bump_invert_normalY();

  bool test_131_blend_simple();
  bool test_132_blend_recursive();
  
  bool test_133_emissive_and_diffuse();
  bool test_134_diff_refl_transp();
  
  bool test_135_opacity_metal();        // not smooth backface
  bool test_136_opacity_glass();        // transparency work as thin;
  bool test_137_opacity_emission();     // #NOT_SUPPORTED_CURRENTLY;  falloff does not work in emission

  bool test_138_translucency_and_diffuse();
  bool test_139_glass_and_bump();
  bool test_140_blend_emission();       // 1) I suggest that the opacity from first mat always cut all the blends. 
                                        // 2) Not visible emission through glass.
  bool test_141_opacity_smooth();
  bool test_142_blend_normalmap_heightmap();

  bool test_150_gloss_mirror_cos_div();
  bool test_151_gloss_mirror_cos_div2();
  bool test_152_texture_color_replace_mode();
  bool test_153_opacity_shadow_matte_opacity();

  bool test_154_proc_checker_precomp();
  bool test_155_proc_checker_HDR_precomp();
  bool test_156_proc_checker_precomp_update();
  bool test_157_proc_checker_precomp_remap();
}

namespace LGHT_TESTS
{
  bool test_200_spot();
  bool test_201_sphere();
  bool test_202_sky_color();
  bool test_203_sky_hdr();
  bool test_204_sky_hdr_rotate();
  bool test_205_sky_and_directional_sun();

  bool test_206_ies1();
  bool test_207_ies2();
  bool test_208_ies3();

  bool test_209_skyportal();
  bool test_210_skyportal_hdr();
  bool test_211_sky_and_sun_perez();
  bool test_212_skyportal_sun();

  bool test_213_point_omni();
  bool test_214_sky_ldr();

  bool test_215_light_scale_intensity();
  bool test_216_ies4();

  bool test_217_cylinder();
  bool test_218_cylinder2();
  bool test_219_cylinder_tex();
  bool test_220_cylinder_tex2();
  bool test_221_cylinder_tex3();
  bool test_222_cylinder_with_end_face();

  bool test_223_rotated_area_light();
  bool test_224_rotated_area_light2();
  bool test_225_point_spot_simple();
  bool test_226_area_spot_simple();
  bool test_227_point_spot_glossy_wall();
  bool test_228_point_ies_for_bpt();
  bool test_229_point_ies_for_bpt();
  bool test_230_area_ies_for_bpt();
  bool test_231_direct_soft_shadow();

  bool test_232_point_area_ies();
  bool test_233_light_group_point_area_ies();
  bool test_234_light_group_light_inst_cust_params();
  bool test_235_stadium();
  bool test_236_light_group_point_area_ies2();
  bool test_237_cubemap_ldr();

  bool test_238_mesh_light_one_triangle();
  bool test_239_mesh_light_two_triangle();
  bool test_240_mesh_light_torus();
  bool test_241_mesh_light_torus_texture_ldr();
  bool test_242_mesh_light_torus_texture_hdr();
  bool test_243_mesh_light_do_not_sample_me();
  bool test_244_do_not_sample_me();
  bool test_245_cylinder_tex_nearest();
}

namespace PP_TESTS
{
  bool test301_resample();
  bool test302_median();
  bool test303_median_in_place();
  bool test304_obsolete_tone_mapping();
  bool test305_fbi_from_render();
  bool test306_post_process_hydra1_exposure05();
  bool test307_post_process_hydra1_exposure2();
  bool test308_post_process_hydra1_compress();
  bool test309_post_process_hydra1_contrast();
  bool test310_post_process_hydra1_desaturation();
  bool test311_post_process_hydra1_saturation();
  bool test312_post_process_hydra1_whiteBalance();
  bool test312_2_post_process_hydra1_whitePointColor();
  bool test313_post_process_hydra1_uniformContrast();
  bool test314_post_process_hydra1_normalize();
  bool test315_post_process_hydra1_vignette();
  bool test316_post_process_hydra1_chromAberr();
  bool test317_post_process_hydra1_sharpness();
  bool test318_post_process_hydra1_ECCSWUNSVC();
  bool test319_post_process_hydra1_diffStars();
};

//These tests need some scene library to exist in their respective folders
bool test1000_loadlibrary_and_edit();
bool test1001_loadlibrary_and_add_textures();
bool test1002_get_material_by_name_and_edit();
bool test1003_get_light_by_name_and_edit();
bool test1004_get_camera_by_name_and_edit();
bool test1005_transform_all_instances();
bool test1006_transform_all_instances_origin();


bool test1007_merge_library();      // run MTL_TESTS::test_131_blend_simple() first
bool test1008_merge_one_texture();  // run MTL_TESTS::test_131_blend_simple() first
bool test1009_merge_one_material(); // run MTL_TESTS::test_131_blend_simple() first
bool test1010_merge_one_mesh();     // run GEO_TESTS::test_005_instancing() and GEO_TESTS::test_002_mesh_from_vsgf() first
bool test1011_merge_scene();        // run MTL_TESTS::test_131_blend_simple() first
bool test1012_merge_one_light();    // run LGHT_TESTS::test_221_cylinder_tex3();


bool test1013_commit_without_render(); //needs scene library with at least 1 light and 2 materials
bool test1014_print_matlib_map();

bool test1015_merge_scene_with_remaps();
bool test1016_merge_scene_remap_override();


void run_all_api_tests(const int startTestId = 0);
void run_all_geo_tests();
void run_all_mtl_tests(int a_start = 0);
void run_all_lgt_tests(int a_start = 0);
void run_all_ipp_tests(int a_start = 0);
void terminate_opengl();

static const int CURR_RENDER_DEVICE = 0;

//void image_p_sandbox();

#endif
