#pragma once

#include <cstdint>
#include <string>

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/json.hpp"

#include "02_shared/lsm/00_common/__dllMacro.h"
#include "02_shared/lsm/00_common/__lsmInfo.h"
#include "02_shared/lsm/02_language/__lsmLanguage.h"

//====----====----====----====----====----====----====----====---- FDEC

namespace lsm::language
{
  extern "C" LINDA_SUPPORT_EXPORT void vd_getInfo(STRUCT_lsmInfo& out_info);

  extern "C" LINDA_SUPPORT_EXPORT TYPE_rc rc_scanSemantic(const std::string& strSource, bool isVerbose, TYPE_json& out_jsonScanResult);
  extern "C" LINDA_SUPPORT_EXPORT TYPE_rc rc_buildRig(const REQUEST_rc_buildRig& request);

  extern "C" LINDA_SUPPORT_EXPORT TYPE_rc rc_openLanguageRuntime(const TYPE_json& jsonRunner, void*& out_handle_languageRuntime);
  extern "C" LINDA_SUPPORT_EXPORT TYPE_rc rc_bindLanguageRuntime(const REQUEST_rc_bindLanguageRuntime& request, void* handle_languageRuntime);
  extern "C" LINDA_SUPPORT_EXPORT TYPE_rc rc_processBuffer(void* handle_languageRuntime);
  extern "C" LINDA_SUPPORT_EXPORT void vd_closeLanguageRuntime(void*& handle_languageRuntime);
}
