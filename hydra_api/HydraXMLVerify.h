#pragma once

#include <cstdint>
#include "pugixml.hpp"

namespace HydraVerify
{
	using ErrorFuncType = void(*)(const wchar_t* a_msg, const wchar_t* a_file, int a_line);

	void SetErrorCallback(ErrorFuncType a_func);
	void Error(const wchar_t* a_msg, const wchar_t* a_file, int a_line);

	bool VerifyLight   (const pugi::xml_node a_node, const wchar_t* a_file, int a_line);
	bool VerifyMaterial(const pugi::xml_node a_node, const wchar_t* a_file, int a_line);
	bool VerifyCamera  (const pugi::xml_node a_node, const wchar_t* a_file, int a_line);
	bool VerifySettings(const pugi::xml_node a_node, const wchar_t* a_file, int a_line);

	bool VerifyXMLParameter(const pugi::xml_node a_node, const wchar_t* a_file, int a_line);
	bool VerifyXMLParameter(const pugi::xml_node a_node, const char* a_file, int a_line);
};


#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
#define VERIFY_XML(node) HydraVerify::VerifyXMLParameter(node, __FILE__, __LINE__);
#elif defined WIN32
#define VERIFY_XML(node) HydraVerify::VerifyXMLParameter(node, __FILEW__, __LINE__);
#endif