#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "03_edge/launcher/linda0/src/_h/01_runtime/00_audioSupport/_audioSupport.h"
#include "03_edge/launcher/linda0/src/_h/01_runtime/01_languageSupport/_languageSupport.h"

//====----====----====----====----====----====----====----====---- FDEC

namespace linda0::command
{
  TYPE_rc _rc_listAudioSupport(std::vector<audioSupport::STRUCT_audioSupportInfo>& out_listInfo);
  TYPE_rc _rc_listLanguageSupport(std::vector<languageSupport::STRUCT_languageSupportInfo>& out_listInfo);
  TYPE_rc _rc_loadAudioSupport(const std::string& nameSupport, audioSupport::INTF_audioSupport& out_interface);
  TYPE_rc _rc_loadLanguageSupport(const std::string& nameSupport, languageSupport::INTF_languageSupport& out_interface);
}
