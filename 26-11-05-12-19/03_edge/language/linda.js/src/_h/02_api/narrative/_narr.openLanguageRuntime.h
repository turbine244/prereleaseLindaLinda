#pragma once

/*
  Narrative functions : lindaJs.openLanguageRuntime

  [-1] Validate the runner and locate RIG package files
  [~] START
  [1] Create the JavaScript runtime state
  [2] Read the payload and optional debug information
  [3] Evaluate the payload and resolve its process function
  [~] END
*/

#include <filesystem>
#include <string>

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/json.hpp"
#include "03_edge/language/linda.js/src/_h/01_runtime/_lindaJsRuntime.h"

//====----====----====----====----====----====----====----====---- TYPE

namespace lindaJs::narr::openLanguageRuntime
{
  struct STRUCT_context
  {
    struct STRUCT_input
    {
      std::filesystem::path pathFilePayloadCore = "";
      std::filesystem::path pathFileSource = "";
      std::filesystem::path pathFileSourceMap = "";
      std::filesystem::path pathFileError = "";
      bool isDebugEnabled = false;
      bool isRuntimeActive = false;
    } input;

    struct STRUCT_state
    {
      std::string strPayload = "";
      runtime::HANDLE_lindaJsRuntime* handle_languageRuntime = nullptr;
    } state;
  };
}

//====----====----====----====----====----====----====----====---- FDEC

namespace lindaJs::narr::openLanguageRuntime
{
  TYPE_rc narr_rc_createRuntime(STRUCT_context& edit_context);
  TYPE_rc narr_rc_loadPayload(STRUCT_context& edit_context);
  TYPE_rc narr_rc_evaluatePayload(STRUCT_context& edit_context);
}
