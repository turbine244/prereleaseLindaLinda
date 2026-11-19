#pragma once

/*
  Standard RIG package structure.

    <package root>/
    ├─ .rig.linda0.json       : package manifest
    ├─ semantic.json          : semantic information(non-verbose)
    ├─ payload/
    │  └─ <payload files>     : support-specific payload files
    └─ debug/
       ├─ error.json          : error report
       └─ <debug files>       : additional support-specific debug data
*/

#include <cstdint>
#include <filesystem>
#include <string>
#include "03_edge/launcher/linda0/src/__env.h"
#include "01_standard/phase00/error.hpp"

//====----====----====----====----====----====----====----====---- PUBLIC : VAL

namespace linda0::rigPackage
{
  inline const std::string const_nameFileManifest = ".rig." ENV_WHOAMI ".json";
  inline const std::string const_typeManifest = ENV_WHOAMI "-rig-package";
  inline const uint32_t const_schemaVersionCurrent = 1;
  inline const std::string const_nameFileSemantic = "semantic.json";
  inline const std::string const_nameDirDebug = "debug";
  inline const std::string const_nameFileError = "error.json";
  inline const std::string const_pathRelativeFileError = (std::filesystem::path(const_nameDirDebug) / const_nameFileError).generic_string();
  inline const std::string const_nameDirPayload = "payload";
}

//====----====----====----====----====----====----====----====---- PUBLIC : FDEC

namespace linda0::rigPackage
{
  std::filesystem::path pathFile_manifest(const std::filesystem::path& pathDirPackage);
  std::filesystem::path pathFile_semantic(const std::filesystem::path& pathDirPackage);
  std::filesystem::path pathDir_debug(const std::filesystem::path& pathDirPackage);
  std::filesystem::path pathDir_payload(const std::filesystem::path& pathDirPackage);
  std::filesystem::path pathFile_error(const std::filesystem::path& pathDirPackage);
  TYPE_rc rc_writeManifest(const std::filesystem::path& pathDirPackage, const std::string& nameLanguageSupport);
}
