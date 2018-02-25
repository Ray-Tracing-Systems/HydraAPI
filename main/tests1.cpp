#include "tests.h"
#include <math.h>
#include <iomanip>
#include <fstream>
#include <set>

#if defined(WIN32)
  #include <GLFW/glfw3.h>
  #pragma comment(lib, "glfw3dll.lib")
#else
  #include <GLFW/glfw3.h>
#endif


bool check_duplacates(pugi::xml_node a_lib) // check that each object with some id have only one xml node.
{
  std::set<int32_t> idSet; 

  for (pugi::xml_node child = a_lib.first_child(); child != nullptr; child = child.next_sibling())
  {
    int32_t objId = child.attribute(L"id").as_int();
    if (idSet.find(objId) == idSet.end())
      idSet.insert(objId);
    else
      return false;
  }

  return true;
}

bool check_all_duplicates(const std::wstring& a_fileName)
{
  pugi::xml_document doc;
  doc.load_file(a_fileName.c_str());
  if (doc == nullptr)
    return false;

  pugi::xml_node lib0 = doc.child(L"textures_lib");
  pugi::xml_node lib1 = doc.child(L"materials_lib");
  pugi::xml_node lib2 = doc.child(L"lights_lib");
  pugi::xml_node lib3 = doc.child(L"geometry_lib");
  pugi::xml_node lib4 = doc.child(L"cam_lib");
  pugi::xml_node lib5 = doc.child(L"render_lib");
  pugi::xml_node lib6 = doc.child(L"scenes");

  if (lib0 == nullptr || lib1 == nullptr || lib2 == nullptr || lib3 == nullptr)
    return false;

  if (lib4 == nullptr || lib5 == nullptr || lib6 == nullptr)
    return false;

  return check_duplacates(lib0) && check_duplacates(lib1) && check_duplacates(lib2) && 
         check_duplacates(lib3) && check_duplacates(lib4) && check_duplacates(lib5) && check_duplacates(lib6);
}

bool check_test_01(const std::wstring a_fileName)
{
  pugi::xml_document doc;
  doc.load_file(a_fileName.c_str());

  pugi::xml_node libMat = doc.child(L"materials_lib");

  pugi::xml_node m1 = libMat.find_child_by_attribute(L"id", L"0");
  pugi::xml_node m2 = libMat.find_child_by_attribute(L"id", L"1");

  bool name1_ok      = std::wstring(m1.attribute(L"name").value()) == L"MyRedMaterialName";
  bool name2_ok      = std::wstring(m2.attribute(L"name").value()) == L"MyGreenMaterialName";
  bool diffColor1_ok = (std::wstring(m1.child(L"diffuse").child(L"color").text().get()) == L"0.35 0.02 0.02");
  bool diffColor2_ok = (std::wstring(m2.child(L"diffuse").child(L"color").text().get()) == L"0.1 0.9 0.1");
  bool reflBrdf_ok   = std::wstring(m1.child(L"reflectivity").attribute(L"brdf_type").value()) == L"phong";

  return name1_ok && name2_ok && diffColor1_ok && diffColor2_ok && reflBrdf_ok;
}


bool test01_materials_add()
{
  hrSceneLibraryOpen(L"tests/test_01", HR_WRITE_DISCARD);

  // material definition
  //
  HRMaterialRef mat  = hrMaterialCreate(L"MyRedMaterialName"); // create new HydraMaterial
  HRMaterialRef mat2 = hrMaterialCreate(L"MyGreenMaterialName"); // create new material

  hrMaterialOpen(mat, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat);

    xml_node diff = matNode.append_child(L"diffuse");
    xml_node refl = matNode.append_child(L"reflectivity");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.35 0.02 0.02");
    diff.append_child(L"roughness").text().set(L"0");

    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").text().set(L"0.1 0.1 0.1");
    refl.append_child(L"resnel_IOR").text().set(L"1.5");
    refl.append_child(L"glosiness").text().set(L"0.75");
  }
  hrMaterialClose(mat); // HRCheckErrorsMacro; ... 


  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.1 0.9 0.1");
    diff.append_child(L"roughness").text().set(L"0");
  }
  hrMaterialClose(mat2);

  hrFlush();

  return check_test_01(L"tests/test_01/statex_00001.xml") && check_test_01(L"tests/test_01/change_00000.xml");
}

bool check_test_02(const std::wstring a_fileName)
{
  pugi::xml_document doc;
  doc.load_file(a_fileName.c_str());

  pugi::xml_node libMat = doc.child(L"materials_lib");

  pugi::xml_node m1 = libMat.find_child_by_attribute(L"id", L"0");
  pugi::xml_node m2 = libMat.find_child_by_attribute(L"id", L"1");

  bool name1_ok = std::wstring(m1.attribute(L"name").value()) == L"MyRedMaterialName";
  bool name2_ok = std::wstring(m2.attribute(L"name").value()) == L"MyGreenMaterialName";

  bool diff1_ok = (m1.child(L"diffuse")      == nullptr);
  bool reft1_ok = (m1.child(L"reflectivity") == nullptr);

  bool diffColor2_ok = (std::wstring(m2.child(L"diffuse").child(L"color").text().get()) == L"0.952941 0.0196078 0.0196078");
  bool reflBrdf_ok   = (std::wstring(m2.child(L"diffuse").attribute(L"brdf_type").value()) == L"oren_nayar");
  bool roughness_ok  = (std::wstring(m2.child(L"diffuse").child(L"roughness").text().get()) == L"1");

  return name1_ok && name2_ok && diff1_ok && reft1_ok && diffColor2_ok && reflBrdf_ok && roughness_ok;
}


bool test02_materials_changes_open_mode()
{
  hrSceneLibraryOpen(L"tests/test_02", HR_WRITE_DISCARD);

  // material definition
  //
  HRMaterialRef mat  = hrMaterialCreate(L"MyRedMaterialName");   // create new HydraMaterial
  HRMaterialRef mat2 = hrMaterialCreate(L"MyGreenMaterialName"); // create new material

  hrMaterialOpen(mat, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat);

    xml_node diff = matNode.append_child(L"diffuse");
    xml_node refl = matNode.append_child(L"reflectivity");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.352941 0.0196078 0.0196078");
    diff.append_child(L"roughness").text().set(L"0");

    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").text().set(L"0 0 0 ");
    refl.append_child(L"resnel_IOR").text().set(L"1.5");
    refl.append_child(L"glosiness").text().set(L"0.75");
  }
  hrMaterialClose(mat); // HRCheckErrorsMacro; ... 


  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.952941 0.0196078 0.0196078");
    diff.append_child(L"roughness").text().set(L"0");
  }
  hrMaterialClose(mat2);

  hrFlush();

  // make changes
  //
  hrMaterialOpen(mat, HR_WRITE_DISCARD);
  hrMaterialClose(mat); 
  
  hrMaterialOpen(mat2, HR_OPEN_EXISTING);
  {
    xml_node matNode = hrMaterialParamNode(mat2);
  
    xml_node diff = matNode.child(L"diffuse");
  
    diff.attribute(L"brdf_type").set_value(L"oren_nayar");
    diff.child(L"roughness").text().set(L"1");
  }
  hrMaterialClose(mat2);
  
  hrFlush();

  // now check xml by parsing it and find essential attributes!
  bool noDups1 = check_all_duplicates(L"tests/test_02/statex_00001.xml");
  bool noDups2 = check_all_duplicates(L"tests/test_02/statex_00002.xml");

  return check_test_02(L"tests/test_02/statex_00002.xml") && check_test_02(L"tests/test_02/change_00001.xml") && noDups1 && noDups2;
}


bool check_test_03()
{
  pugi::xml_document doc;
  doc.load_file(L"tests/test_03/statex_00001.xml");

  pugi::xml_node libLights = doc.child(L"lights_lib");

  pugi::xml_node l1 = libLights.find_child_by_attribute(L"id", L"0");
  pugi::xml_node l2 = libLights.find_child_by_attribute(L"id", L"1");

  bool color1_ok = std::wstring(l1.child(L"intensity").child(L"color").text().get()) == L"10 10 10";
  bool color2_ok = std::wstring(l2.child(L"intensity").child(L"color").text().get()) == L"1 1 1";

  return color1_ok && color2_ok;
}


bool test03_lights_add()
{
  hrSceneLibraryOpen(L"tests/test_03", HR_WRITE_DISCARD);

  HRLightRef light  = hrLightCreate(L"testPointLight1");
  HRLightRef light2 = hrLightCreate(L"testSpotLight2");
  HRLightRef light3 = hrLightCreate(L"testPointLight3");

  // simple light example; the container will be translated to point light;
  //
  hrLightOpen(light, HR_WRITE_DISCARD);
  {
    xml_node lightNode = hrLightParamNode(light);

    lightNode.attribute(L"shape").set_value(L"point");                // you don't have to add attributes, only modify them
    lightNode.attribute(L"distribution").set_value(L"onmi");          // you don't have to add attributes, only modify them

    lightNode.append_child(L"position").text().set(L"0 10 0");

    xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"10 10 10");
    intensityNode.append_child(L"multiplier").text().set(L"2");
  }
  hrLightClose(light);

  hrLightOpen(light2, HR_WRITE_DISCARD);
  {
    xml_node lightNode = hrLightParamNode(light2);

    lightNode.append_child(L"position").text().set(L"0 10 0");

    xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"1 1 1");
    intensityNode.append_child(L"multiplier").text().set(L"1.5");
  }
  hrLightClose(light2);

  hrFlush();

  return check_test_03();
}

bool check_test_04()
{
  pugi::xml_document doc;
  doc.load_file(L"tests/test_04/statex_00002.xml");

  pugi::xml_node libLights = doc.child(L"lights_lib");

  pugi::xml_node l1 = libLights.find_child_by_attribute(L"id", L"0");
  pugi::xml_node l2 = libLights.find_child_by_attribute(L"id", L"1");

  bool color1_ok     = (std::wstring(l1.child(L"intensity").child(L"color").text().get()) == L"10 10 10");
  bool color2_not_ok = (l2.child(L"intensity").child(L"color") == nullptr);

  bool pos1_ok       = (std::wstring(l1.child(L"position").text().get()) == L"0 10 0");
  bool pos2_ok       = (std::wstring(l2.child(L"position").text().get()) == L"100 0 100");

  return color1_ok && color2_not_ok && pos1_ok && pos2_ok;
}

bool check_test_04_changes()
{
  pugi::xml_document doc;
  doc.load_file(L"tests/test_04/change_00001.xml");

  pugi::xml_node libLights = doc.child(L"lights_lib");

  pugi::xml_node l1 = libLights.find_child_by_attribute(L"id", L"0");
  pugi::xml_node l2 = libLights.find_child_by_attribute(L"id", L"1");
  pugi::xml_node l3 = libLights.find_child_by_attribute(L"id", L"2");

  return (l1 != nullptr) && (l2 != nullptr) && (l3 == nullptr);
}

bool test04_lights_add_change()
{
  hrSceneLibraryOpen(L"tests/test_04", HR_WRITE_DISCARD);

  HRLightRef light  = hrLightCreate(L"testPointLight1");
  HRLightRef light2 = hrLightCreate(L"testSpotLight2");
  HRLightRef light3 = hrLightCreate(L"testPointLight3");

  // simple light example; the container will be translated to point light;
  //
  hrLightOpen(light, HR_WRITE_DISCARD);
  {
    xml_node lightNode = hrLightParamNode(light);

    lightNode.attribute(L"shape").set_value(L"point");                // you don't have to add attributes, only modify them
    lightNode.attribute(L"distribution").set_value(L"onmi");          // you don't have to add attributes, only modify them

    lightNode.append_child(L"position").text().set(L"20 20 20");

    xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"10 10 10");
    intensityNode.append_child(L"multiplier").text().set(L"2");
  }
  hrLightClose(light);

  hrLightOpen(light2, HR_WRITE_DISCARD);
  {
    xml_node lightNode = hrLightParamNode(light2);

    lightNode.append_child(L"position").text().set(L"0 10 0");

    xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"1 1 1");
    intensityNode.append_child(L"multiplier").text().set(L"1.5");
  }
  hrLightClose(light2);

  hrFlush();

  hrLightOpen(light, HR_OPEN_EXISTING);
  {
    xml_node lightNode = hrLightParamNode(light);
    lightNode.child(L"position").text().set(L"0 10 0");
  }
  hrLightClose(light);

  hrLightOpen(light2, HR_WRITE_DISCARD);
  {
    xml_node lightNode = hrLightParamNode(light2);
    lightNode.append_child(L"position").text().set(L"100 0 100");
  }
  hrLightClose(light2);

  hrFlush();

  return check_test_04() && check_test_04_changes();
}

////////////////////////////////////////////////

bool check_05_06_mesh_cube(const wchar_t* a_path) // L"tests/test_64/statex_00003.xml"
{
  pugi::xml_document doc;
  doc.load_file(a_path);

  pugi::xml_node libLights = doc.child(L"geometry_lib");
  pugi::xml_node mesh      = libLights.find_child_by_attribute(L"id", L"0");

  pugi::xml_node pos       = mesh.child(L"positions");
  pugi::xml_node norm      = mesh.child(L"normals");
  pugi::xml_node texcoords = mesh.child(L"texcoords");

  pugi::xml_node ind       = mesh.child(L"indices");
  pugi::xml_node mind      = mesh.child(L"matindices");

  int vertNum = pos.attribute(L"bytesize").as_int() / (sizeof(float)*4);
  int indNum  = ind.attribute(L"bytesize").as_int() / sizeof(int);
  int triNum  = mind.attribute(L"bytesize").as_int() / sizeof(int);

  return (vertNum == 24) && (indNum == 36) && (triNum == 12) && (norm != nullptr) && (texcoords != nullptr);
}

bool check_test_05(const wchar_t* path)
{
  bool cubeIsOk = check_05_06_mesh_cube(path);
  if (!cubeIsOk)
    return false;

  pugi::xml_document doc;
  doc.load_file(path);

  pugi::xml_node scene = doc.child(L"scenes").child(L"scene");

  int childCount = 0;
  for (auto p = scene.first_child(); p != nullptr; p = p.next_sibling())
    childCount++;

  if (childCount != 1)
    return false;

  std::wstring matStr = scene.child(L"instance").attribute(L"matrix").as_string();
  bool matrixIsOk = (matStr == L"3 0 0 0 0 3 0 0 0 0 3 0 0 0 0 3 ");

  return matrixIsOk;
}


bool check_test_05_changes()
{
  bool cubeIsOk = check_05_06_mesh_cube(L"tests/test_05/change_00000.xml");
  if (!cubeIsOk)
    return false;

  pugi::xml_document doc;
  doc.load_file("tests/test_05/change_00000.xml");

  pugi::xml_node scene = doc.child(L"scenes").child(L"scene");

  int childCount = 0;
  for (auto p = scene.first_child(); p != nullptr; p = p.next_sibling())
    childCount++;

  if (childCount != 1)
    return false;

  std::wstring matStr = scene.child(L"instance").attribute(L"matrix").as_string();
  bool matrixIsOk = (matStr == L"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1 ");

  return matrixIsOk;
}

bool check_test_05_changes1()
{
  pugi::xml_document doc;
  doc.load_file(L"tests/test_05/change_00001.xml");

  pugi::xml_node libLights = doc.child(L"geometry_lib");
  pugi::xml_node mesh      = libLights.find_child_by_attribute(L"id", L"0");
  pugi::xml_node scene     = doc.child(L"scenes").child(L"scene");

  int childCount = 0;
  for (auto p = scene.first_child(); p != nullptr; p = p.next_sibling())
    childCount++;

  if (childCount != 1)
    return false;

  std::wstring matStr = scene.child(L"instance").attribute(L"matrix").as_string();
  bool matrixIsOk = (matStr == L"2 0 0 0 0 2 0 0 0 0 2 0 0 0 0 2 ");

  return matrixIsOk && (mesh == nullptr);
}

bool test05_instances_write_discard()
{
  hrSceneLibraryOpen(L"tests/test_05", HR_WRITE_DISCARD);

  uint32_t numberVertices = 24;
  uint32_t numberIndices  = 36;

  float cubeVertices[] =
  {
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f
  };

  float cubeNormals[] =
  {
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f
  };

  float cubeTexCoords[] =
  {
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
  };

  uint32_t cubeIndices[] =
  {
    0, 2, 1,
    0, 3, 2,
    4, 5, 6,
    4, 6, 7,
    8, 9, 10,
    8, 10, 11,
    12, 15, 14,
    12, 14, 13,
    16, 17, 18,
    16, 18, 19,
    20, 23, 22,
    20, 22, 21
  };

  // glEnableClientState(GL_VERTEX_ARRAY);
  // glVertexPointer(4, GL_FLOAT, 0, cubeVertices);
  // glDrawElements(GL_TRIANGLES, numberIndices, GL_UNSIGNED_INT, cubeIndices);

  auto cubeRef = hrMeshCreate(L"cube");

  hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeRef, L"pos", cubeVertices);
    hrMeshVertexAttribPointer4f(cubeRef, L"norm", cubeNormals);
    hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", cubeTexCoords);
    

    hrMeshMaterialId(cubeRef, 0);
    hrMeshAppendTriangles3(cubeRef, numberIndices, (int*)cubeIndices);
  }
  hrMeshClose(cubeRef);

  auto scnRef = hrSceneCreate(L"my scene");

  // draw scene (1)
  //
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    float matrixRot[16] = { 1, 0, 0, 0,
                            0, 1, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1 };

    hrMeshInstance(scnRef, cubeRef, matrixRot);
  }
  hrSceneClose(scnRef);

  hrFlush();


  // draw scene (2)
  //
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    float matrixRot[16] = { 2, 0, 0, 0,
                            0, 2, 0, 0,
                            0, 0, 2, 0,
                            0, 0, 0, 2 };

    hrMeshInstance(scnRef, cubeRef, matrixRot);
  }
  hrSceneClose(scnRef);

  hrFlush();

  // draw scene (3)
  //
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    float matrixRot[16] = { 3, 0, 0, 0,
                            0, 3, 0, 0,
                            0, 0, 3, 0,
                            0, 0, 0, 3 };

    hrMeshInstance(scnRef, cubeRef, matrixRot);
  }
  hrSceneClose(scnRef);

  hrFlush();

  bool noDups1 = check_all_duplicates(L"tests/test_05/statex_00002.xml");
  bool noDups2 = check_all_duplicates(L"tests/test_05/statex_00003.xml");

  return check_test_05(L"tests/test_05/statex_00003.xml") && check_test_05_changes() && check_test_05_changes1() && noDups1 && noDups2;
}

bool check_test_06()
{
  const wchar_t* path = L"tests/test_06/statex_00003.xml";

  bool cubeIsOk = check_05_06_mesh_cube(path);
  if (!cubeIsOk)
    return false;

  pugi::xml_document doc;
  doc.load_file(path);

  pugi::xml_node scene = doc.child(L"scenes").child(L"scene");

  int childCount = 0;
  for (auto p = scene.first_child(); p != nullptr; p = p.next_sibling())
    childCount++;

  if (childCount != 3)
    return false;

  pugi::xml_node i1 = scene.child(L"instance");
  pugi::xml_node i2 = i1.next_sibling();
  pugi::xml_node i3 = i2.next_sibling();

  bool matrix1IsOk = (std::wstring(i1.attribute(L"matrix").as_string()) == L"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1 ");
  bool matrix2IsOk = (std::wstring(i2.attribute(L"matrix").as_string()) == L"2 0 0 0 0 2 0 0 0 0 2 0 0 0 0 2 ");
  bool matrix3IsOk = (std::wstring(i3.attribute(L"matrix").as_string()) == L"3 0 0 0 0 3 0 0 0 0 3 0 0 0 0 3 ");

  return matrix1IsOk && matrix2IsOk && matrix3IsOk;
}

bool test06_instances_open_existent()
{
  hrSceneLibraryOpen(L"tests/test_06", HR_WRITE_DISCARD);

  uint32_t numberVertices = 24;
  uint32_t numberIndices = 36;

  float cubeVertices[] =
  {
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f
  };

  float cubeNormals[] =
  {
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f
  };

  float cubeTexCoords[] =
  {
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
  };

  uint32_t cubeIndices[] =
  {
    0, 2, 1,
    0, 3, 2,
    4, 5, 6,
    4, 6, 7,
    8, 9, 10,
    8, 10, 11,
    12, 15, 14,
    12, 14, 13,
    16, 17, 18,
    16, 18, 19,
    20, 23, 22,
    20, 22, 21
  };

  // glEnableClientState(GL_VERTEX_ARRAY);
  // glVertexPointer(4, GL_FLOAT, 0, cubeVertices);
  // glDrawElements(GL_TRIANGLES, numberIndices, GL_UNSIGNED_INT, cubeIndices);

  auto cubeRef = hrMeshCreate(L"cube");

  hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeRef, L"pos", cubeVertices);
    hrMeshVertexAttribPointer4f(cubeRef, L"norm", cubeNormals);
    hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", cubeTexCoords);

    hrMeshMaterialId(cubeRef, 0);
    hrMeshAppendTriangles3(cubeRef, numberIndices, (int*)cubeIndices);
  }
  hrMeshClose(cubeRef);

  auto scnRef = hrSceneCreate(L"my scene");

  // draw scene (1)
  //
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    float matrixRot[16] = { 1, 0, 0, 0,
                            0, 1, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1 };

    hrMeshInstance(scnRef, cubeRef, matrixRot);
  }
  hrSceneClose(scnRef);

  hrFlush();


  // draw scene (2)
  //
  hrSceneOpen(scnRef, HR_OPEN_EXISTING);
  {
    float matrixRot[16] = { 2, 0, 0, 0,
                            0, 2, 0, 0,
                            0, 0, 2, 0,
                            0, 0, 0, 2 };

    hrMeshInstance(scnRef, cubeRef, matrixRot);
  }
  hrSceneClose(scnRef);

  hrFlush();

  // draw scene (3)
  //
  hrSceneOpen(scnRef, HR_OPEN_EXISTING);
  {
    float matrixRot[16] = { 3, 0, 0, 0,
                            0, 3, 0, 0,
                            0, 0, 3, 0,
                            0, 0, 0, 3 };

    hrMeshInstance(scnRef, cubeRef, matrixRot);
  }
  hrSceneClose(scnRef);

  hrFlush();

  bool noDups1 = check_all_duplicates(L"tests/test_06/statex_00002.xml");
  bool noDups2 = check_all_duplicates(L"tests/test_06/statex_00003.xml");

  return check_test_06() && noDups1 && noDups2;
}

bool check_test_07(const wchar_t* a_path)
{
  pugi::xml_document doc;
  doc.load_file(a_path);

  pugi::xml_node libLights = doc.child(L"cam_lib");

  pugi::xml_node c1 = libLights.find_child_by_attribute(L"id", L"0");

  bool ok1 = std::wstring(c1.child(L"position").text().get()) == L"0 0 4.50035";
  bool ok2 = std::wstring(c1.child(L"look_at").text().get())  == L"0 1.62921e-005 -95.4996";

  return ok1 && ok2;
}

bool test07_camera_add()
{
  hrSceneLibraryOpen(L"tests/test_07", HR_WRITE_DISCARD);

  HRLightRef  light = hrLightCreate(L"testPointLight1");

  // simple light example; the container will be translated to point light;
  //
  hrLightOpen(light, HR_WRITE_DISCARD);
  {
    xml_node lightNode = hrLightParamNode(light);

    lightNode.attribute(L"shape").set_value(L"point");                // you don't have to add attributes, only modify them
    lightNode.attribute(L"distribution").set_value(L"onmi");          // you don't have to add attributes, only modify them

    lightNode.append_child(L"position").text().set(L"0 10 0");

    xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"10 10 10");
    intensityNode.append_child(L"multiplier").text().set(L"2");
  }
  hrLightClose(light);

  HRCameraRef cam = hrCameraCreate(L"MyCamera");

  hrCameraOpen(cam, HR_WRITE_DISCARD);
  {
    xml_node camNode = hrCameraParamNode(cam);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.0254");
    camNode.append_child(L"farClipPlane").text().set(L"25.4");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 0 4.50035");
    camNode.append_child(L"look_at").text().set(L"0 1.62921e-005 -95.4996");
  }
  hrCameraClose(cam);

  hrFlush();

  return check_test_07(L"tests/test_07/statex_00001.xml") && check_test_07(L"tests/test_07/change_00000.xml");
}

bool check_test_08_1(const wchar_t* a_fileName)
{
  pugi::xml_document doc;
  doc.load_file(a_fileName);

  pugi::xml_node libLights = doc.child(L"cam_lib");

  pugi::xml_node c1 = libLights.find_child_by_attribute(L"id", L"0");

  bool ok1 = std::wstring(c1.child(L"position").text().get()) == L"0 0 4.50035";
  bool ok2 = std::wstring(c1.child(L"look_at").text().get()) == L"0 1.62921e-005 -95.4996";

  return ok1 && ok2;
}

bool check_test_08_2(const wchar_t* a_fileName)
{
  pugi::xml_document doc;
  doc.load_file(a_fileName);

  pugi::xml_node libLights = doc.child(L"cam_lib");

  pugi::xml_node c1 = libLights.find_child_by_attribute(L"id", L"0");

  bool ok1 = std::wstring(c1.child(L"position").text().get()) == L"1 1 1";
  bool ok2 = std::wstring(c1.child(L"look_at").text().get()) == L"0 -1 0";

  return ok1 && ok2;
}

bool test08_camera_add_change()
{
  hrSceneLibraryOpen(L"tests/test_08", HR_WRITE_DISCARD);

  HRLightRef  light = hrLightCreate(L"testPointLight1");

  // simple light example; the container will be translated to point light;
  //
  hrLightOpen(light, HR_WRITE_DISCARD);
  {
    xml_node lightNode = hrLightParamNode(light);

    lightNode.attribute(L"shape").set_value(L"point");                // you don't have to add attributes, only modify them
    lightNode.attribute(L"distribution").set_value(L"onmi");          // you don't have to add attributes, only modify them

    lightNode.append_child(L"position").text().set(L"0 10 0");

    xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"10 10 10");
    intensityNode.append_child(L"multiplier").text().set(L"2");
  }
  hrLightClose(light);

  HRCameraRef cam = hrCameraCreate(L"MyCamera");

  hrCameraOpen(cam, HR_WRITE_DISCARD);
  {
    xml_node camNode = hrCameraParamNode(cam);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.0254");
    camNode.append_child(L"farClipPlane").text().set(L"25.4");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 0 4.50035");
    camNode.append_child(L"look_at").text().set(L"0 1.62921e-005 -95.4996");
  }
  hrCameraClose(cam);

  hrFlush();

  hrCameraOpen(cam, HR_OPEN_EXISTING);
  {
    xml_node camNode = hrCameraParamNode(cam);

    camNode.child(L"up").text().set(L"0 1 0");
    camNode.child(L"position").text().set(L"10 10 10");
    camNode.child(L"look_at").text().set(L"20 20 20");
  }
  hrCameraClose(cam);

  hrCameraOpen(cam, HR_OPEN_EXISTING);
  {
    xml_node camNode = hrCameraParamNode(cam);

    camNode.child(L"up").text().set(L"0 1 0");
    camNode.child(L"position").text().set(L"1 1 1");
    camNode.child(L"look_at").text().set(L"0 -1 0");
  }
  hrCameraClose(cam);

  hrFlush();

  bool noDups1 = check_all_duplicates(L"tests/test_08/statex_00001.xml");
  bool noDups2 = check_all_duplicates(L"tests/test_08/statex_00002.xml");

  return check_test_08_1(L"tests/test_08/change_00000.xml") && check_test_08_1(L"tests/test_08/statex_00001.xml") && 
         check_test_08_2(L"tests/test_08/change_00001.xml") && check_test_08_2(L"tests/test_08/statex_00002.xml") && noDups1 && noDups2;
}

bool check_test_09()
{

  bool ok3 = false, ok4 = false;
  {
    pugi::xml_document doc;
    doc.load_file(L"tests/test_09/statex_00001.xml");

    pugi::xml_node libLights = doc.child(L"render_lib");

    pugi::xml_node c1 = libLights.find_child_by_attribute(L"id", L"0");

    ok3 = std::wstring(c1.child(L"width").text().get()) == L"1024";
    ok4 = std::wstring(c1.child(L"height").text().get()) == L"768";
  }

  pugi::xml_document doc;
  doc.load_file(L"tests/test_09/statex_00002.xml");

  pugi::xml_node libLights = doc.child(L"render_lib");

  pugi::xml_node c1 = libLights.find_child_by_attribute(L"id", L"0");

  bool ok1 = std::wstring(c1.child(L"width").text().get()) == L"768";
  bool ok2 = std::wstring(c1.child(L"height").text().get()) == L"768";

  return ok1 && ok2 && ok3 && ok4;
}


bool test09_render_ogl()
{
  hrSceneLibraryOpen(L"tests/test_09", HR_WRITE_DISCARD);

  HRLightRef  light = hrLightCreate(L"testPointLight1");

  // simple light example; the container will be translated to point light;
  //
  hrLightOpen(light, HR_WRITE_DISCARD);
  {
    xml_node lightNode = hrLightParamNode(light);

    lightNode.attribute(L"shape").set_value(L"point");                // you don't have to add attributes, only modify them
    lightNode.attribute(L"distribution").set_value(L"onmi");          // you don't have to add attributes, only modify them

    lightNode.append_child(L"position").text().set(L"0 10 0");

    xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"10 10 10");
    intensityNode.append_child(L"multiplier").text().set(L"2");
  }
  hrLightClose(light);

  // camera
  //
  HRCameraRef cam = hrCameraCreate(L"MyCamera");

  hrCameraOpen(cam, HR_WRITE_DISCARD);
  {
    xml_node camNode = hrCameraParamNode(cam);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.0254");
    camNode.append_child(L"farClipPlane").text().set(L"25.4");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 0 4.50035");
    camNode.append_child(L"look_at").text().set(L"0 1.62921e-005 -95.4996");
  }
  hrCameraClose(cam);

  // render
  //
  HRRenderRef settingsRef = hrRenderCreate(L"opengl1");

  hrRenderOpen(settingsRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(settingsRef);

    node.append_child(L"width").text().set(L"1024");
    node.append_child(L"height").text().set(L"768");
  }
  hrRenderClose(settingsRef);

  //
  //
  hrFlush();

  hrCameraOpen(cam, HR_OPEN_EXISTING);
  {
    xml_node camNode = hrCameraParamNode(cam);

    camNode.child(L"up").text().set(L"0 1 0");
    camNode.child(L"position").text().set(L"10 10 10");
    camNode.child(L"look_at").text().set(L"20 20 20");
  }
  hrCameraClose(cam);

  hrCameraOpen(cam, HR_OPEN_EXISTING);
  {
    xml_node camNode = hrCameraParamNode(cam);

    camNode.child(L"up").text().set(L"0 1 0");
    camNode.child(L"position").text().set(L"1 1 1");
    camNode.child(L"look_at").text().set(L"0 -1 0");
  }
  hrCameraClose(cam);

  hrRenderOpen(settingsRef, HR_OPEN_EXISTING);
  {
    pugi::xml_node node = hrRenderParamNode(settingsRef);
    node.child(L"width").text().set(L"768");
  }
  hrRenderClose(settingsRef);

  hrFlush();

  return check_test_09();
}



bool check_test_16()
{
	pugi::xml_document doc;
	doc.load_file(L"tests/test_16/statex_00002.xml");

	pugi::xml_node libMat = doc.child(L"materials_lib");
	pugi::xml_node libTex1 = doc.child(L"textures_lib");

	pugi::xml_node m1 = libMat.find_child_by_attribute(L"id", L"0");
	pugi::xml_node m2 = libMat.find_child_by_attribute(L"id", L"1");

	bool tex1_id_ok    = std::wstring(m1.child(L"diffuse").child(L"texture").attribute(L"id").value()) == L"2";
	bool tex2_id_ok    = std::wstring(m2.child(L"reflect").child(L"texture").attribute(L"id").value()) == L"1";
	bool del_tex_id_ok = std::wstring(m2.child(L"diffuse").child(L"texture").attribute(L"id").value()) == L"";

	pugi::xml_node ts1 = libTex1.find_child_by_attribute(L"id", L"1");
	pugi::xml_node ts2 = libTex1.find_child_by_attribute(L"id", L"2");

	bool tex1_state_name_ok = std::wstring(ts1.attribute(L"name").value()) == L"data/textures/texture1.bmp";
	bool tex2_state_name_ok = std::wstring(ts2.attribute(L"name").value()) == L"data/textures/163.jpg";


	pugi::xml_document doc2;
	doc2.load_file(L"tests/test_16/change_00001.xml");

	pugi::xml_node libTex2 = doc2.child(L"textures_lib");

	pugi::xml_node t1 = libTex2.find_child_by_attribute(L"id", L"2");
	pugi::xml_node t2 = libTex2.find_child_by_attribute(L"id", L"1");
	
	bool tex1_name_ok = std::wstring(t1.attribute(L"name").value()) == L"data/textures/163.jpg";
	//bool tex2_name_ok = std::wstring(t2.attribute(L"name").value()) == L"data/textures/texture1.bmp";


	
	return tex1_id_ok && tex2_id_ok && tex1_name_ok && tex1_state_name_ok && tex2_state_name_ok;
}


bool test16_texture_add_change()
{
	hrSceneLibraryOpen(L"tests/test_16", HR_WRITE_DISCARD);


	HRMaterialRef mat = hrMaterialCreate(L"MyTestMaterial_1");   
	HRMaterialRef mat2 = hrMaterialCreate(L"MyTestMaterial_2"); 

	hrMaterialOpen(mat, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat);

		xml_node diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"0.5 0.5 0.001");
		diff.append_child(L"roughness").text().set(L"0.88");

	}
	hrMaterialClose(mat); 


	hrMaterialOpen(mat2, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat2);

		xml_node diff = matNode.append_child(L"diffuse");
		xml_node refl = matNode.append_child(L"reflect");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"0.01 0.33 0.55");
		diff.append_child(L"roughness").text().set(L"0");

		refl.append_attribute(L"brdf_type").set_value(L"microfacet");
		refl.append_child(L"color").text().set(L"0.25 0.25 0.25");
		refl.append_child(L"resnel_IOR").text().set(L"1.5");
		refl.append_child(L"glosiness").text().set(L"0.88");

		HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
		hrTextureBind(testTex, refl);
		hrTextureBind(testTex, diff);
	}
	hrMaterialClose(mat2);

	hrFlush();

	// make changes
	//
	hrMaterialOpen(mat, HR_OPEN_EXISTING);
	{
		xml_node matNode = hrMaterialParamNode(mat);
		xml_node diff = matNode.child(L"diffuse");

		HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/163.jpg");
		hrTextureBind(testTex, diff);
	}
	hrMaterialClose(mat);


	hrMaterialOpen(mat2, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat2);

		xml_node diff = matNode.append_child(L"diffuse");
		xml_node refl = matNode.append_child(L"reflect");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"0.01 0.33 0.55");
		diff.append_child(L"roughness").text().set(L"0");

		refl.append_attribute(L"brdf_type").set_value(L"microfacet");
		refl.append_child(L"color").text().set(L"0.25 0.25 0.25");
		refl.append_child(L"resnel_IOR").text().set(L"1.5");
		refl.append_child(L"glosiness").text().set(L"0.88");

		HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");

		hrTextureBind(testTex, refl);
	}
	hrMaterialClose(mat2);

	hrFlush();

  bool noDups1 = check_all_duplicates(L"tests/test_16/statex_00001.xml");
  bool noDups2 = check_all_duplicates(L"tests/test_16/statex_00002.xml");

	return check_test_16() && noDups1 && noDups2;
}


bool check_test_17()
{
	pugi::xml_document doc;
	doc.load_file(L"tests/test_17/change_00000.xml");

	pugi::xml_node libTex = doc.child(L"textures_lib");

	pugi::xml_node t1 = libTex.find_child_by_attribute(L"id", L"1");

	bool tex1_name_ok = std::wstring(t1.attribute(L"name").value()) == L"fu_1";
	bool tex1_type_ok = std::wstring(t1.attribute(L"type").value()) == L"falloff";

	bool colorA_ok = std::wstring(t1.child(L"color_A").text().get()) == L"0.0 1.0 0.0";
	bool colorB_ok = std::wstring(t1.child(L"color_B").text().get()) == L"0.0 0.0 1.0";
	bool someparam1_ok = std::wstring(t1.child(L"some_param1").text().get()) == L"some_value";


	pugi::xml_document doc2;
	doc.load_file(L"tests/test_17/change_00001.xml");

	pugi::xml_node libTex2 = doc.child(L"textures_lib");

	pugi::xml_node t2 = libTex2.find_child_by_attribute(L"id", L"2");

	bool tex2_name_ok = std::wstring(t2.attribute(L"name").value()) == L"fu_1";
	bool tex2_type_ok = std::wstring(t2.attribute(L"type").value()) == L"falloff";

	bool colorA2_ok = std::wstring(t2.child(L"color_A").text().get()) == L"1.0 1.0 0.0";
	bool colorB2_ok = std::wstring(t2.child(L"color_B").text().get()) == L"0.0 1.0 1.0";
	bool someparam2_1_ok = std::wstring(t2.child(L"some_param1").text().get()) == L"some_new_value";
	bool someparam2_2_ok = std::wstring(t2.child(L"some_param2").text().get()) == L"some_new_value";

	return tex1_name_ok && tex1_type_ok && colorA_ok && colorB_ok && someparam1_ok && colorA2_ok && colorB2_ok && someparam2_1_ok && someparam2_2_ok;
}


bool test17_falloff()
{
	hrSceneLibraryOpen(L"tests/test_17", HR_WRITE_DISCARD);


	HRMaterialRef mat = hrMaterialCreate(L"MyTestMaterial_1");

	hrMaterialOpen(mat, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat);

		xml_node diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"0.5 0.5 0.001");
		diff.append_child(L"roughness").text().set(L"0.88");

		HRTextureNodeRef testTex = hrTextureCreateAdvanced(L"falloff", L"fu_1");
		hrTextureBind(testTex, diff);

		hrTextureNodeOpen(testTex);
		{
			xml_node texNode = hrTextureParamNode(testTex);

			texNode.append_child(L"color_A").text().set(L"0.0 1.0 0.0");
			texNode.append_child(L"color_B").text().set(L"0.0 0.0 1.0");

			texNode.append_child(L"some_param1").text().set(L"some_value");

		}
		hrTextureNodeClose(testTex);

	}
	hrMaterialClose(mat);

	hrFlush();

	// make changes
	//
	hrMaterialOpen(mat, HR_OPEN_EXISTING);
	{
		xml_node matNode = hrMaterialParamNode(mat);
		xml_node diff = matNode.child(L"diffuse");

		HRTextureNodeRef testTex = hrTextureCreateAdvanced(L"falloff", L"fu_1");
		hrTextureBind(testTex, diff);

		hrTextureNodeOpen(testTex);
		{
			xml_node texNode = hrTextureParamNode(testTex);

			texNode.append_child(L"color_A").text().set(L"1.0 1.0 0.0");
			texNode.append_child(L"color_B").text().set(L"0.0 1.0 1.0");

			texNode.append_child(L"some_param1").text().set(L"some_new_value");
			texNode.append_child(L"some_param2").text().set(L"some_new_value");
		}
		hrTextureNodeClose(testTex);

	}
	hrMaterialClose(mat);


	hrFlush();


	return check_test_17();
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void test03_open_scene()
{
  HRMeshRef      cubeRef;
  HRSceneInstRef scnRef;

  uint32_t numberVertices = 24;
  uint32_t numberIndices  = 36;

  float cubeVertices[] =
  {
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f
  };

  float cubeNormals[] =
  {
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f
  };

  float cubeTexCoords[] =
  {
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
  };

  uint32_t cubeIndices[] =
  {
    0, 2, 1,
    0, 3, 2,
    4, 5, 6,
    4, 6, 7,
    8, 9, 10,
    8, 10, 11,
    12, 15, 14,
    12, 14, 13,
    16, 17, 18,
    16, 18, 19,
    20, 23, 22,
    20, 22, 21
  };

  hrSceneLibraryOpen(L"OpenGL1", HR_WRITE_DISCARD);
  
  cubeRef = hrMeshCreate(L"cube");

  hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeRef, L"pos", cubeVertices);
    hrMeshVertexAttribPointer4f(cubeRef, L"norm", cubeNormals);
    hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", cubeTexCoords);

    hrMeshMaterialId(cubeRef, 0);
    hrMeshAppendTriangles3(cubeRef, numberIndices, (int*)cubeIndices);
  }
  hrMeshClose(cubeRef);

  scnRef = hrSceneCreate(L"my scene");

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneOpen(scnRef, HR_WRITE_DISCARD); 
  {
    float matrixRot[16] = {1,0,0,0,
                           0,1,0,0,
                           0,0,1,0,
                           0,0,0,1};

    hrMeshInstance(scnRef, cubeRef, matrixRot); 
  }
  hrSceneClose(scnRef);                         

  hrSceneOpen(scnRef, HR_WRITE_DISCARD); 
  {
    float matrixRot[16] = {1,1,1,1,
                           0,1,0,0,
                           0,0,1,0,
                           0,0,0,1};

    hrMeshInstance(scnRef, cubeRef, matrixRot); 
  }
  hrSceneClose(scnRef);       

  hrSceneOpen(scnRef, HR_OPEN_EXISTING);
  {
    float matrixRot[16] = { 2, 2, 2, 2,
                            0, 1, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1 };

     float matrixRot2[16] = { 3, 3, 3, 3,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                              0, 0, 0, 1 };

    hrMeshInstance(scnRef, cubeRef, matrixRot);
    hrMeshInstance(scnRef, cubeRef, matrixRot2);
  }

  hrSceneClose(scnRef);

  hrFlush();

  /*
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    float matrixRot[16] = { 1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1 };

    hrMeshInstance(scnRef, cubeRef, matrixRot);
  }
  hrSceneClose(scnRef);

  hrFlush();
  */
}

bool g_testWasIgnored = false;

bool dummy_test() 
{ 
  g_testWasIgnored = true;  
  return false; 
}


typedef bool(*TestFunc)();

float g_MSEOutput = 0.0f;

void run_all_api_tests(const int startTestId)
{
  TestFunc tests[] = { &test01_materials_add,
                       &test02_materials_changes_open_mode,
                       &test03_lights_add,
                       &test04_lights_add_change,
                       &test05_instances_write_discard,
                       &test06_instances_open_existent,
                       &test07_camera_add,
                       &test08_camera_add_change,
                       &test09_render_ogl,
                       &test10_render_ogl_cube,
                       &test11_render_ogl_some_figures,
                       &test12_render_ogl_100_random_figures,
                       &test13_render_ogl_some_figures_diff_mats_prom_ptr,
                       &test14_bad_material_indices,
                       &test15_main_scene_and_mat_editor,
											 &test16_texture_add_change,
											 &test17_falloff,
                       &test18_camera_move,
                       &test19_material_change,
                       &test20_mesh_change,
                       &test21_add_same_textures_from_file,
                       &test22_can_not_load_texture,
                       &test23_texture_from_memory,
                       &test24_many_textures_big_data,
                       &test25_many_textures_big_data, 
                       &test26_many_textures_big_data,
                       &test27_many_textures_big_data_from_mem,
                       &test28_compute_normals,
                       &test29_many_textures_and_meshes,
                       &test30_many_textures_and_meshes,
											 &test31_procedural_texture_LDR,
											 &test32_procedural_texture_HDR,
											 &test33_update_from_file,
                       &test34_delayed_textures_does_not_exists,
                       &test35_cornell_with_light,
											 &test36_update_from_memory,
                       &dummy_test,                                      // &test37_cornell_with_light_different_image_layers,
                       &dummy_test,                                      // &test38_mlt_images,
                       &test39_mesh_from_vsgf,
                       &test40_several_changes,
                       &test41_load_library_basic,
                       &test42_load_library_basic,
                       &test43_test_direct_light,
                       &test44_four_lights,
                       &test45_mesh_from_vsgf_opengl_bug_teapot,
                       &test46_light_geom_rect,
                       &test47_light_geom_disk,
                       &test48_light_geom_sphere,
	                     &test49_light_geom_disk,
	                     &test50_open_library_several_times,
                       &dummy_test,                 // 51
                       &dummy_test,                 // 52
                       &dummy_test,                 // 53
                       &dummy_test,                 // 54
                       &test55_clear_scene,
                       &test56_mesh_change_open_existing,
                       &test57_single_instance,
                       &dummy_test,                 // 58
                       &test59_cornell_water_mlt,
                       &dummy_test,                 // 60
                       &test61_cornell_with_light_near_wall_and_glossy_wall,  
                       &dummy_test,                 // 62
                       &dummy_test,
                       &test64_several_changes_light_area,
                       &test65_several_changes_light_rect,
                       &test66_fast_render_no_final_update,
                       &test67_fast_empty_scene,
                       &test68_scene_library_file_info,
                       &test69_pause_and_resume,
                       &test70_area_lights16,
                       &test71_out_of_memory,
                       &dummy_test,                  // 72
                       &dummy_test,                  // 73 test73_big_resolution
                       &test74_frame_buffer_line,
                       &test75_repeated_render,
                       &test76_empty_mesh,
                       &test77_save_gbuffer_layers,
                       &test78_material_remap_list1,
                       &test79_material_remap_list2,
  };


  int testNum = sizeof(tests) / sizeof(TestFunc);

  int startTestId2 = startTestId - 1;
  if (startTestId2 < 0) 
    startTestId2 = 0;

  std::ofstream fout("z_test_api.txt");

  for (int i = startTestId2; i < testNum; i++)
  {
    bool res = tests[i]();
    if (res)
    {
      std::cout << "api_test_" << std::setfill('0') << std::setw(2) << i + 1 << "\tPASSED!\t\n";
      fout << std::fixed << "api_test_" << std::setfill('0') << std::setw(2) << i + 1 << "\tPASSED!\t\n";
    }
    else if (g_testWasIgnored)
    {
      std::cout << "api_test_" << std::setfill('0') << std::setw(2) << i + 1 << "\tSKIPPED!" << std::endl;
      fout << std::fixed << "api_test_" << std::setfill('0') << std::setw(2) << i + 1 << "\tSKIPPED!" << std::endl;
    }
    else
    {
      std::cout << "api_test_" << std::setfill('0') << std::setw(2) << i + 1 << "\tFAILED! :-: MSE = " << g_MSEOutput << std::endl;
      fout << std::fixed << "api_test_" << std::setfill('0') << std::setw(2) << i + 1 << "\tFAILED! :-: MSE = " << g_MSEOutput << std::endl;
    }

    fout.flush();

    g_testWasIgnored = false;
  }

  fout.close();


  glfwTerminate();
}

void run_all_geo_tests()
{
	using namespace GEO_TESTS;
	TestFunc tests[] = { &test_001_mesh_from_memory,
						           &test_002_mesh_from_vsgf,
						           &test_003_compute_normals,
                       &test_004_dof,
                       &test_005_instancing, };


	int testNum = sizeof(tests) / sizeof(TestFunc);

  std::ofstream fout("z_test_geometry.txt");

	for (int i = 0; i < testNum; i++)
	{
		bool res = tests[i]();
    if (res)
    {
      std::cout << "geo_test_" << std::setfill('0') << std::setw(3) << i + 1 << "\tPASSED!\t\n";
      fout      << "geo_test_" << std::setfill('0') << std::setw(3) << i + 1 << "\tPASSED!\t\n";
    }
    else if (g_testWasIgnored)
    {
      std::cout << "geo_test_" << std::setfill('0') << std::setw(3) << i + 1 << "\tSKIPPED!\t\n";
      fout      << "geo_test_" << std::setfill('0') << std::setw(3) << i + 1 << "\tSKIPPED!\t\n";
    }
    else
    {
      std::cout << "geo_test_" << std::setfill('0') << std::setw(3) << i + 1 << "\tFAILED! :-: MSE = " << g_MSEOutput << std::endl;
      fout      << "geo_test_" << std::setfill('0') << std::setw(3) << i + 1 << "\tFAILED! :-: MSE = " << g_MSEOutput << std::endl;
    }

    g_testWasIgnored = false;
	}

  fout.close();
}

void run_all_mtl_tests(int a_start)
{
	using namespace MTL_TESTS;
	TestFunc tests[] = { &test_101_diffuse_lambert,
						           &test_102_diffuse_orennayar,
						           &test_103_diffuse_texture,
						           &test_104_reflect_phong,
						           &test_105_reflect_microfacet,
						           &test_106_reflect_fresnel_ior,
						           &test_107_reflect_extrusion,
						           &test_108_reflect_texture,
						           &test_109_reflect_glossiness_texture,
                       &test_110_texture_sampler,
                       &test_111_glossiness_texture_sampler,
	                     &test_112_transparency,
	                     &test_113_transparency_ior,
                       &test_114_transparency_fog,
                       &test_115_transparency_fog_mult,
                       &test_116_transparency_thin,
                       &test_117_transparency_texture,
                       &test_118_transparency_glossiness_texture,
                       &test_119_opacity_texture,
                       &test_120_opacity_shadow_matte,
                       &test_121_translucency,
                       &test_122_translucency_texture,
                       &test_123_emission,
                       &test_124_emission_texture,
                       &test_125_emission_cast_gi,
                       &test_126_bump_amount,
                       &test_127_normal_map_height,
                       &test_128_bump_radius,
                       &test_129_parallax,
                       &test_130_bump_invert_normalY,
                       &test_131_blend_simple,
                       &test_132_blend_recursive,
		                   &test_133_emissive_and_diffuse,
		                   &test_134_diff_refl_transp,
                       &test_135_opacity_metal,
                       &test_136_opacity_glass,
                       &test_137_opacity_emission,
                       &test_138_translucency_and_diffuse,
                       &test_139_glass_and_bump,
                       &test_140_blend_emission,
                       &test_141_opacity_smooth,
                       &test_150_gloss_mirror_cos_div,
                       &test_151_gloss_mirror_cos_div2
	                   };


	const int testNum = sizeof(tests) / sizeof(TestFunc);

  std::ofstream fout("z_test_materials.txt");

	for (int i = a_start; i < testNum; i++)
	{
		bool res = tests[i]();
    if (res)
    {
      std::cout          << "mtl_test_" << std::setfill('0') << std::setw(3) << 100 + i + 1 << "\tPASSED!\t\n";
      fout << std::fixed << "mtl_test_" << std::setfill('0') << std::setw(3) << 100 + i + 1 << "\tPASSED!\t\n";
    }
    else if (g_testWasIgnored)
    {
      std::cout          << "mtl_test_" << std::setfill('0') << std::setw(3) << 100 + i + 1 << "\tSKIPPED!\t\n";
      fout << std::fixed << "mtl_test_" << std::setfill('0') << std::setw(3) << 100 + i + 1 << "\tSKIPPED!\t\n";
    }
    else
    {
      std::cout          << "mtl_test_" << std::setfill('0') << std::setw(3) << 100 + i + 1 << "\tFAILED! :-: MSE = " << g_MSEOutput << std::endl;
      fout << std::fixed << "mtl_test_" << std::setfill('0') << std::setw(3) << 100 + i + 1 << "\tFAILED! :-: MSE = " << g_MSEOutput << std::endl;
    }

    fout.flush();

    g_testWasIgnored = false;
	}

  fout.close();
	
}

void run_all_lgt_tests(int a_start)
{
  using namespace LGHT_TESTS;
  TestFunc tests[] = { &test_200_spot,
                       &test_201_sphere,
                       &test_202_sky_color,
                       &test_203_sky_hdr,
                       &test_204_sky_hdr_rotate,
                       &test_205_sky_and_directional_sun,
                       &test_206_ies1,
                       &test_207_ies2,
                       &test_208_ies3,
                       &test_209_skyportal,
                       &test_210_skyportal_hdr,
                       &test_211_sky_and_sun_perez,
                       &test_212_skyportal_sun,
                       &test_213_point_omni,
	                     &test_214_sky_ldr,
	                     &test_215_light_scale_intensity,
	                     &test_216_ies4,
                       &test_217_cylinder,
                       &test_218_cylinder2,
                       &test_219_cylinder_tex,
                       &test_220_cylinder_tex2,
                       &test_221_cylinder_tex3,
                       &test_222_cylinder_with_end_face,

                       &test_223_rotated_area_light,
                       &test_224_rotated_area_light2,
                       &test_225_point_spot_simple,
                       &test_226_area_spot_simple,
                       &test_227_point_spot_glossy_wall,
                       &test_228_point_ies_for_bpt,
                       &test_229_point_ies_for_bpt,
                       &test_230_area_ies_for_bpt,
                       &test_231_direct_soft_shadow,
                      
                       &test_232_point_area_ies,
                       &test_233_light_group_point_area_ies,
                       &test_234_light_group_light_inst_cust_params,
                       &test_235_stadium,
                       &test_236_light_group_point_area_ies2,
                       &test_237_cubemap_ldr,
                       
                       &test_238_mesh_light_one_triangle,
                       &test_239_mesh_light_two_triangle,
                       &test_240_mesh_light_torus,
                       &test_241_mesh_light_torus_texture_ldr,
                       &test_242_mesh_light_torus_texture_hdr,
                       &test_243_mesh_light_do_not_sample_me,
                       &test_244_do_not_sample_me,
                       &test_245_cylinder_tex_nearest,
  };

  std::ofstream fout("z_test_lights.txt");

  const int testNum = sizeof(tests) / sizeof(TestFunc);

  for (int i = a_start; i < testNum; i++)
  {
    bool res = tests[i]();
    if (res)
    {
      std::cout          << "light_test_" << std::setfill('0') << std::setw(3) << 200 + i << "\tPASSED!\t\n";
      fout << std::fixed << "light_test_" << std::setfill('0') << std::setw(3) << 200 + i << "\tPASSED!\t\n";
    }
    else if (g_testWasIgnored)
    {
      std::cout          << "light_test_" << std::setfill('0') << std::setw(3) << 200 + i << "\tSKIPPED!\t\n";
      fout << std::fixed << "light_test_" << std::setfill('0') << std::setw(3) << 200 + i << "\tSKIPPED!\t\n";
    }
    else
    {
      std::cout          << "light_test_" << std::setfill('0') << std::setw(3) << 200 + i << "\tFAILED! :-: MSE = " << g_MSEOutput << std::endl;
      fout << std::fixed << "light_test_" << std::setfill('0') << std::setw(3) << 200 + i << "\tFAILED! :-: MSE = " << g_MSEOutput << std::endl;
    }

    fout.flush();

    g_testWasIgnored = false;
  }

  fout.close();

}
#ifdef WIN32
void run_all_ipp_tests(int a_start)
{
  TestFunc tests[] = {                 
    &PP_TESTS::test301_resample,
    &PP_TESTS::test302_median,
    &PP_TESTS::test303_median_in_place,
    &PP_TESTS::test304_obsolete_tone_mapping,
    &PP_TESTS::test305_fbi_from_render,
    &PP_TESTS::test306_post_process_hydra1_exposure05,
    &PP_TESTS::test307_post_process_hydra1_exposure2,
    &PP_TESTS::test308_post_process_hydra1_compress,
    &PP_TESTS::test309_post_process_hydra1_contrast,
    &PP_TESTS::test310_post_process_hydra1_desaturation,
    &PP_TESTS::test311_post_process_hydra1_saturation,
    &PP_TESTS::test312_post_process_hydra1_whiteBalance,
    &PP_TESTS::test313_post_process_hydra1_uniformContrast,
    &PP_TESTS::test314_post_process_hydra1_normalize,
    &PP_TESTS::test315_post_process_hydra1_vignette,
    &PP_TESTS::test316_post_process_hydra1_chromAberr,
    &PP_TESTS::test317_post_process_hydra1_sharpness,
    &PP_TESTS::test318_post_process_hydra1_ECCSWUNSVC,
    &PP_TESTS::test319_post_process_hydra1_diffStars
  };

  std::ofstream fout("z_test_post_process.txt");

  const int testNum = sizeof(tests) / sizeof(TestFunc);

  for (int i = a_start; i < testNum; i++)
  {
    bool res = tests[i]();
    if (res)
    {
      std::cout          << "pp_test_" << std::setfill('0') << std::setw(3) << 300 + i << "\tPASSED!\t\n";
      fout << std::fixed << "pp_test_" << std::setfill('0') << std::setw(3) << 300 + i << "\tPASSED!\t\n";
    }
    else if (g_testWasIgnored)
    {
      std::cout          << "pp_test_" << std::setfill('0') << std::setw(3) << 300 + i << "\tSKIPPED!\t\n";
      fout << std::fixed << "pp_test_" << std::setfill('0') << std::setw(3) << 300 + i << "\tSKIPPED!\t\n";
    }
    else
    {
      std::cout          << "pp_test_" << std::setfill('0') << std::setw(3) << 300 + i << "\tFAILED! :-: MSE = " << g_MSEOutput << std::endl;
      fout << std::fixed << "pp_test_" << std::setfill('0') << std::setw(3) << 300 + i << "\tFAILED! :-: MSE = " << g_MSEOutput << std::endl;
    }

    fout.flush();

    g_testWasIgnored = false;
  }

  fout.close();
}
#endif

void terminate_opengl()
{
	glfwTerminate();
}
