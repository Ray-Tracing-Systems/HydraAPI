//
// Created by vsan on 21.01.18.
//

#include <unistd.h>
#include <spawn.h>
#include <csignal>
#include "HydraLegacyUtils.h"

#include <sys/types.h>
#include <pwd.h>

struct HydraProcessLauncher : IHydraNetPluginAPI
{
  HydraProcessLauncher(const char* imageFileName, int width, int height, const char* connectionType, std::ostream* a_pLog = nullptr);
  ~HydraProcessLauncher();

  bool hasConnection() const override;

  void runAllRenderProcesses(RenderProcessRunParams a_params, const std::vector<HydraRenderDevice>& a_devList, const std::vector<int>& a_activeDevices, bool a_appendMode) override;
  void stopAllRenderProcesses() override;

protected:

  bool m_hydraServerStarted;
  std::ostream* m_pLog;
  
  std::vector<pid_t> m_mdProcessList;

  std::string m_connectionType;
  std::string m_imageFileName;

  int m_width;
  int m_height;
};


static std::ofstream g_logMain;

IHydraNetPluginAPI* CreateHydraServerConnection(int renderWidth, int renderHeight, bool inMatEditor)
{
  IHydraNetPluginAPI* pImpl = nullptr;
  long ticks = sysconf(_SC_CLK_TCK);

  std::stringstream ss;
  ss << ticks;

  std::string imageName   = std::string("HydraHDRImage_") + ss.str();
  std::string messageName = std::string("HydraMessageShmem_") + ss.str();
  std::string guiName     = std::string("HydraGuiShmem_") + ss.str();

  std::ostream* logPtr = nullptr;

  if (!inMatEditor)
  {
    if (!g_logMain.is_open())
      g_logMain.open("/home/vsan/test/log.txt");
    logPtr = &g_logMain;
    pImpl  = new HydraProcessLauncher(imageName.c_str(), renderWidth, renderHeight, "main", logPtr);
  }
  else // if in matEditor
  {

  }

  if (pImpl->hasConnection())
    return pImpl;
  else
  {
    delete pImpl;
    return nullptr;
  }

}



HydraProcessLauncher::HydraProcessLauncher(const char* imageFileName, int width, int height, const char* connectionType, std::ostream* a_pLog) :
                                           m_imageFileName(imageFileName), m_connectionType(connectionType), m_width(width), m_height(height), m_hydraServerStarted(false), m_pLog(a_pLog)
{
  m_mdProcessList.clear();
}

HydraProcessLauncher::~HydraProcessLauncher()
{
  stopAllRenderProcesses();
}

bool HydraProcessLauncher::hasConnection() const
{
  return true;
}

#include <thread>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>
#include <cstring>
#include <algorithm>

char* CommandLineToArgv(const std::string& line, int a_maxArgs,
                        int& argc, char** argv)
{
  typedef std::vector<char*> CharPtrVector;
  char const * WHITESPACE_STR = " \n\r\t";
  char const SPACE = ' ';
  char const TAB = '\t';
  char const DQUOTE = '\"';
  char const SQUOTE = '\'';
  char const TERM = '\0';
  
  //--------------------------------------------------------------------------
  // Copy the command line string to a character array.
  // strdup() uses malloc() to get memory for the new string.
#if defined( WIN32 )
  char * pLine = _strdup( line.c_str() );
#else
  char * pLine = strdup( line.c_str() );
#endif
  
  //--------------------------------------------------------------------------
  // Crawl the character array and tokenize in place.
  CharPtrVector tokens;
  char * pCursor = pLine;
  while ( *pCursor )
  {
    // Whitespace.
    if ( *pCursor == SPACE || *pCursor == TAB )
      ++pCursor;
    else if ( *pCursor == DQUOTE ) // Double quoted token.
    {
      // Begin of token is one char past the begin quote.
      // Replace the quote with whitespace.
      tokens.push_back( pCursor + 1 );
      *pCursor = SPACE;
      
      char * pEnd = strchr( pCursor + 1, DQUOTE );
      if ( pEnd )
      {
        // End of token is one char before the end quote.
        // Replace the quote with terminator, and advance cursor.
        *pEnd = TERM;
        pCursor = pEnd + 1;
      }
      else
      {
        // End of token is end of line.
        break;
      }
    }
    // Single quoted token.
    else if ( *pCursor == SQUOTE )
    {
      // Begin of token is one char past the begin quote.
      // Replace the quote with whitespace.
      tokens.push_back( pCursor + 1 );
      *pCursor = SPACE;
      
      char * pEnd = strchr( pCursor + 1, SQUOTE );
      if ( pEnd )
      {
        // End of token is one char before the end quote.
        // Replace the quote with terminator, and advance cursor.
        *pEnd = TERM;
        pCursor = pEnd + 1;
      }
      else
      {
        // End of token is end of line.
        break;
      }
    }
    // Unquoted token.
    else
    {
      // Begin of token is at cursor.
      tokens.push_back( pCursor );
      
      char * pEnd = strpbrk( pCursor + 1, WHITESPACE_STR );
      if ( pEnd )
      {
        // End of token is one char before the next whitespace.
        // Replace whitespace with terminator, and advance cursor.
        *pEnd = TERM;
        pCursor = pEnd + 1;
      }
      else
      {
        // End of token is end of line.
        break;
      }
    }
  }
  
  //--------------------------------------------------------------------------
  // Fill the argv array.
  argc = std::min<int>(int(tokens.size()), a_maxArgs);
  int a = 0;
  for (CharPtrVector::const_iterator it = tokens.begin(); it != tokens.end(); ++it )
    argv[a++] = (*it);

  return pLine;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CreateProcessUnix(const char* exePath, const char* allArgs, const bool a_debug, std::ostream* a_pLog)
{
  //std::string command = std::string(exePath) + " " + std::string(allArgs) + " &";
  //system(command.c_str());
  
  std::vector<char*> argv(256);
  int argc = 0;
  
  argv[0] = (char*)exePath;
  char* pLine = CommandLineToArgv(allArgs, 256,
                                  argc, argv.data() + 1);
  
  int pid = fork();
  if(pid == 0)
  {
    execvp(exePath, argv.data());
    free(pLine);
    exit(0);
    return 0;
  }
  else
  {
    free(pLine);
    return pid;
  }
}

void HydraProcessLauncher::runAllRenderProcesses(RenderProcessRunParams a_params, const std::vector<HydraRenderDevice>& a_devList, const std::vector<int>& a_activeDevices, bool a_appendMode)
{
  signal(SIGCHLD, SIG_IGN); // Silently (and portably) reap children
  
  if (m_connectionType == "main")
  {
    //char user_name[L_cuserid];
    //cuserid(user_name);

    auto username = getpwuid(geteuid());
    std::stringstream ss;
    ss << username->pw_dir << "/hydra/";

    std::string hydraPath = ss.str();
    if (a_params.customExePath != "")
      hydraPath = a_params.customExePath;

    if (!isFileExist(hydraPath.c_str()))
    {
      m_hydraServerStarted = false;
    }
    else
    {
      ss.str(std::string());
      ss << "-nowindow 1 ";
      ss << a_params.customExeArgs.c_str();
      if(!a_params.customLogFold.empty())
        ss << " -logdir \"" << a_params.customLogFold.c_str() << "\" ";
      
      std::string basicCmd = ss.str();

      m_hydraServerStarted = true;
      std::ofstream fout(hydraPath + "zcmd.txt");
  
      if(!a_appendMode)
        m_mdProcessList.resize(0);
      
      for (int devId : a_activeDevices)
      {
        ss.str(std::string());
        ss << " -cl_device_id " << devId;

        std::string cmdFull = basicCmd + ss.str();
        std::string hydraExe(hydraPath + "hydra");
  
        if(!a_params.debug)
          m_mdProcessList.push_back(CreateProcessUnix(hydraExe.c_str(), cmdFull.c_str(), a_params.debug, m_pLog));
        fout << cmdFull.c_str() << std::endl;
      }

      fout.close();
    }
  }
}

void HydraProcessLauncher::stopAllRenderProcesses()
{
  if (m_hydraServerStarted)
  {
    for (auto pid : m_mdProcessList)
    {
      if (pid <= 0)
        continue;

      kill(pid, SIGILL);
    }
  
    m_mdProcessList.resize(0);
  }
}
