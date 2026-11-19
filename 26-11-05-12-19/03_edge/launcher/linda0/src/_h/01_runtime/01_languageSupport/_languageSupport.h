#pragma once

/*
  Language support DLL discovery, loading & release.
*/

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "01_standard/phase00/error.hpp"
#include "02_shared/lsm/00_common/__lsmInfo.h"
#include "02_shared/lsm/02_language/__lsmLanguage.h"

//====----====----====----====----====----====----====----====---- TYPE

namespace linda0::languageSupport
{
  struct STRUCT_languageSupportInfo
  {
    std::filesystem::path pathFileDll = "";
    std::string name = "";
    uint32_t versionMajor = 0;
    uint32_t versionMinor = 0;
    uint32_t versionPatch = 0;
    std::string description = "";
  };

  struct INTF_languageSupport
  {
    void* handle = nullptr;
    STRUCT_languageSupportInfo info = {};

    lsm::language::CALLBACK_rc_scanSemantic rc_scanSemantic = nullptr;
    lsm::language::CALLBACK_rc_buildRig rc_buildRig = nullptr;
    lsm::language::CALLBACK_rc_openLanguageRuntime rc_openLanguageRuntime = nullptr;
    lsm::language::CALLBACK_rc_bindLanguageRuntime rc_bindLanguageRuntime = nullptr;
    lsm::language::CALLBACK_rc_processBuffer rc_processBuffer = nullptr;
    lsm::language::CALLBACK_vd_closeLanguageRuntime vd_closeLanguageRuntime = nullptr;
  };
}

//====----====----====----====----====----====----====----====---- FDEC

namespace linda0::languageSupport
{
  TYPE_rc rc_list(const std::filesystem::path& pathDirLsm, std::vector<STRUCT_languageSupportInfo>& out_listInfo);
  TYPE_rc rc_load(const std::filesystem::path& pathDirLsm, const std::string& nameFileDll, INTF_languageSupport& out_interface);
  TYPE_rc rc_free(INTF_languageSupport& edit_interface);
}

