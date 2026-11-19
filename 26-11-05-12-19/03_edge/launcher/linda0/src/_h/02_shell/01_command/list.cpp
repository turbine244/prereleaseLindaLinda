#include "03_edge/launcher/linda0/src/_h/02_shell/01_command/_list.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "01_standard/phase00/error.hpp"
#include "03_edge/launcher/linda0/src/_h/02_shell/01_command/_support.h"

using namespace std;
using namespace linda0::cli;

//====----====----====----====----====----====----====----====---- PRIVATE : FDEC

namespace
{
  TYPE_rc _rc_listSupport(const STRUCT_lineParsed& line);
  TYPE_rc _rc_listAudioSupport();
  TYPE_rc _rc_listLanguageSupport();
  TYPE_rc _rc_listDevice(const STRUCT_lineParsed& line);
  void _vd_printAudioSupport(const vector<linda0::audioSupport::STRUCT_audioSupportInfo>& listInfo);
  void _vd_printLanguageSupport(const vector<linda0::languageSupport::STRUCT_languageSupportInfo>& listInfo);
}

//====----====----====----====----====----====----====----====---- PUBLIC : FDEF

namespace linda0::command::list
{
  TYPE_rc rc_commandList(const cli::STRUCT_lineParsed& line)
  {
    if (line.subCommand == "support")
    {
      return to_abs(_rc_listSupport(line));
    }

    if (line.subCommand == "device")
    {
      return to_abs(_rc_listDevice(line));
    }

    TYPE_rc rci = 0;
    rci--;
    vd_printErrorRc(rci, "Unknown list subcommand");
    return rci;
  }
}

//====----====----====----====----====----====----====----====---- PRIVATE : FDEF

namespace
{
  TYPE_rc _rc_listSupport(const STRUCT_lineParsed& line)
  {
    const bool isLanguageRequested = line.setOptionFlag.contains("--language");
    const bool isAudioRequested = line.setOptionFlag.contains("--audio");
    const bool isAll = !isLanguageRequested && !isAudioRequested;

    if (isLanguageRequested || isAll)
    {
      const TYPE_rc rce = to_abs(_rc_listLanguageSupport());
      if (rce != 0)
      {
        return rce;
      }
    }

    if (isAudioRequested || isAll)
    {
      const TYPE_rc rce = to_abs(_rc_listAudioSupport());
      if (rce != 0)
      {
        return rce;
      }
    }

    // END
    return 0;
  }

  TYPE_rc _rc_listAudioSupport()
  {
    vector<linda0::audioSupport::STRUCT_audioSupportInfo> listInfo;
    const TYPE_rc rce = to_abs(linda0::command::_rc_listAudioSupport(listInfo));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed listing audio support modules");
      return rce;
    }

    _vd_printAudioSupport(listInfo);

    // END
    return 0;
  }

  TYPE_rc _rc_listLanguageSupport()
  {
    vector<linda0::languageSupport::STRUCT_languageSupportInfo> listInfo;
    const TYPE_rc rce = to_abs(linda0::command::_rc_listLanguageSupport(listInfo));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed listing language support modules");
      return rce;
    }

    _vd_printLanguageSupport(listInfo);

    // END
    return 0;
  }

  TYPE_rc _rc_listDevice(const STRUCT_lineParsed& line)
  {
    TYPE_rc rci = 0;
    const auto itSupport = line.mapOperand.find("support");
    rci--;
    if (itSupport == line.mapOperand.end())
    {
      vd_printErrorRc(rci, "Audio support name is missing");
      return rci;
    }

    linda0::audioSupport::INTF_audioSupport interfaceAudio = {};
    TYPE_rc rce = to_abs(linda0::command::_rc_loadAudioSupport(itSupport->second, interfaceAudio));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed loading audio support: " + itSupport->second);
      return rce;
    }

    rci--;
    if (interfaceAudio.rc_listDevice == nullptr)
    {
      const TYPE_rc rceFree = to_abs(linda0::audioSupport::rc_free(interfaceAudio));
      if (rceFree != 0)
      {
        return rceFree;
      }
      vd_printErrorRc(rci, "Device listing is not supported by: " + itSupport->second);
      return rci;
    }

    vector<lsm::audio::STRUCT_audioDeviceInfo> listAudioDeviceInfo;
    rce = to_abs(interfaceAudio.rc_listDevice(listAudioDeviceInfo));
    const TYPE_rc rceFree = to_abs(linda0::audioSupport::rc_free(interfaceAudio));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed listing devices for: " + itSupport->second);
      return rce;
    }
    if (rceFree != 0)
    {
      return rceFree;
    }

    if (listAudioDeviceInfo.empty())
    {
      cout << "No devices found." << endl;
      return 0;
    }

    size_t widthName = string("NAME").size();
    for (const auto& device : listAudioDeviceInfo)
    {
      widthName = max(widthName, device.name.size());
    }

    cout
      << left << setw(static_cast<int>(widthName + 2)) << "NAME"
      << right << setw(7) << "INPUT"
      << setw(8) << "OUTPUT"
      << endl;

    for (const auto& device : listAudioDeviceInfo)
    {
      const string numInput = device.numInput == 0 ? "-" : to_string(device.numInput);
      const string numOutput = device.numOutput == 0 ? "-" : to_string(device.numOutput);
      cout
        << left << setw(static_cast<int>(widthName + 2)) << device.name
        << right << setw(7) << numInput
        << setw(8) << numOutput
        << endl;
    }

    // END
    return 0;
  }

  void _vd_printAudioSupport(const vector<linda0::audioSupport::STRUCT_audioSupportInfo>& listInfo)
  {
    cout << "AUDIO SUPPORT" << endl;
    for (const auto& info : listInfo)
    {
      cout << "  " << info.name << " "
        << info.versionMajor << "." << info.versionMinor << "." << info.versionPatch;
      if (!info.description.empty())
      {
        cout << " : " << info.description;
      }
      cout << endl;
    }
    if (listInfo.empty())
    {
      cout << "  No audio support modules found." << endl;
    }

    // END
    return;
  }

  void _vd_printLanguageSupport(const vector<linda0::languageSupport::STRUCT_languageSupportInfo>& listInfo)
  {
    cout << "LANGUAGE SUPPORT" << endl;
    for (const auto& info : listInfo)
    {
      cout << "  " << info.name << " "
        << info.versionMajor << "." << info.versionMinor << "." << info.versionPatch;
      if (!info.description.empty())
      {
        cout << " : " << info.description;
      }
      cout << endl;
    }
    if (listInfo.empty())
    {
      cout << "  No language support modules found." << endl;
    }

    // END
    return;
  }
}
