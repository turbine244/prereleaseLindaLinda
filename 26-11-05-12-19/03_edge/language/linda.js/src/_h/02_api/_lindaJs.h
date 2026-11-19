#pragma once

/*
  linda.js language support API.
*/

#include <string>

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/json.hpp"
#include "02_shared/lsm/02_language/__lsmLanguage.h"

//====----====----====----====----====----====----====----====---- FDEC

namespace lindaJs::api
{
  TYPE_rc rc_scanSemantic(const std::string& strSource, bool isVerbose, TYPE_json& out_jsonScanResult);
  TYPE_rc rc_buildRig(const lsm::language::REQUEST_rc_buildRig& request);

  TYPE_rc rc_openLanguageRuntime(const TYPE_json& jsonRunner, void*& out_handle_languageRuntime);
  TYPE_rc rc_bindLanguageRuntime(const lsm::language::REQUEST_rc_bindLanguageRuntime& request, void* handle_languageRuntime);
  TYPE_rc rc_processBuffer(void* handle_languageRuntime);
  void vd_closeLanguageRuntime(void*& handle_languageRuntime);
}
