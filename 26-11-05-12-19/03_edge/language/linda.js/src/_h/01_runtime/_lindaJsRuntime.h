#pragma once

/*
  linda.js JavaScript runtime state and diagnostics.
*/

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/json.hpp"
#include "02_shared/lsm/02_language/__lsmLanguage.h"
#include "00_vendored/quickjs-ng/quickjs.h"

//====----====----====----====----====----====----====----====---- VAL

namespace lindaJs::runtime
{
  inline const std::string const_nameRuntimeProcess = "__linda_process";
  inline const std::string const_nameRuntimeIndex = "__linda_i";
  inline const std::string const_nameRuntimeSps = "__linda_sps";
  inline const std::string const_nameRuntimeInput = "__linda_in";
  inline const std::string const_nameRuntimeOutput = "__linda_out";
}

//====----====----====----====----====----====----====----====---- TYPE

namespace lindaJs::runtime
{
  struct STRUCT_debugInfo
  {
    std::filesystem::path pathFileSource = "";
    std::filesystem::path pathFilePayloadCore = "";
    std::filesystem::path pathFileSourceMap = "";

    std::string strSource = "";
    std::string strPayload = "";

    TYPE_json jsonSourceMap = {};
  };

  struct HANDLE_lindaJsRuntime
  {
    struct STRUCT_runtimeErrorInfo
    {
      std::filesystem::path pathFileError = "";
      TYPE_json jsonPendingError = {};
    };

    JSRuntime* jsRuntime = nullptr;
    JSContext* jsContext = nullptr;

    JSValue jsFunctionProcess = JS_UNDEFINED;
    JSValue jsVarBufferIn = JS_UNDEFINED;
    JSValue jsVarBufferOut = JS_UNDEFINED;

    lsm::language::REQUEST_rc_bindLanguageRuntime binding = {};

    STRUCT_runtimeErrorInfo runtimeErrorInfo = {};
    std::unique_ptr<STRUCT_debugInfo> ptrDebugInfo = nullptr;
  };
}

//====----====----====----====----====----====----====----====---- FDEC

namespace lindaJs::runtime
{
  TYPE_json json_exceptionInfo(JSContext* jsContext);
  void vd_printJsExceptionInfo(const TYPE_json& jsonExceptionInfo);
  void vd_resolveExceptionInfo(const TYPE_json& jsonExceptionInfo, const STRUCT_debugInfo& debugInfo, TYPE_json& out_jsonExceptionInfoResolved);
  TYPE_json json_runtimeError(const std::string& stage, TYPE_rc rc, const TYPE_json& jsonExceptionInfo, const std::filesystem::path& pathFileSourceMap);
  void vd_releaseJavascriptRuntime(HANDLE_lindaJsRuntime* handle_lindaJsRuntime);
}
