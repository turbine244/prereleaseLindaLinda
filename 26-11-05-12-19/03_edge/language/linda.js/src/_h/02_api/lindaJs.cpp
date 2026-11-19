#include "03_edge/language/linda.js/src/_h/02_api/_lindaJs.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <system_error>
#include <string>

#include "01_standard/phase01/ioFile.hpp"
#include "03_edge/language/linda.js/src/_h/00_file/_lindaJsFile.h"
#include "03_edge/language/linda.js/src/_h/02_api/narrative/_narr.scanSemantic.h"
#include "03_edge/language/linda.js/src/_h/02_api/narrative/_narr.buildRig.h"
#include "03_edge/language/linda.js/src/_h/02_api/narrative/_narr.openLanguageRuntime.h"

using namespace std;

using namespace lsm::language;
using namespace lindaJs::runtime;
using namespace lindaJs::file;

//====----====----====----====----====----====----====----====---- VAL

namespace lindaJs::api
{
  namespace /* private */
  {
    HANDLE_lindaJsRuntime* _handle_activeLanguageRuntime = nullptr;

    TYPE_rc _rc_prepareBuildRigInputs(const REQUEST_rc_buildRig& request, string& out_strSource, TYPE_json& out_jsonSemanticVerbose);
    TYPE_rc _rc_prepareContextOpenLanguageRuntime(const TYPE_json& jsonRunner, lindaJs::narr::openLanguageRuntime::STRUCT_context& context);
    TYPE_rc _rc_validateBindLanguageRuntime(const REQUEST_rc_bindLanguageRuntime& request, void* handle_languageRuntime);
  }
}

//====----====----====----====----====----====----====----====---- FDEF

namespace lindaJs::api
{
  TYPE_rc rc_scanSemantic(const string& strSource, bool isVerbose, TYPE_json& out_jsonScanResult)
  {
    // NULLIFY : rc, out
    out_jsonScanResult = nullptr;
    TYPE_rc rce = 0;

    // NARRATIVE [0]
    narr::scanSemantic::STRUCT_context narrativeContext =
    {
      .input =
      {
        .strSource = strSource,
        .isVerbose = isVerbose
      }
    };

    // NARRATIVE [1]
    if (true)
    {
      rce = to_abs(narr::scanSemantic::narr_rc_getJsonMatchInfo(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed getting semantic match-info");
        return rce;
      }
    }

    // NARRATIVE [2]
    if (true)
    {
      narr::scanSemantic::narr_vd_getJsonSemanticInfo(narrativeContext);
    }

    // END
    out_jsonScanResult = narrativeContext.state.jsonSemanticInfo;
    return 0;
  }

  TYPE_rc rc_buildRig(const REQUEST_rc_buildRig& request)
  {
    // NULLIFY : rc, out
    TYPE_rc rce = 0;

    string strSource;
    TYPE_json jsonSemanticVerbose;
    // NARRATIVE [-1]
    if (true)
    {
      rce = to_abs(_rc_prepareBuildRigInputs(request, strSource, jsonSemanticVerbose));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed preparing RIG build inputs.");
        return rce;
      }
    }

    // NARRATIVE [0]
    narr::buildRig::STRUCT_context narrativeContext =
    {
      .input =
      {
        .requestBuildRig = request,
        .strSource = strSource,
        .jsonSemanticVerbose = jsonSemanticVerbose
      }
    };

    // NARRATIVE [1]
    if (true)
    {
      rce = to_abs(narr::buildRig::narr_rc_generateFeatures(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed generating support features.");
        return rce;
      }
    }
  
    // NARRATIVE [2]
    if (true)
    {
      rce = to_abs(narr::buildRig::narr_rc_validateSyntax(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Created payload has invalid syntax.");
        return rce;
      }
    }

    // NARRATIVE [3]
    if (true)
    {
      rce = to_abs(narr::buildRig::narr_rc_writeRigPackage(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed writing RIG package.");
        return rce;
      }
    }

    // END
    return 0;
  }

  TYPE_rc rc_openLanguageRuntime(const TYPE_json& jsonRunner, void*& out_handle_languageRuntime)
  {
    // NULLIFY : rc, out
    out_handle_languageRuntime = nullptr;
    TYPE_rc rce = 0;

    // NARRATIVE [-1]
    narr::openLanguageRuntime::STRUCT_context narrativeContext;
    if (true)
    {
      rce = to_abs(_rc_prepareContextOpenLanguageRuntime(jsonRunner, narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed preparing JavaScript runtime");
        return rce;
      }
    }

    // NARRATIVE [1]
    if (true)
    {
      rce = to_abs(narr::openLanguageRuntime::narr_rc_createRuntime(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed creating JavaScript runtime");
        return rce;
      }
    }

    // NARRATIVE [2]
    if (true)
    {
      rce = to_abs(narr::openLanguageRuntime::narr_rc_loadPayload(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed loading JavaScript payload");
        return rce;
      }
    }

    // NARRATIVE [3]
    if (true)
    {
      rce = to_abs(narr::openLanguageRuntime::narr_rc_evaluatePayload(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed evaluating JavaScript payload");
        return rce;
      }
    }

    // END
    out_handle_languageRuntime = narrativeContext.state.handle_languageRuntime;
    _handle_activeLanguageRuntime = narrativeContext.state.handle_languageRuntime;
    return 0;
  }

  TYPE_rc rc_bindLanguageRuntime(const REQUEST_rc_bindLanguageRuntime& request, void* handle_languageRuntime)
  {
    // NULLIFY : rc
    TYPE_rc rce = 0;
    TYPE_rc rci = 0;

    rce = to_abs(_rc_validateBindLanguageRuntime(request, handle_languageRuntime));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed validating JavaScript runtime binding");
      return rce;
    }

    HANDLE_lindaJsRuntime* handle_javascriptRuntime = static_cast<HANDLE_lindaJsRuntime*>(handle_languageRuntime);
    const size_t countInSize = static_cast<size_t>(request.numSemanticIn) * request.spb;
    const size_t countOutSize = static_cast<size_t>(request.numSemanticOut) * request.spb;
    JSValue jsArrayBufferIn = JS_NewArrayBuffer(handle_javascriptRuntime->jsContext, reinterpret_cast<uint8_t*>(request.ptrSemanticIn), sizeof(float) * countInSize, 0, nullptr, nullptr, false);
    // RCI : create JavaScript input buffer
    rci--;
    if (JS_IsException(jsArrayBufferIn))
    {
      vd_printJsExceptionInfo(json_exceptionInfo(handle_javascriptRuntime->jsContext));
      vd_printErrorRc(rci, "Failed creating JavaScript input buffer");
      return rci;
    }

    JSValue listArgumentIn[3] = { jsArrayBufferIn, JS_NewUint32(handle_javascriptRuntime->jsContext, 0), JS_NewUint32(handle_javascriptRuntime->jsContext, static_cast<uint32_t>(countInSize)) };
    JSValue jsBufferIn = JS_NewTypedArray(handle_javascriptRuntime->jsContext, 3, listArgumentIn, JS_TYPED_ARRAY_FLOAT32);
    JS_FreeValue(handle_javascriptRuntime->jsContext, listArgumentIn[2]);
    JS_FreeValue(handle_javascriptRuntime->jsContext, listArgumentIn[1]);
    JS_FreeValue(handle_javascriptRuntime->jsContext, jsArrayBufferIn);
    // RCI : link JavaScript input buffer
    rci--;
    if (JS_IsException(jsBufferIn))
    {
      vd_printJsExceptionInfo(json_exceptionInfo(handle_javascriptRuntime->jsContext));
      vd_printErrorRc(rci, "Failed linking JavaScript input buffer");
      return rci;
    }

    JSValue jsArrayBufferOut = JS_NewArrayBuffer(handle_javascriptRuntime->jsContext, reinterpret_cast<uint8_t*>(request.ptrSemanticOut), sizeof(float) * countOutSize, 0, nullptr, nullptr, false);
    // RCI : create JavaScript output buffer
    rci--;
    if (JS_IsException(jsArrayBufferOut))
    {
      vd_printJsExceptionInfo(json_exceptionInfo(handle_javascriptRuntime->jsContext));
      JS_FreeValue(handle_javascriptRuntime->jsContext, jsBufferIn);
      vd_printErrorRc(rci, "Failed creating JavaScript output buffer");
      return rci;
    }

    JSValue listArgumentOut[3] = { jsArrayBufferOut, JS_NewUint32(handle_javascriptRuntime->jsContext, 0), JS_NewUint32(handle_javascriptRuntime->jsContext, static_cast<uint32_t>(countOutSize)) };
    JSValue jsBufferOut = JS_NewTypedArray(handle_javascriptRuntime->jsContext, 3, listArgumentOut, JS_TYPED_ARRAY_FLOAT32);
    JS_FreeValue(handle_javascriptRuntime->jsContext, listArgumentOut[2]);
    JS_FreeValue(handle_javascriptRuntime->jsContext, listArgumentOut[1]);
    JS_FreeValue(handle_javascriptRuntime->jsContext, jsArrayBufferOut);
    // RCI : link JavaScript output buffer
    rci--;
    if (JS_IsException(jsBufferOut))
    {
      vd_printJsExceptionInfo(json_exceptionInfo(handle_javascriptRuntime->jsContext));
      JS_FreeValue(handle_javascriptRuntime->jsContext, jsBufferIn);
      vd_printErrorRc(rci, "Failed linking JavaScript output buffer");
      return rci;
    }

    // Commit the binding after creating both JavaScript buffers
    JS_FreeValue(handle_javascriptRuntime->jsContext, handle_javascriptRuntime->jsVarBufferOut);
    JS_FreeValue(handle_javascriptRuntime->jsContext, handle_javascriptRuntime->jsVarBufferIn);
    handle_javascriptRuntime->jsVarBufferIn = jsBufferIn;
    handle_javascriptRuntime->jsVarBufferOut = jsBufferOut;
    handle_javascriptRuntime->binding = request;
    // END
    return 0;
  }

  TYPE_rc rc_processBuffer(void* handle_languageRuntime)
  {
    // NULLIFY : rc
    TYPE_rc rci = 0;

    // RCI : validate runtime handle
    rci--;
    if (handle_languageRuntime == nullptr || handle_languageRuntime != _handle_activeLanguageRuntime)
    {
      vd_printErrorRc(rci, "JavaScript runtime handle is null or inactive");
      return rci;
    }

    HANDLE_lindaJsRuntime* handle_javascriptRuntime = static_cast<HANDLE_lindaJsRuntime*>(handle_languageRuntime);
    // RCI : validate Linda buffers
    rci--;
    if (JS_IsUndefined(handle_javascriptRuntime->jsVarBufferIn) || JS_IsUndefined(handle_javascriptRuntime->jsVarBufferOut))
    {
      vd_printErrorRc(rci, "Linda buffers are not bound");
      return rci;
    }

    // RCI : validate SPS pointer
    rci--;
    if (handle_javascriptRuntime->binding.ptrSps == nullptr)
    {
      vd_printErrorRc(rci, "SPS pointer is null");
      return rci;
    }

    JSValue listArgument[4] = { JS_NewUint32(handle_javascriptRuntime->jsContext, handle_javascriptRuntime->binding.spb), JS_NewUint32(handle_javascriptRuntime->jsContext, *handle_javascriptRuntime->binding.ptrSps), handle_javascriptRuntime->jsVarBufferIn, handle_javascriptRuntime->jsVarBufferOut };
    JSValue jsResultProcess = JS_Call(handle_javascriptRuntime->jsContext, handle_javascriptRuntime->jsFunctionProcess, JS_UNDEFINED, 4, listArgument);
    JS_FreeValue(handle_javascriptRuntime->jsContext, listArgument[1]);
    JS_FreeValue(handle_javascriptRuntime->jsContext, listArgument[0]);

    // RCI : call JavaScript process function
    rci--;
    if (JS_IsException(jsResultProcess))
    {
      TYPE_json jsonExceptionInfo = json_exceptionInfo(handle_javascriptRuntime->jsContext);
      TYPE_json jsonExceptionInfoResolved = jsonExceptionInfo;
      if (handle_javascriptRuntime->ptrDebugInfo != nullptr)
      {
        vd_resolveExceptionInfo(jsonExceptionInfo, *handle_javascriptRuntime->ptrDebugInfo, jsonExceptionInfoResolved);
      }

      handle_javascriptRuntime->runtimeErrorInfo.jsonPendingError = json_runtimeError("process", rci, jsonExceptionInfoResolved, handle_javascriptRuntime->ptrDebugInfo != nullptr ? handle_javascriptRuntime->ptrDebugInfo->pathFileSourceMap : filesystem::path());
      handle_javascriptRuntime->runtimeErrorInfo.jsonPendingError["payloadError"] = jsonExceptionInfo;
      JS_FreeValue(handle_javascriptRuntime->jsContext, jsResultProcess);
      vd_printJsExceptionInfo(jsonExceptionInfo);
      vd_printErrorRc(rci, "JavaScript process function failed");
      return rci;
    }

    JS_FreeValue(handle_javascriptRuntime->jsContext, jsResultProcess);
    // END
    return 0;
  }

  void vd_closeLanguageRuntime(void*& handle_languageRuntime)
  {
    if (handle_languageRuntime == nullptr)
    {
      return;
    }

    HANDLE_lindaJsRuntime* handle_javascriptRuntime = static_cast<HANDLE_lindaJsRuntime*>(handle_languageRuntime);
    if (!handle_javascriptRuntime->runtimeErrorInfo.jsonPendingError.empty())
    {
      // RCE : write pending JavaScript runtime error
      const TYPE_rc rce = to_abs(rc_writeFileJson(handle_javascriptRuntime->runtimeErrorInfo.pathFileError, handle_javascriptRuntime->runtimeErrorInfo.jsonPendingError));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed writing JavaScript runtime error");
      }
    }

    if (handle_languageRuntime == _handle_activeLanguageRuntime)
    {
      _handle_activeLanguageRuntime = nullptr;
    }

    vd_releaseJavascriptRuntime(handle_javascriptRuntime);
    handle_languageRuntime = nullptr;

    // END
    return;
  }
}

//====----====----====----====----====----====----====----====---- FDEF

namespace lindaJs::api
{
  namespace /* private */
  {
    TYPE_rc _rc_prepareBuildRigInputs(const REQUEST_rc_buildRig& request, string& out_strSource, TYPE_json& out_jsonSemanticVerbose)
    {
      // NULLIFY : rc, out
      out_strSource.clear();
      out_jsonSemanticVerbose = nullptr;
      TYPE_rc rce = 0;
      TYPE_rc rci = 0;

      // RCE : read the source file
      rce = to_abs(rc_readFileText(request.pathFileSource, out_strSource));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed reading the source file.");
        return rce;
      }

      // RCE : scan source semantics
      rce = to_abs(rc_scanSemantic(out_strSource, true, out_jsonSemanticVerbose));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed scanning source semantics.");
        return rce;
      }

      // RCI : reject semantic errors
      rci--;
      if (true)
      {
        const bool isSemanticError =
        {
          out_jsonSemanticVerbose.contains("problem")
          && out_jsonSemanticVerbose["problem"].contains("listError")
          && out_jsonSemanticVerbose["problem"]["listError"].is_array()
          && !out_jsonSemanticVerbose["problem"]["listError"].empty()
        };

        if (isSemanticError)
        {
          for (const TYPE_json& error : out_jsonSemanticVerbose["problem"]["listError"])
          {
            vd_printErrorRc(rci, error.value("detail", "Semantic validation failed."));
          }
          return rci;
        }
      }

      // RCI : validate fx syntax
      rci--;
      if (!out_jsonSemanticVerbose["fx"]["ok"].get<bool>())
      {
        vd_printErrorRc(rci, "The fx function contains syntax errors.");
        return rci;
      }

      // RCI : validate the fx body range
      rci--;
      if (out_jsonSemanticVerbose["fx"]["body"]["startByte"].is_null()
        || out_jsonSemanticVerbose["fx"]["body"]["endByte"].is_null())
      {
        vd_printErrorRc(rci, "The fx function body range is missing.");
        return rci;
      }

      // END
      return 0;
    }

    TYPE_rc _rc_prepareContextOpenLanguageRuntime(const TYPE_json& jsonRunner, lindaJs::narr::openLanguageRuntime::STRUCT_context& context)
    {
      // NULLIFY : rc, context
      context = {};
      TYPE_rc rci = 0;

      context.input.isRuntimeActive = (_handle_activeLanguageRuntime != nullptr);

      // RCI : reject another active runtime
      rci--;
      if (context.input.isRuntimeActive)
      {
        vd_printErrorRc(rci, "Language runtime is already active");
        return rci;
      }

      // RCI : validate runner paths
      rci--;
      if (true)
      {
        const bool isRunnerRigPathValid =
        {
          jsonRunner.is_object()
          && jsonRunner.contains("paths")
          && jsonRunner["paths"].is_object()
          && jsonRunner["paths"].contains("rig")
          && jsonRunner["paths"]["rig"].is_string()
        };
        if (!isRunnerRigPathValid)
        {
          vd_printErrorRc(rci, "Runner paths are missing or invalid");
          return rci;
        }
      }

      const filesystem::path pathDirRig = jsonRunner["paths"]["rig"].get<string>();
      context.input.pathFilePayloadCore = pathFile_payloadCore(pathDirRig);
      context.input.pathFileSourceMap = pathFile_sourceMap(pathDirRig);

      if (jsonRunner["paths"].contains("source") && jsonRunner["paths"]["source"].is_string())
      {
        context.input.pathFileSource = jsonRunner["paths"]["source"].get<string>();
      }

      if (true)
      {
        const bool isErrorPathConfigured =
        {
          jsonRunner.contains("rigPackage")
          && jsonRunner["rigPackage"].is_object()
          && jsonRunner["rigPackage"].contains("error")
          && jsonRunner["rigPackage"]["error"].is_string()
        };
        if (isErrorPathConfigured)
        {
          context.input.pathFileError = jsonRunner["rigPackage"]["error"].get<string>();
          if (context.input.pathFileError.is_relative())
          {
            if (context.input.pathFileError.parent_path().empty() && context.input.pathFileError.filename().generic_string() == const_nameFileError)
            {
              context.input.pathFileError = pathDir_debug(pathDirRig) / context.input.pathFileError;
            }
            else
            {
              context.input.pathFileError = pathDirRig / context.input.pathFileError;
            }
          }
        }
        else
        {
          context.input.pathFileError = pathFile_error(pathDirRig);
        }
      }

      if (true)
      {
        const bool isDebugOptionConfigured =
        {
          jsonRunner.contains("runOption")
          && jsonRunner["runOption"].is_object()
          && jsonRunner["runOption"].contains("debug")
          && jsonRunner["runOption"]["debug"].is_boolean()
        };
        if (isDebugOptionConfigured)
        {
          context.input.isDebugEnabled = jsonRunner["runOption"]["debug"].get<bool>();
        }
      }

      std::error_code ecRemoveError;
      filesystem::remove(context.input.pathFileError, ecRemoveError);

      // RCI : clear previous runtime error file
      rci--;
      if (ecRemoveError)
      {
        vd_printErrorRc(rci, "Failed clearing previous runtime error file: " + context.input.pathFileError.string() + " (" + ecRemoveError.message() + ")");
        return rci;
      }

      // END
      return 0;
    }
    
    TYPE_rc _rc_validateBindLanguageRuntime(const REQUEST_rc_bindLanguageRuntime& request, void* handle_languageRuntime)
    {
      // NULLIFY : rc
      TYPE_rc rci = 0;

      // RCI : validate runtime handle
      rci--;
      if (handle_languageRuntime == nullptr || handle_languageRuntime != _handle_activeLanguageRuntime)
      {
        vd_printErrorRc(rci, "JavaScript runtime handle is null or inactive");
        return rci;
      }

      // RCI : validate SPB
      rci--;
      if (request.spb == 0)
      {
        vd_printErrorRc(rci, "SPB must be greater than 0");
        return rci;
      }

      // RCI : validate Semantic-In buffer
      rci--;
      if (request.numSemanticIn > 0 && request.ptrSemanticIn == nullptr)
      {
        vd_printErrorRc(rci, "Semantic-In buffer is null");
        return rci;
      }

      // RCI : validate Semantic-Out buffer
      rci--;
      if (request.numSemanticOut > 0 && request.ptrSemanticOut == nullptr)
      {
        vd_printErrorRc(rci, "Semantic-Out buffer is null");
        return rci;
      }

      // RCI : validate SPS pointer
      rci--;
      if (request.ptrSps == nullptr)
      {
        vd_printErrorRc(rci, "SPS pointer is null");
        return rci;
      }

      const uint64_t countIn = static_cast<uint64_t>(request.numSemanticIn) * request.spb;
      const uint64_t countOut = static_cast<uint64_t>(request.numSemanticOut) * request.spb;
      // RCI : validate buffer element count
      rci--;
      if (countIn > (numeric_limits<uint32_t>::max)() || countOut > (numeric_limits<uint32_t>::max)())
      {
        vd_printErrorRc(rci, "Semantic buffer element count is too large");
        return rci;
      }

      // RCI : validate buffer byte size
      rci--;
      if (countIn > (numeric_limits<size_t>::max)() / sizeof(float) || countOut > (numeric_limits<size_t>::max)() / sizeof(float))
      {
        vd_printErrorRc(rci, "Semantic buffer byte size overflows");
        return rci;
      }

      // END
      return 0;
    }
  }
}
