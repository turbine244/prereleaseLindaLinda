#include "03_edge/launcher/linda0/src/_h/01_runtime/00_audioSupport/_audioSupport.h"

#include <Windows.h>

#include <filesystem>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "01_standard/phase00/error.hpp"

using namespace std;

//====----====----====----====----====----====----====----====---- FDEC

namespace linda0::audioSupport
{
  namespace /* private */
  {
    bool _is_dllFile(const filesystem::path& pathFile);
    bool _is_audioSupportInterfaceValid(HMODULE handleModule, INTF_audioSupport& out_interface);
  }
}

//====----====----====----====----====----====----====----====---- FDEF

namespace linda0::audioSupport
{
  TYPE_rc rc_list(const filesystem::path& pathDirLsm, vector<STRUCT_audioSupportInfo>& out_listInfo)
  {
    // NULLIFY : rc, out
    out_listInfo.clear();
    vector<STRUCT_audioSupportInfo> temp_listInfo;
    TYPE_rc rci = 0;
    error_code ecFilesystem;

    // RCI : validate the support directory
    rci--;
    if (pathDirLsm.empty() || !filesystem::is_directory(pathDirLsm, ecFilesystem) || ecFilesystem)
    {
      vd_printErrorRc(rci, "Audio LSM directory is unavailable: " + pathDirLsm.string());
      return rci;
    }

    filesystem::directory_iterator iterator(pathDirLsm, ecFilesystem);
    const filesystem::directory_iterator end;
    rci--;
    if (ecFilesystem)
    {
      vd_printErrorRc(rci, "Failed enumerating audio LSM directory: " + pathDirLsm.string());
      return rci;
    }

    while (iterator != end)
    {
      const filesystem::path pathFileDll = iterator->path();
      const bool isDllFile = _is_dllFile(pathFileDll);
      bool isRegularFile = false;

      if (isDllFile)
      {
        ecFilesystem.clear();
        isRegularFile = filesystem::is_regular_file(pathFileDll, ecFilesystem);
      }

      if (isDllFile && isRegularFile && !ecFilesystem)
      {
        HMODULE handleModule = LoadLibraryW(pathFileDll.c_str());
        if (handleModule != nullptr)
        {
          INTF_audioSupport temp_interface = {};
          const bool isAudioSupportValid = _is_audioSupportInterfaceValid(handleModule, temp_interface);
          if (isAudioSupportValid)
          {
            temp_interface.info.pathFileDll = pathFileDll;
            temp_listInfo.push_back(temp_interface.info);
          }

          if (FreeLibrary(handleModule) == 0)
          {
            rci--;
            vd_printErrorRc(rci, "Failed releasing inspected audio support DLL: " + pathFileDll.string());
            return rci;
          }
        }
      }

      ecFilesystem.clear();
      iterator.increment(ecFilesystem);
      if (ecFilesystem)
      {
        rci--;
        vd_printErrorRc(rci, "Failed enumerating audio LSM directory: " + pathDirLsm.string());
        return rci;
      }
    }

    out_listInfo = move(temp_listInfo);

    // END
    return 0;
  }

  TYPE_rc rc_load(const filesystem::path& pathDirLsm, const string& nameFileDll, INTF_audioSupport& out_interface)
  {
    // NULLIFY : rc
    TYPE_rc rci = 0;
    error_code ecFilesystem;

    // RCI : require an empty output interface
    rci--;
    if (out_interface.handle != nullptr)
    {
      vd_printErrorRc(rci, "Audio support interface is already loaded");
      return rci;
    }

    // NULLIFY : out
    out_interface = {};

    // RCI : validate the DLL file name without its extension
    rci--;
    const bool isFileNameValid =
    {
      !nameFileDll.empty()
      && nameFileDll.find_first_of("\\/:") == string::npos
      && nameFileDll != "."
      && nameFileDll != ".."
      && !(nameFileDll.size() > 4 && nameFileDll.substr(nameFileDll.size() - 4) == ".dll")
    };
    if (!isFileNameValid)
    {
      vd_printErrorRc(rci, "Audio support DLL file name is invalid: " + nameFileDll);
      return rci;
    }

    // RCI : validate the support directory
    rci--;
    if (pathDirLsm.empty() || !filesystem::is_directory(pathDirLsm, ecFilesystem) || ecFilesystem)
    {
      vd_printErrorRc(rci, "Audio LSM directory is unavailable: " + pathDirLsm.string());
      return rci;
    }

    filesystem::path pathFileSupport = pathDirLsm / (nameFileDll + ".dll");
    ecFilesystem.clear();
    const bool isSupportFile = filesystem::is_regular_file(pathFileSupport, ecFilesystem);
    rci--;
    if (ecFilesystem)
    {
      vd_printErrorRc(rci, "Failed checking Audio support DLL: " + pathFileSupport.string());
      return rci;
    }

    // RCI : require the exact DLL file selected by the caller
    rci--;
    if (!isSupportFile)
    {
      vd_printErrorRc(rci, "Audio support DLL not found: " + pathFileSupport.string());
      return rci;
    }

    HMODULE handleModule = LoadLibraryW(pathFileSupport.c_str());
    rci--;
    if (handleModule == nullptr)
    {
      const DWORD errorWindows = GetLastError();
      vd_printErrorRc(rci, "Failed loading Audio support DLL: " + pathFileSupport.string() + " (Windows error " + to_string(errorWindows) + ")");
      return rci;
    }

    INTF_audioSupport temp_interface = {};
    const bool isSupportValid = _is_audioSupportInterfaceValid(handleModule, temp_interface);
    rci--;
    if (!isSupportValid)
    {
      FreeLibrary(handleModule);
      vd_printErrorRc(rci, "Audio support DLL has an incompatible interface: " + pathFileSupport.string());
      return rci;
    }

    temp_interface.handle = handleModule;
    temp_interface.info.pathFileDll = pathFileSupport;
    out_interface = move(temp_interface);

    // END
    return 0;
  }
  TYPE_rc rc_free(INTF_audioSupport& edit_interface)
  {
    // NULLIFY : rc
    TYPE_rc rci = 0;

    if (edit_interface.handle != nullptr)
    {
      HMODULE handleModule = static_cast<HMODULE>(edit_interface.handle);
      rci--;
      if (FreeLibrary(handleModule) == 0)
      {
        const DWORD errorWindows = GetLastError();
        vd_printErrorRc(rci, "Failed releasing audio support DLL (Windows error " + to_string(errorWindows) + ")");
        return rci;
      }
    }

    edit_interface = {};

    // END
    return 0;
  }

  namespace /* private */
  {
    bool _is_dllFile(const filesystem::path& pathFile)
    {
      const wstring strExtension = pathFile.extension().wstring();
      return CompareStringOrdinal(strExtension.c_str(), -1, L".dll", -1, TRUE) == CSTR_EQUAL;
    }

    bool _is_audioSupportInterfaceValid(HMODULE handleModule, INTF_audioSupport& out_interface)
    {
      out_interface = {};

      const lsm::CALLBACK_vd_getInfo callback_getInfo = reinterpret_cast<lsm::CALLBACK_vd_getInfo>(GetProcAddress(handleModule, "vd_getInfo"));
      out_interface.rc_listDevice = reinterpret_cast<lsm::audio::CALLBACK_rc_listDevice>(GetProcAddress(handleModule, "rc_listDevice"));
      out_interface.rc_scanChannel = reinterpret_cast<lsm::audio::CALLBACK_rc_scanChannel>(GetProcAddress(handleModule, "rc_scanChannel"));
      out_interface.rc_openAudioRuntime = reinterpret_cast<lsm::audio::CALLBACK_rc_openAudioRuntime>(GetProcAddress(handleModule, "rc_openAudioRuntime"));
      out_interface.rc_bindAudioRuntime = reinterpret_cast<lsm::audio::CALLBACK_rc_bindAudioRuntime>(GetProcAddress(handleModule, "rc_bindAudioRuntime"));
      out_interface.rc_setSps = reinterpret_cast<lsm::audio::CALLBACK_rc_setSps>(GetProcAddress(handleModule, "rc_setSps"));
      out_interface.rc_startAudioRuntime = reinterpret_cast<lsm::audio::CALLBACK_rc_startAudioRuntime>(GetProcAddress(handleModule, "rc_startAudioRuntime"));
      out_interface.rc_stopAudioRuntime = reinterpret_cast<lsm::audio::CALLBACK_rc_stopAudioRuntime>(GetProcAddress(handleModule, "rc_stopAudioRuntime"));
      out_interface.vd_closeAudioRuntime = reinterpret_cast<lsm::audio::CALLBACK_vd_closeAudioRuntime>(GetProcAddress(handleModule, "vd_closeAudioRuntime"));

      const bool isAudioSupportInterfaceComplete =
      {
        callback_getInfo != nullptr
        && out_interface.rc_listDevice != nullptr
        && out_interface.rc_scanChannel != nullptr
        && out_interface.rc_openAudioRuntime != nullptr
        && out_interface.rc_bindAudioRuntime != nullptr
        && out_interface.rc_setSps != nullptr
        && out_interface.rc_startAudioRuntime != nullptr
        && out_interface.rc_stopAudioRuntime != nullptr
        && out_interface.vd_closeAudioRuntime != nullptr
      };
      if (!isAudioSupportInterfaceComplete)
      {
        out_interface = {};
        return false;
      }

      lsm::STRUCT_lsmInfo infoSupport = {};
      callback_getInfo(infoSupport);
      const bool isAudioSupportInfoValid =
      {
        infoSupport.kind == lsm::ENUM_lsmKind::audio
        && infoSupport.name != nullptr
        && infoSupport.name[0] != '\0'
      };
      if (!isAudioSupportInfoValid)
      {
        out_interface = {};
        return false;
      }

      out_interface.info.name = infoSupport.name;
      out_interface.info.versionMajor = infoSupport.versionMajor;
      out_interface.info.versionMinor = infoSupport.versionMinor;
      out_interface.info.versionPatch = infoSupport.versionPatch;
      out_interface.info.description = infoSupport.description;

      // END
      return true;
    }
  }
}

