#include "03_edge/language/linda.js/src/_h/02_api/narrative/_narr.openLanguageRuntime.h"

#include <new>

#include "01_standard/phase01/ioFile.hpp"
#include "03_edge/language/linda.js/src/_h/00_file/_lindaJsFile.h"

using namespace std;
using namespace lindaJs::narr::openLanguageRuntime;
using namespace lindaJs::runtime;

//====----====----====----====----====----====----====----====---- FDEC

namespace lindaJs::narr::openLanguageRuntime
{
  namespace /* private */
  {
    // [3-A]
    inline TYPE_rc _narr_rc_preparePayloadEvaluation(STRUCT_context& context);

    // [3-B]
    inline TYPE_rc _narr_rc_evaluatePayload(STRUCT_context& context);

    // [3-C]
    inline TYPE_rc _narr_rc_resolveProcessFunction(STRUCT_context& context);

    // [3-D]
    inline TYPE_rc _narr_rc_validateProcessFunction(STRUCT_context& context);

    // non-narrative
    void _narr_vd_releaseRuntime(STRUCT_context& edit_context);

    // non-narrative
    TYPE_rc _narr_rc_writeRuntimeError(const STRUCT_context& context, const TYPE_json& jsonError);
  }
}

//====----====----====----====----====----====----====----====---- FDEF

namespace lindaJs::narr::openLanguageRuntime
{
  // [1]
  TYPE_rc narr_rc_createRuntime(STRUCT_context& edit_context)
  {
    // NULLIFY : rc, context.state.handle_languageRuntime
    edit_context.state.handle_languageRuntime = nullptr;
    TYPE_rc rci = 0;

    // RCI : reject another active runtime
    rci--;
    if (edit_context.input.isRuntimeActive)
    {
      vd_printErrorRc(rci, "Language runtime is already active");
      return rci;
    }

    HANDLE_lindaJsRuntime* temp_handle_languageRuntime = new (nothrow) HANDLE_lindaJsRuntime();
    // RCI : allocate runtime state
    rci--;
    if (temp_handle_languageRuntime == nullptr)
    {
      vd_printErrorRc(rci, "Failed allocating JavaScript runtime handle");
      return rci;
    }

    temp_handle_languageRuntime->runtimeErrorInfo.pathFileError = edit_context.input.pathFileError;
    // RCI : allocate debug information
    rci--;
    if (edit_context.input.isDebugEnabled)
    {
      temp_handle_languageRuntime->ptrDebugInfo.reset(new (nothrow) STRUCT_debugInfo());
      if (temp_handle_languageRuntime->ptrDebugInfo == nullptr)
      {
        vd_printErrorRc(rci, "Failed allocating JavaScript debug information");
        vd_releaseJavascriptRuntime(temp_handle_languageRuntime);
        return rci;
      }

      temp_handle_languageRuntime->ptrDebugInfo->pathFileSource = edit_context.input.pathFileSource;
      temp_handle_languageRuntime->ptrDebugInfo->pathFilePayloadCore = edit_context.input.pathFilePayloadCore;
      temp_handle_languageRuntime->ptrDebugInfo->pathFileSourceMap = edit_context.input.pathFileSourceMap;
    }

    // RCI : create JavaScript runtime
    temp_handle_languageRuntime->jsRuntime = JS_NewRuntime();
    rci--;
    if (temp_handle_languageRuntime->jsRuntime == nullptr)
    {
      vd_printErrorRc(rci, "Failed creating JavaScript runtime");
      vd_releaseJavascriptRuntime(temp_handle_languageRuntime);
      return rci;
    }

    // RCI : create JavaScript context
    temp_handle_languageRuntime->jsContext = JS_NewContext(temp_handle_languageRuntime->jsRuntime);
    rci--;
    if (temp_handle_languageRuntime->jsContext == nullptr)
    {
      vd_printErrorRc(rci, "Failed creating JavaScript context");
      vd_releaseJavascriptRuntime(temp_handle_languageRuntime);
      return rci;
    }

    edit_context.state.handle_languageRuntime = temp_handle_languageRuntime;
    // END
    return 0;
  }

  // [2]
  TYPE_rc narr_rc_loadPayload(STRUCT_context& edit_context)
  {
    // NULLIFY : rc
    edit_context.state.strPayload.clear();
    TYPE_rc rce = 0;
    TYPE_rc rci = 0;

    // RCI : validate runtime handle
    rci--;
    if (edit_context.state.handle_languageRuntime == nullptr)
    {
      vd_printErrorRc(rci, "JavaScript runtime handle is null");
      return rci;
    }

    HANDLE_lindaJsRuntime* handle_javascriptRuntime = edit_context.state.handle_languageRuntime;
    string strPayload;

    // RCE : read JavaScript payload
    rce = to_abs(rc_readFileText(edit_context.input.pathFilePayloadCore, strPayload));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed reading JavaScript payload");
      vd_releaseJavascriptRuntime(handle_javascriptRuntime);
      edit_context.state.handle_languageRuntime = nullptr;
      return rce;
    }

    if (handle_javascriptRuntime->ptrDebugInfo != nullptr)
    {
      STRUCT_debugInfo& debugInfo = *handle_javascriptRuntime->ptrDebugInfo;
      if (!edit_context.input.pathFileSource.empty())
      {
        // RCE : read source for debug information
        rce = to_abs(rc_readFileText(edit_context.input.pathFileSource, debugInfo.strSource));
        if (rce != 0)
        {
          vd_printErrorRc(rce, "Failed reading source for JavaScript debug information");
          vd_releaseJavascriptRuntime(handle_javascriptRuntime);
          edit_context.state.handle_languageRuntime = nullptr;
          return rce;
        }
      }

      // RCE : read source-map for debug information
      rce = to_abs(rc_readFileJson(edit_context.input.pathFileSourceMap, debugInfo.jsonSourceMap));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed reading source-map for JavaScript debug information");
        vd_releaseJavascriptRuntime(handle_javascriptRuntime);
        edit_context.state.handle_languageRuntime = nullptr;
        return rce;
      }
    }

    edit_context.state.strPayload = strPayload;
    // END
    return 0;
  }

  // [3]
  TYPE_rc narr_rc_evaluatePayload(STRUCT_context& edit_context)
  {
    // NULLIFY : rc
    TYPE_rc rce = 0;

    // [3-A]
    if (true)
    {
      rce = to_abs(_narr_rc_preparePayloadEvaluation(edit_context));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed preparing JavaScript payload evaluation");
        return rce;
      }
    }

    // [3-B]
    if (true)
    {
      rce = to_abs(_narr_rc_evaluatePayload(edit_context));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed evaluating JavaScript payload");
        return rce;
      }
    }

    // [3-C]
    if (true)
    {
      rce = to_abs(_narr_rc_resolveProcessFunction(edit_context));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed resolving JavaScript process function");
        return rce;
      }
    }

    // [3-D]
    if (true)
    {
      rce = to_abs(_narr_rc_validateProcessFunction(edit_context));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed validating JavaScript process function");
        return rce;
      }
    }

    // END
    return 0;
  }
}

//====----====----====----====----====----====----====----====---- FDEF

namespace lindaJs::narr::openLanguageRuntime
{
  namespace /* private */
  {
    // [3-A]
    inline TYPE_rc _narr_rc_preparePayloadEvaluation(STRUCT_context& context)
    {
      // NULLIFY : rc
      TYPE_rc rci = 0;

      // RCI : validate runtime handle
      rci--;
      if (context.state.handle_languageRuntime == nullptr)
      {
        vd_printErrorRc(rci, "JavaScript runtime handle is null");
        return rci;
      }

      HANDLE_lindaJsRuntime* handle_javascriptRuntime = context.state.handle_languageRuntime;
      if (handle_javascriptRuntime->ptrDebugInfo != nullptr)
      {
        handle_javascriptRuntime->ptrDebugInfo->strPayload = context.state.strPayload;
      }

      // END
      return 0;
    }

    // [3-B]
    inline TYPE_rc _narr_rc_evaluatePayload(STRUCT_context& context)
    {
      // NULLIFY : rc
      TYPE_rc rci = 0;
      HANDLE_lindaJsRuntime* handle_javascriptRuntime = context.state.handle_languageRuntime;
      JSValue jsResultEval = JS_Eval(handle_javascriptRuntime->jsContext, context.state.strPayload.data(), context.state.strPayload.size(), lindaJs::file::const_nameFilePayloadCore.c_str(), JS_EVAL_TYPE_GLOBAL);

      // RCI : evaluate JavaScript payload
      rci--;
      if (JS_IsException(jsResultEval))
      {
        TYPE_json jsonExceptionInfo = json_exceptionInfo(handle_javascriptRuntime->jsContext);
        TYPE_json jsonExceptionInfoResolved = jsonExceptionInfo;
        if (handle_javascriptRuntime->ptrDebugInfo != nullptr)
        {
          vd_resolveExceptionInfo(jsonExceptionInfo, *handle_javascriptRuntime->ptrDebugInfo, jsonExceptionInfoResolved);
        }

        TYPE_json jsonError = json_runtimeError("load", rci, jsonExceptionInfoResolved, context.input.pathFileSourceMap);
        jsonError["payloadError"] = jsonExceptionInfo;
        const TYPE_rc rce = _narr_rc_writeRuntimeError(context, jsonError);
        JS_FreeValue(handle_javascriptRuntime->jsContext, jsResultEval);
        vd_printJsExceptionInfo(jsonExceptionInfo);
        _narr_vd_releaseRuntime(context);
        if (rce != 0)
        {
          return rce;
        }

        vd_printErrorRc(rci, "JavaScript payload evaluation failed");
        return rci;
      }

      JS_FreeValue(handle_javascriptRuntime->jsContext, jsResultEval);
      // END
      return 0;
    }

    // [3-C]
    inline TYPE_rc _narr_rc_resolveProcessFunction(STRUCT_context& context)
    {
      // NULLIFY : rc
      TYPE_rc rci = 0;
      HANDLE_lindaJsRuntime* handle_javascriptRuntime = context.state.handle_languageRuntime;

      // RCI : resolve process function
      JSValue jsGlobal = JS_GetGlobalObject(handle_javascriptRuntime->jsContext);
      handle_javascriptRuntime->jsFunctionProcess = JS_GetPropertyStr(handle_javascriptRuntime->jsContext, jsGlobal, const_nameRuntimeProcess.c_str());
      JS_FreeValue(handle_javascriptRuntime->jsContext, jsGlobal);
      rci--;
      if (JS_IsException(handle_javascriptRuntime->jsFunctionProcess))
      {
        TYPE_json jsonExceptionInfo = json_exceptionInfo(handle_javascriptRuntime->jsContext);
        TYPE_json jsonExceptionInfoResolved = jsonExceptionInfo;
        if (handle_javascriptRuntime->ptrDebugInfo != nullptr)
        {
          vd_resolveExceptionInfo(jsonExceptionInfo, *handle_javascriptRuntime->ptrDebugInfo, jsonExceptionInfoResolved);
        }

        TYPE_json jsonError = json_runtimeError("load", rci, jsonExceptionInfoResolved, context.input.pathFileSourceMap);
        jsonError["payloadError"] = jsonExceptionInfo;
        const TYPE_rc rce = _narr_rc_writeRuntimeError(context, jsonError);
        vd_printJsExceptionInfo(jsonExceptionInfo);
        _narr_vd_releaseRuntime(context);
        if (rce != 0)
        {
          return rce;
        }

        vd_printErrorRc(rci, "Failed resolving JavaScript process function");
        return rci;
      }

      // END
      return 0;
    }

    // [3-D]
    inline TYPE_rc _narr_rc_validateProcessFunction(STRUCT_context& context)
    {
      // NULLIFY : rc
      TYPE_rc rci = 0;
      HANDLE_lindaJsRuntime* handle_javascriptRuntime = context.state.handle_languageRuntime;

      // RCI : validate process function
      rci--;
      if (!JS_IsFunction(handle_javascriptRuntime->jsContext, handle_javascriptRuntime->jsFunctionProcess))
      {
        TYPE_json jsonError = json_runtimeError("load", rci, TYPE_json::object(), context.input.pathFileSourceMap);
        jsonError["kind"] = "RuntimeError";
        jsonError["message"] = "JavaScript process function was not found";
        const TYPE_rc rce = _narr_rc_writeRuntimeError(context, jsonError);
        _narr_vd_releaseRuntime(context);
        if (rce != 0)
        {
          return rce;
        }

        vd_printErrorRc(rci, "JavaScript process function was not found");
        return rci;
      }

      // END
      return 0;
    }

    void _narr_vd_releaseRuntime(STRUCT_context& edit_context)
    {
      vd_releaseJavascriptRuntime(edit_context.state.handle_languageRuntime);
      edit_context.state.handle_languageRuntime = nullptr;

      // END
      return;
    }

    TYPE_rc _narr_rc_writeRuntimeError(const STRUCT_context& context, const TYPE_json& jsonError)
    {
      const TYPE_rc rce = to_abs(rc_writeFileJson(context.input.pathFileError, jsonError));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed writing JavaScript runtime error");
      }

      return rce;
    }
  }
}
