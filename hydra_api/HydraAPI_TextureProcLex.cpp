#include "HydraAPI.h"
#include "HydraInternal.h"
#include "HydraInternalCommon.h"

#include <memory>
#include <vector>
#include <string>
#include <map>

#include <sstream>
#include <fstream>
#include <iomanip>

//#include "HydraObjectManager.h"
#include "HydraLegacyUtils.h"
#include "xxhash.h"


#include <unordered_map>
#include <regex>
#include <limits>       

#include <memory> 
#include <math.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string tokenregex = "\\s+|\\,|\\(|\\)|\\;";

std::string ReplaceAttr(std::string a_line)
{
  a_line = std::regex_replace(a_line, std::regex("readAttr\\(sHit\\,\\s*\"TexCoord0\"\\)"), "readAttr_TexCoord0(sHit)");
  a_line = std::regex_replace(a_line, std::regex("readAttr\\(sHit\\,\\s*\"TexCoord1\"\\)"), "readAttr_TexCoord1(sHit)");
  a_line = std::regex_replace(a_line, std::regex("readAttr\\(sHit\\,\\s*\"WorldPos\"\\)"), "readAttr_WorldPos(sHit)");
  a_line = std::regex_replace(a_line, std::regex("readAttr\\(sHit\\,\\s*\"ShadeNorm\"\\)"), "readAttr_ShadeNorm(sHit)");
  a_line = std::regex_replace(a_line, std::regex("readAttr\\(sHit\\,\\s*\"AO\"\\)"), "readAttr_AO(sHit)");

  a_line = std::regex_replace(a_line, std::regex("readAttr\\(sHit\\,\\s*\"(.*)\"\\)"), "readAttr_Custom(0)"); // #TODO: insert xxhash32 here.

  return a_line;
}

std::string FunctionDeclRegex()
{
  // <Identifier> = [a-zA-Z_][a-zA-Z0-9_]
  // <Whitespace> = [\n\r\s]
  // <Anything>   = .

  // + -- 1 or more
  // * -- 0 or more

  // <Identifier><Whitespace><Identifier>(<Anthing>)<Whitespace>{
  //      <Identifier>            <Whitespace>       <Identifier>      (<Anthing>)<Whitespace> {
  return "([a-zA-Z_][a-zA-Z0-9_]+)([\\n|\\s]+)([a-zA-Z_][a-zA-Z0-9_]+)\\((.*)\\)([\\n|\\s]*)\\{";
}

std::string FunctionDeclRegex(const std::string& fname)
{
  // <Identifier> <Whitespace> <fname> (<Anthing>) <Whitespace> {
  return std::string("([a-zA-Z_][a-zA-Z0-9_]+)([\\n|\\s]+)") + fname + "\\((.*)\\)([\\n|\\s]*)\\{";
}

std::string IncludeRegex()
{
  //      #include <Whitespace> "<Anything>"
  return "#include([\\n|\\s]+)\"(.*)\"";
}

std::vector<std::string> ExtractElementsByRegex(const std::string& allString, const std::string& a_regex, const int elemNumber)
{
  std::vector<std::string> funNames;

  std::smatch m;
  std::regex findFuncDelc(a_regex);

  auto searchStart = allString.cbegin();
  while (std::regex_search(searchStart, allString.cend(), m, findFuncDelc))
  {
    funNames.push_back(m[elemNumber].str());
    searchStart += m.position() + m.length();
  }

  return funNames;
}

struct Arg
{
  std::string type;
  std::string name;
  int         size; ///< array size if this is array
};


int SizeOfInputTypeInWords(const std::string& a_type)
{
  if (a_type == "float4" || a_type == "int4")
    return 4;
  else if (a_type == "float3" || a_type == "int3")
    return 3;
  else if (a_type == "float2" || a_type == "int2")
    return 2;
  else
    return 1;
}

void PopArgument(const std::string& argList, std::vector<Arg>& a_outList)
{
  std::smatch m;
  std::regex args("([a-zA-Z_][a-zA-Z0-9_]+)([\\n|\\s]+)([a-zA-Z_][a-zA-Z0-9_]+)(\\[[0-9_]\\])*(\\,|\\))(.*)");
  std::regex_search(argList, m, args);

  if (m.size() >= 3)
  {
    Arg arg;
    arg.type = m[1].str();
    arg.name = m[3].str();
    arg.size = 1;

    if (m.size() >= 4 && m[4].str().size() > 0)
    {
      std::string arraySize = m[4].str().substr(1, m[4].str().size() - 2);
      arg.size = atoi(arraySize.c_str())*SizeOfInputTypeInWords(arg.type);
    }

    a_outList.push_back(arg);
  }
  else
  {
    std::regex args2("([a-zA-Z_][a-zA-Z0-9_]+)([\\n|\\s]+)([a-zA-Z_][a-zA-Z0-9_]+)(\\[[0-9_]\\])*(.*)");
    std::regex_search(argList, m, args2);
    if (m.size() >= 2)
    {
      Arg arg;
      arg.type = m[1].str();
      arg.name = m[3].str();
      arg.size = 1;

      if (m.size() >= 5 && m[5].str().size() > 0)
      {
        std::string arraySize = m[5].str().substr(1, m[5].str().size() - 2);
        arg.size = atoi(arraySize.c_str());
      }

      a_outList.push_back(arg);
    }
  }

  if (m.size() >= 6)
    PopArgument(m[6].str(), a_outList);
}

std::vector<Arg> ParseProcMainArgs(const std::string& fname, const std::string& allString)
{
  std::vector<Arg> res;

  std::smatch m;
  std::regex funcDelc(FunctionDeclRegex(fname));
  std::regex_search(allString, m, funcDelc);

  PopArgument(m[3].str(), res);

  return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProcessProcTexFile(const std::wstring& in_file, const std::wstring& out_file, const std::wstring& mainName, const std::wstring& a_prefix,
                        pugi::xml_node a_node)
{
  const std::string in_fileS  = ws2s(in_file);
  const std::string out_fileS = ws2s(out_file);
  const std::string fname     = ws2s(mainName);
  const std::string prefix    = "prtex_" + ws2s(a_prefix);

  std::ifstream fin(in_fileS.c_str());
  std::ofstream fout(out_fileS.c_str());

  if (!fin.is_open())
  {
    std::cout << "can't open file" << std::endl;
    return;
  }

  std::string allString;
  allString.assign((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
  fin.seekg(0, std::ios::beg);

  // first we will find all functions names and include files
  //
  std::vector<std::string> funNames = ExtractElementsByRegex(allString, FunctionDeclRegex(), 3);
  std::vector<std::string> incFiles = ExtractElementsByRegex(allString, IncludeRegex(), 2);

  auto args = ParseProcMainArgs(fname.c_str(), allString);

  // next we have to read inputs from XML
  //

  for (auto f : funNames)
    std::cout << f.c_str() << std::endl;

  try
  {
    // transform code
    //
    std::string line;
    while (std::getline(fin, line))
    {
      std::string line2 = std::regex_replace(line, std::regex("\\bsampler2D\\b"), "const int");

      line2 = ReplaceAttr(line2);

      for (auto f : funNames)
      {
        std::regex e("\\b" + f + "\\b");
        line2 = std::regex_replace(line2, e, prefix + f);
      }

      //std::regex inclDelc(IncludeRegex());
      //std::regex_search(line2, m, inclDelc);
      //if (m.size() > 2)
      //  std::cout << m[2] << std::endl;
      //
      fout << line2.c_str() << std::endl;
    }

    // generate function call
    //
    fout << std::endl << std::endl;

    const std::string fname2 = prefix + fname;

    std::stringstream genStream;

    int currOffset = 0;
    genStream << fname2.c_str() << "(sHit, ";

    for (size_t i = 0; i<args.size(); i++)
    {
      pugi::xml_node argNode = a_node.append_child(L"arg");

      std::wstring type = s2ws(args[i].type);
      std::wstring name = s2ws(args[i].name);

      argNode.append_attribute(L"id")      = i;
      argNode.append_attribute(L"type")    = type.c_str();
      argNode.append_attribute(L"name")    = name.c_str();
      argNode.append_attribute(L"size")    = args[i].size;
      argNode.append_attribute(L"wsize")   = args[i].size*SizeOfInputTypeInWords(args[i].type);
      argNode.append_attribute(L"woffset") = currOffset;

      if (args[i].size > 1)
      {
        genStream << "(__global " << args[i].type.c_str() << "*" << ")(pMat->data + " << currOffset << ")";
        currOffset += args[i].size;
      }
      else
      {
        if (args[i].type == "float4")
        {
          genStream << "make_float4(pMat->data[" << currOffset + 0 << "], " \
                                << "pMat->data[" << currOffset + 1 << "], " \
                                << "pMat->data[" << currOffset + 2 << "], " \
                                << "pMat->data[" << currOffset + 3 << "]" << ")";
          currOffset += 4;
        }
        else if (args[i].type == "float3")
        {
          genStream << "make_float3(pMat->data[" << currOffset + 0 << "], " \
                                << "pMat->data[" << currOffset + 1 << "], " \
                                << "pMat->data[" << currOffset + 2 << "]" << ")";
          currOffset += 3;
        }
        else if (args[i].type == "float2")
        {
          genStream << "make_float2(pMat->data[" << currOffset + 0 << "], " << "pMat->data[" << currOffset + 1 << "]" << ")";
          currOffset += 2;
        }
        else if (args[i].type == "int" || args[i].type == "sampler2D")
        {
          genStream << "as_int(pMat->data[" << currOffset << "])";
          currOffset++;
        }
        else
        {
          genStream << "pMat->data[" << currOffset << "]";
          currOffset++;
        }
      }

      if (i != args.size() - 1)
        genStream << ", ";
      else
        genStream << ");";

    }


    std::wstring fnCall = s2ws(genStream.str());
    a_node.append_child(L"call").text() = fnCall.c_str();

  }
  catch (...)
  {
    std::cout << "probably bad regex or some other exception" << std::endl;
  }

}