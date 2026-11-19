#pragma once

/*
  linda.js files in a standard RIG package.

    <package root>/
    ├─ .rig.linda.js.json : package manifest
    ├─ semantic.json      : semantic information
    ├─ payload/
    │  └─ core.js         : generated JavaScript
    └─ debug/
       ├─ error.json      : runtime error report
       └─ sourceMap.json  : A-to-B text mapping
*/

#include <cstdint>
#include <filesystem>
#include <string>

//====----====----====----====----====----====----====----====---- VAL

namespace lindaJs::file
{
  inline const std::string const_typeRigPackage = "linda0-rig-package";
  inline const uint32_t const_schemaVersionRigPackage = 1;
  inline const std::string const_nameLanguageSupport = "linda.js";
  inline const std::string const_nameFileManifest = ".rig.linda.js.json";
  inline const std::string const_nameFileSemantic = "semantic.json";
  inline const std::string const_nameDirPayload = "payload";
  inline const std::string const_nameFilePayloadCore = "core.js";
  inline const std::string const_nameDirDebug = "debug";
  inline const std::string const_nameFileError = "error.json";
  inline const std::string const_nameFileSourceMap = "sourceMap.json";
  inline const std::string const_pathRelativeFileError = "debug/error.json";
}

//====----====----====----====----====----====----====----====---- FDEC

namespace lindaJs::file
{
  std::filesystem::path pathDir_payload(const std::filesystem::path& pathDirPackage);
  std::filesystem::path pathDir_debug(const std::filesystem::path& pathDirPackage);
  std::filesystem::path pathFile_manifest(const std::filesystem::path& pathDirPackage);
  std::filesystem::path pathFile_semantic(const std::filesystem::path& pathDirPackage);
  std::filesystem::path pathFile_error(const std::filesystem::path& pathDirPackage);
  std::filesystem::path pathFile_payloadCore(const std::filesystem::path& pathDirPackage);
  std::filesystem::path pathFile_sourceMap(const std::filesystem::path& pathDirPackage);
}
