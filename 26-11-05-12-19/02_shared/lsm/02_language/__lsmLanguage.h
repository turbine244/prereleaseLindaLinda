#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/json.hpp"

//====----====----====----====----====----====----====----====---- TYPE

namespace lsm::language
{
  struct REQUEST_rc_buildRig
  {
    const std::filesystem::path& pathFileSource;
    const std::filesystem::path& pathDirPackage;
    bool absolute = false;
  };

  using CALLBACK_rc_scanSemantic =
    TYPE_rc (*)(const std::string& strSource, bool isVerbose, TYPE_json& out_jsonScanResult);

  using CALLBACK_rc_buildRig =
    TYPE_rc (*)(const REQUEST_rc_buildRig& request);

  struct REQUEST_rc_bindLanguageRuntime
  {
    uint32_t spb = 0; // Fixed until runtime close.
    uint32_t numSemanticIn = 0;
    uint32_t numSemanticOut = 0;

    float* ptrSemanticIn = nullptr; // Borrowed until runtime close.
    float* ptrSemanticOut = nullptr; // Borrowed until runtime close.
    uint32_t* ptrSps = nullptr; // Borrowed until runtime close.
  };

  using CALLBACK_rc_openLanguageRuntime =
    TYPE_rc (*)(const TYPE_json& jsonRunner, void*& out_handle_languageRuntime);

  using CALLBACK_rc_bindLanguageRuntime =
    TYPE_rc (*)(const REQUEST_rc_bindLanguageRuntime& request, void* handle_languageRuntime);

  using CALLBACK_rc_processBuffer =
    TYPE_rc (*)(void* handle_languageRuntime);

  using CALLBACK_vd_closeLanguageRuntime =
    void (*)(void*& handle_languageRuntime);
}
