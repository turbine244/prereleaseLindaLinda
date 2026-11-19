#include "03_edge/launcher/linda0/src/_h/02_shell/01_command/_scan.h"

#include <filesystem>
#include <iostream>
#include <string>

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/json.hpp"
#include "01_standard/phase01/ioFile.hpp"
#include "03_edge/launcher/linda0/src/_h/02_shell/01_command/_support.h"

using namespace std;
using namespace linda0::cli;

//====----====----====----====----====----====----====----====---- PRIVATE : FDEC

namespace
{
  TYPE_rc _rc_scanSemantic(const STRUCT_lineParsed& line);
  TYPE_rc _rc_scanChannel(const STRUCT_lineParsed& line);
}

//====----====----====----====----====----====----====----====---- PUBLIC : FDEF

namespace linda0::command::scan
{
  TYPE_rc rc_commandScan(const cli::STRUCT_lineParsed& line)
  {
    if (line.subCommand == "semantic")
    {
      return to_abs(_rc_scanSemantic(line));
    }
    if (line.subCommand == "channel")
    {
      return to_abs(_rc_scanChannel(line));
    }

    TYPE_rc rci = 0;
    rci--;
    vd_printErrorRc(rci, "Unknown scan subcommand");
    return rci;
  }
}

//====----====----====----====----====----====----====----====---- PRIVATE : FDEF

namespace
{
  TYPE_rc _rc_scanSemantic(const STRUCT_lineParsed& line)
  {
    TYPE_rc rci = 0;
    const filesystem::path pathFileSource = line.mapOperand.at("source");
    const string& nameLanguageSupport = line.mapOperand.at("support");

    string strSource;
    TYPE_rc rce = to_abs(rc_readFileText(pathFileSource, strSource));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed reading source file: " + pathFileSource.string());
      return rce;
    }

    linda0::languageSupport::INTF_languageSupport interfaceLanguage = {};
    rce = to_abs(linda0::command::_rc_loadLanguageSupport(nameLanguageSupport, interfaceLanguage));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed loading language support: " + nameLanguageSupport);
      return rce;
    }

    rci--;
    if (interfaceLanguage.rc_scanSemantic == nullptr)
    {
      const TYPE_rc rceFree = to_abs(linda0::languageSupport::rc_free(interfaceLanguage));
      if (rceFree != 0)
      {
        return rceFree;
      }
      vd_printErrorRc(rci, "Semantic scanning is not supported by: " + nameLanguageSupport);
      return rci;
    }

    TYPE_json jsonScanResult;
    const bool isVerbose = line.setOptionFlag.contains("--verbose");
    rce = to_abs(interfaceLanguage.rc_scanSemantic(strSource, isVerbose, jsonScanResult));
    const TYPE_rc rceFree = to_abs(linda0::languageSupport::rc_free(interfaceLanguage));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed scanning semantic source");
      return rce;
    }
    if (rceFree != 0)
    {
      return rceFree;
    }

    cout << jsonScanResult.dump(2) << endl;

    // END
    return 0;
  }

  TYPE_rc _rc_scanChannel(const STRUCT_lineParsed& line)
  {
    TYPE_rc rci = 0;
    const string& nameAudioSupport = line.mapOperand.at("support");
    const auto itDevice = line.mapOptionValue.find("--device");
    rci--;
    if (itDevice == line.mapOptionValue.end())
    {
      vd_printErrorRc(rci, "Audio device name is missing");
      return rci;
    }

    linda0::audioSupport::INTF_audioSupport interfaceAudio = {};
    TYPE_rc rce = to_abs(linda0::command::_rc_loadAudioSupport(nameAudioSupport, interfaceAudio));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed loading audio support: " + nameAudioSupport);
      return rce;
    }

    rci--;
    if (interfaceAudio.rc_scanChannel == nullptr)
    {
      const TYPE_rc rceFree = to_abs(linda0::audioSupport::rc_free(interfaceAudio));
      if (rceFree != 0)
      {
        return rceFree;
      }
      vd_printErrorRc(rci, "Channel scanning is not supported by: " + nameAudioSupport);
      return rci;
    }

    TYPE_json jsonScanResult;
    const bool isVerbose = line.setOptionFlag.contains("--verbose");
    rce = to_abs(interfaceAudio.rc_scanChannel(itDevice->second, isVerbose, jsonScanResult));
    const TYPE_rc rceFree = to_abs(linda0::audioSupport::rc_free(interfaceAudio));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed scanning audio channel");
      return rce;
    }
    if (rceFree != 0)
    {
      return rceFree;
    }

    cout << jsonScanResult.dump(2) << endl;

    // END
    return 0;
  }
}
