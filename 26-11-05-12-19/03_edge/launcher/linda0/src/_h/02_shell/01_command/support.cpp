#include "03_edge/launcher/linda0/src/_h/02_shell/01_command/_support.h"

#include <cstdlib>
#include <system_error>

#include "01_standard/phase00/error.hpp"

using namespace std;
using namespace std::filesystem;

//====----====----====----====----====----====----====----====---- PRIVATE : FDEC

namespace linda0::command
{
  namespace /* private */
  {
    filesystem::path _pathDirSupport(const string& nameKind);
    string _str_nameFileSupport(const filesystem::path& pathDirSupport, const string& nameSupport);
  }
}

//====----====----====----====----====----====----====----====---- PUBLIC : FDEF

namespace linda0::command
{
  TYPE_rc _rc_listAudioSupport(vector<audioSupport::STRUCT_audioSupportInfo>& out_listInfo)
  {
    const filesystem::path pathDirSupport = _pathDirSupport("audio");
    return audioSupport::rc_list(pathDirSupport, out_listInfo);
  }

  TYPE_rc _rc_listLanguageSupport(vector<languageSupport::STRUCT_languageSupportInfo>& out_listInfo)
  {
    const filesystem::path pathDirSupport = _pathDirSupport("language");
    return languageSupport::rc_list(pathDirSupport, out_listInfo);
  }

  TYPE_rc _rc_loadAudioSupport(const string& nameSupport, audioSupport::INTF_audioSupport& out_interface)
  {
    const filesystem::path pathDirSupport = _pathDirSupport("audio");
    const string nameFileDll = _str_nameFileSupport(pathDirSupport, nameSupport);
    return audioSupport::rc_load(pathDirSupport, nameFileDll, out_interface);
  }

  TYPE_rc _rc_loadLanguageSupport(const string& nameSupport, languageSupport::INTF_languageSupport& out_interface)
  {
    const filesystem::path pathDirSupport = _pathDirSupport("language");
    const string nameFileDll = _str_nameFileSupport(pathDirSupport, nameSupport);
    return languageSupport::rc_load(pathDirSupport, nameFileDll, out_interface);
  }
}

//====----====----====----====----====----====----====----====---- PRIVATE : FDEF

namespace linda0::command
{
  namespace /* private */
  {
    filesystem::path _pathDirSupport(const string& nameKind)
    {
      const wchar_t* pathAppData = _wgetenv(L"APPDATA");
      if (pathAppData == nullptr || pathAppData[0] == '\0')
      {
        return {};
      }

      return filesystem::path(pathAppData) / "linda0" / "lsm" / nameKind;
    }

    string _str_nameFileSupport(const filesystem::path& pathDirSupport, const string& nameSupport)
    {
      if (pathDirSupport.empty() || nameSupport.empty())
      {
        return nameSupport;
      }

      if (nameSupport.find_first_of("\\/:") != string::npos)
      {
        return nameSupport;
      }

      error_code ecFilesystem;
      const filesystem::path pathFileRequested = pathDirSupport / (nameSupport + ".dll");
      const bool isRequestedFile = filesystem::is_regular_file(pathFileRequested, ecFilesystem);
      if (isRequestedFile && !ecFilesystem)
      {
        return nameSupport;
      }

      ecFilesystem.clear();
      const string nameFileFallback = "linda." + nameSupport;
      const filesystem::path pathFileFallback = pathDirSupport / (nameFileFallback + ".dll");
      const bool isFallbackFile = filesystem::is_regular_file(pathFileFallback, ecFilesystem);
      if (isFallbackFile && !ecFilesystem)
      {
        return nameFileFallback;
      }

      return nameSupport;
    }
  }
}
