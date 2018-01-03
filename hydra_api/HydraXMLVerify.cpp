#include "HydraXMLVerify.h"
#include <iostream>
#include <sstream>
#include <cmath>

namespace HydraVerify
{
	ErrorFuncType g_errCallBack = nullptr;

	void SetErrorCallback(ErrorFuncType a_func)
	{
		g_errCallBack = a_func;
	}

	void Error(const wchar_t* a_msg, const wchar_t* a_file, int a_line)
	{
		if (g_errCallBack != nullptr)
			g_errCallBack(a_msg, a_file, a_line);
		else
			std::wcout << a_msg << L"\nfile: " << a_file << L", line: " << a_line << std::endl << std::endl;
	}

  bool VerifyXMLParameter(const pugi::xml_node a_node, const char* a_file, int a_line)
  {
    const std::wstring tagName = a_node.name();
    //const std::wstring objType = a_node.attribute(L"type").as_string();

    std::string  tmp(a_file);
    std::wstring wsTmp(tmp.begin(), tmp.end());

    if (tagName == L"light")    // objType == L"hydra_light"
      return VerifyLight(a_node, wsTmp.c_str(), a_line);
    else if (tagName == L"material") // && objType == L"hydra_material"
      return VerifyMaterial(a_node, wsTmp.c_str(), a_line);
    else if (tagName == L"camera")   //  && objType == L"uvn"
      return VerifyCamera(a_node, wsTmp.c_str(), a_line);
    else if (tagName == L"render_settings")   //  && objType == L"uvn"
      return VerifySettings(a_node, wsTmp.c_str(), a_line);
    else
    {
      std::wstring errMsg = std::wstring(L"verify, unknown tag name: ") + tagName;
      Error(errMsg.c_str(), wsTmp.c_str(), a_line);
      return false;
    }
  }

	bool VerifyXMLParameter(const pugi::xml_node a_node, const wchar_t* a_file, int a_line)
	{
		const std::wstring tagName = a_node.name();
		//const std::wstring objType = a_node.attribute(L"type").as_string();

		if (tagName == L"light")    // objType == L"hydra_light"
			return VerifyLight(a_node, a_file, a_line);
		else if (tagName == L"material") // && objType == L"hydra_material"
			return VerifyMaterial(a_node, a_file, a_line);
		else if (tagName == L"camera")   //  && objType == L"uvn"
			return VerifyCamera(a_node, a_file, a_line);
		else if (tagName == L"render_settings")   //  && objType == L"uvn"
			return VerifySettings(a_node, a_file, a_line);
		else
		{
			std::wstring errMsg = std::wstring(L"verify, unknown tag name: ") + tagName;
			Error(errMsg.c_str(), a_file, a_line);
			return false;
		}
	}

	static const std::wstring ObjectInfo(const pugi::xml_node a_node)
	{
		const std::wstring tagName = a_node.name();
		const std::wstring id = a_node.attribute(L"id").as_string();
		const std::wstring name = a_node.attribute(L"name").as_string();

		std::wstringstream outStream;
		outStream << L"[VERIFICATION FAILED]: " << tagName.c_str() << L" id = " << id.c_str() << L", name = " << name.c_str() << std::endl;
		return outStream.str();
	}

	static bool ValidFloat3(const std::wstring& colorVal)
	{
		float data[3] = { NAN,NAN,NAN };

		std::wstringstream instr(colorVal);
		instr >> data[0] >> data[1] >> data[2];

		return std::isfinite(data[0]) && std::isfinite(data[1]) && std::isfinite(data[2]);
	}

	static bool VerifyHydraLightIntensity(const pugi::xml_node a_node, const wchar_t* a_file, int a_line)
	{
		const std::wstring ltype = a_node.attribute(L"type").as_string();
		const std::wstring shape = a_node.attribute(L"shape").as_string();
		const std::wstring distr = a_node.attribute(L"distribution").as_string();

		if (a_node.child(L"intensity").child(L"color") == nullptr)
		{
			std::wstring err = ObjectInfo(a_node) + L"Any light must have at least: '<intensity> <color val = \"...\"/> </intensity>'";
			Error(err.c_str(), a_file, a_line);
			return false;
		}
		else
		{
			std::wstring colorVal = a_node.child(L"intensity").child(L"color").attribute(L"val").as_string();
			if (!ValidFloat3(colorVal))
			{
				std::wstring err = ObjectInfo(a_node) + L"intensity.color val attrib is set incorrectly; use ' val = \"0.5 0.5 0.5\" ' ";
				Error(err.c_str(), a_file, a_line);
				return false;
			}
		}

		return true;
	}

	bool VerifyLight(const pugi::xml_node a_node, const wchar_t* a_file, int a_line)
	{
		const std::wstring ltype = a_node.attribute(L"type").as_string();
		const std::wstring shape = a_node.attribute(L"shape").as_string();
		const std::wstring distr = a_node.attribute(L"distribution").as_string();

		if (!VerifyHydraLightIntensity(a_node, a_file, a_line))
			return false;

		if (ltype == L"point")
		{
			if (shape != L"" && shape != L"point")
			{
				std::wstring err = ObjectInfo(a_node) + L"Light with type 'point' may only have 'point' shape or don't set shape at all";
				Error(err.c_str(), a_file, a_line);
				return false;
			}
		}
		else if (ltype == L"area")
		{
			if (shape != L"rect" && shape != L"disk" && shape != L"cylinder" && shape != L"sphere" && shape != L"mesh")
			{
				std::wstring err = ObjectInfo(a_node) + L"Light with type 'area' may only have 'rect', 'disk', 'cylinder' or 'sphere' or 'mesh' shape";
				Error(err.c_str(), a_file, a_line);
				return false;
			}

			if (shape == L"rect")
			{
				pugi::xml_node sizeNode = a_node.child(L"size");

				if (sizeNode == nullptr || sizeNode.attribute(L"half_length") == nullptr || sizeNode.attribute(L"half_width") == nullptr)
				{
					std::wstring err = ObjectInfo(a_node) + L"Light with type 'area' and shape 'rect' has wrong size child or don't have it \n" \
                                                + L"it must be like: <size half_length=\"8.0\" half_width=\"8.0\" />";
					Error(err.c_str(), a_file, a_line);
					return false;
				}
			}
			else if (shape == L"disk")
			{
				pugi::xml_node sizeNode = a_node.child(L"size");
				if (sizeNode == nullptr || sizeNode.attribute(L"radius") == nullptr)
				{
					std::wstring err = ObjectInfo(a_node) + L"Light with type 'area' and shape 'disk' has wrong size child or don't have it \n" \
						                                    + L"it must be like: <size radius=\"2.0\"  />";
					Error(err.c_str(), a_file, a_line);
					return false;
				}
			}
      else if (shape == L"cylinder")
      {
        pugi::xml_node sizeNode = a_node.child(L"size");

        if (sizeNode == nullptr || sizeNode.attribute(L"radius") == nullptr || sizeNode.attribute(L"height") == nullptr)
        {
          std::wstring err = ObjectInfo(a_node) + L"Light with type 'area' and shape 'cylinder' has wrong size child or don't have it \n" \
						                                    + L"it must be like: <size radius=\"2.0\" height=\"3.0\" angle=\"360.0\" />";
					Error(err.c_str(), a_file, a_line);
					return false;
        }

      }

		}
		else if (ltype == L"sphere")
		{
			if (shape != L"" && shape != L"sphere")
			{
				std::wstring err = ObjectInfo(a_node) + L"Light with type 'sphere' may only have 'sphere' shape or don't set shape at all";
				Error(err.c_str(), a_file, a_line);
				return false;
			}

			pugi::xml_node sizeNode = a_node.child(L"size");
			if (sizeNode == nullptr || sizeNode.attribute(L"radius") == nullptr)
			{
				std::wstring err = ObjectInfo(a_node) + L"Light with type 'sphere' has wrong size child or don't have it \n" \
					+ L"it must be like: <size radius=\"2.0\"  />";
				Error(err.c_str(), a_file, a_line);
				return false;
			}
		}
		else if (ltype == L"directional")
		{
			pugi::xml_node sizeNode = a_node.child(L"size");

			if (sizeNode.attribute(L"inner_radius") == nullptr)
			{
				std::wstring err = ObjectInfo(a_node) + L"Light with type 'directional' must have 'size' child with inner_radius attrib \n" + 
					                                      std::wstring(L"like: <size inner_radius = \"0.0f\" outer_radius = \"1000.0f\" />");
				Error(err.c_str(), a_file, a_line);
			}

			if (sizeNode.attribute(L"outer_radius") == nullptr)
			{
				std::wstring err = ObjectInfo(a_node) + L"Light with type 'directional' must have 'size' child with outer_radius attrib \n" + 
					                                      std::wstring(L"like: <size inner_radius = \"0.0f\" outer_radius = \"1000.0f\" />");
				Error(err.c_str(), a_file, a_line);
			}

		}
		else if (ltype == L"sky")
		{
			if (distr == L"perez")
			{
				auto sunModel = a_node.child(L"perez");

				const bool sunOk     = sunModel.attribute(L"sun_id")  != nullptr;
				const bool turbidity = sunModel.attribute(L"turbidity") != nullptr;

				if (!sunOk || !turbidity)
				{
					std::wstring err = ObjectInfo(a_node) + L"ski light with type 'perez' distribution must have 'perez' child \n" + 
					                                      std::wstring(L"like: <perez sun_id = \"1\" turbidity = \"2.0f\" />");
				  Error(err.c_str(), a_file, a_line);
				  return false;
				}
			}
			else if (distr == L"map")
			{
				// check valid texture ???
			}


		}
		else
		{
			std::wstring err = ObjectInfo(a_node) + L"light have unknown type " + ltype;
			Error(err.c_str(), a_file, a_line);
			return false;
		}

		if (distr == L"spot")
		{
			if (a_node.child(L"falloff_angle") == nullptr || a_node.child(L"falloff_angle2") == nullptr)
			{
				std::wstring err = ObjectInfo(a_node) + L"light with 'spot' distrubution must set 'falloff_angle' and 'falloff_angle2' childs";
				Error(err.c_str(), a_file, a_line);
				return false;
			}
			else
			{
				if (a_node.child(L"falloff_angle").attribute(L"val") == nullptr)
				{
					std::wstring err = ObjectInfo(a_node) + L"light.falloff_angle, 'val' attrib is not set";
					Error(err.c_str(), a_file, a_line);
				}

				if (a_node.child(L"falloff_angle2").attribute(L"val") == nullptr)
				{
					std::wstring err = ObjectInfo(a_node) + L"light.falloff_angle2, 'val' attrib is not set";
					Error(err.c_str(), a_file, a_line);
				}

			}

		}
		else if (distr == L"ies")
		{
			// check ies
			if (a_node.child(L"ies").attribute(L"data") == nullptr)
			{
				std::wstring err = ObjectInfo(a_node) + L"light with ies distribution must have ies child with attrib ' data = \"...\" ' ;";
				Error(err.c_str(), a_file, a_line);
				return false;
			}

		}

		return true;
	}

	
	static bool CheckColor(const pugi::xml_node a_node, const pugi::xml_node a_component, const wchar_t* a_file, int a_line)
	{
		if (a_component == nullptr)
			return true;

		pugi::xml_node color = a_component.child(L"color");

		if (color == nullptr || color.attribute(L"val") == nullptr)
		{
			std::wstring err = ObjectInfo(a_node) + L"material child '" + a_component.name() + L"' don't have valid color child with attrib val = \"...\" ";
			Error(err.c_str(), a_file, a_line);
			return false;
		}

		const std::wstring cval = color.attribute(L"val").as_string();

		if (!ValidFloat3(cval))
		{
			std::wstring err = ObjectInfo(a_node) + L"material child '" + a_component.name() + L"' have invalid color val ";
			Error(err.c_str(), a_file, a_line);
			return false;
		}

		return true;
	}


	bool VerifyMaterial(const pugi::xml_node a_node, const wchar_t* a_file, int a_line)
	{
		const std::wstring mtype = a_node.attribute(L"type").as_string();

		if (mtype == L"hydra_material")
		{
			pugi::xml_node emission = a_node.child(L"emission");
			pugi::xml_node diffuse  = a_node.child(L"diffuse");
			pugi::xml_node reflect  = a_node.child(L"reflectivity");
			pugi::xml_node transpar = a_node.child(L"transparency");
			pugi::xml_node sss      = a_node.child(L"translucency");

			if (emission == nullptr && diffuse == nullptr && reflect == nullptr && transpar == nullptr && sss == nullptr)
			{
				std::wstring err = ObjectInfo(a_node) + L"material of type 'hydra_material' must have at least one of these childs: \n(emission, diffuse, reflectivity, transparency, translucency)";
				Error(err.c_str(), a_file, a_line);
				return false;
			}

			CheckColor(a_node, emission, a_file, a_line);
			CheckColor(a_node, diffuse,  a_file, a_line);
			CheckColor(a_node, reflect,  a_file, a_line);
			CheckColor(a_node, transpar, a_file, a_line);
			CheckColor(a_node, sss,      a_file, a_line);
		}

		return true;
	}


	static void CheckChildNotNull(const pugi::xml_node a_node, const wchar_t* a_str, const wchar_t* a_file, int a_line)
	{
		pugi::xml_node child = a_node.child(a_str);

		if (child == nullptr)
		{
			std::wstring err = ObjectInfo(a_node) + L"node '" + a_node.name() + L"' must have child '" + a_str + L"'";
			Error(err.c_str(), a_file, a_line);
		}

	}

	static void CheckFloat3Val(const pugi::xml_node a_node, const wchar_t* a_str, const wchar_t* a_file, int a_line)
	{
		pugi::xml_node child = a_node.child(a_str);
		if (child == nullptr)
			return;

		const std::wstring cval = (child.attribute(L"val") != nullptr) ? child.attribute(L"val").as_string() : child.text().as_string();

		if (!ValidFloat3(cval))
		{
			std::wstring err = ObjectInfo(a_node) + L" child '" + child.name() + L"' don't set valid float3 in 'val' attrib or text";
			Error(err.c_str(), a_file, a_line);
		}
	}


	bool VerifyCamera(const pugi::xml_node a_node, const wchar_t* a_file, int a_line)
	{
		const std::wstring camType = a_node.attribute(L"type").as_string();
		
		if (camType != L"uvn")
		{
			std::wstring err = ObjectInfo(a_node) + L" camera type must be 'uvn' ";
			Error(err.c_str(), a_file, a_line);
			return false;
		}
		else
		{
			CheckChildNotNull(a_node, L"fov", a_file, a_line);
			CheckChildNotNull(a_node, L"nearClipPlane", a_file, a_line);
			CheckChildNotNull(a_node, L"farClipPlane", a_file, a_line);

			CheckChildNotNull(a_node, L"up", a_file, a_line);
			CheckChildNotNull(a_node, L"position", a_file, a_line);
			CheckChildNotNull(a_node, L"look_at", a_file, a_line);

			CheckFloat3Val(a_node, L"up", a_file, a_line);
			CheckFloat3Val(a_node, L"position", a_file, a_line);
			CheckFloat3Val(a_node, L"look_at", a_file, a_line);
		}

		return true;
	}

	bool VerifySettings(const pugi::xml_node a_node, const wchar_t* a_file, int a_line)
	{
		const std::wstring rtype = a_node.attribute(L"type").as_string();

		if (rtype == L"HydraLegacy" || rtype == L"HydraModern" || rtype == L"HydraRTE")
		{
			CheckChildNotNull(a_node, L"width", a_file, a_line);
			CheckChildNotNull(a_node, L"height", a_file, a_line);

			CheckChildNotNull(a_node, L"method_primary", a_file, a_line);
			CheckChildNotNull(a_node, L"method_secondary", a_file, a_line);
			CheckChildNotNull(a_node, L"method_tertiary", a_file, a_line);
			CheckChildNotNull(a_node, L"method_caustic", a_file, a_line);
			CheckChildNotNull(a_node, L"shadows", a_file, a_line);
			CheckChildNotNull(a_node, L"trace_depth", a_file, a_line);
			CheckChildNotNull(a_node, L"diff_trace_depth", a_file, a_line);
			CheckChildNotNull(a_node, L"pt_error", a_file, a_line);
			CheckChildNotNull(a_node, L"minRaysPerPixel", a_file, a_line);
			CheckChildNotNull(a_node, L"maxRaysPerPixel", a_file, a_line);
		}
		else
		{
			CheckChildNotNull(a_node, L"width",  a_file, a_line);
			CheckChildNotNull(a_node, L"height", a_file, a_line);
		}

		return true;
	}



};