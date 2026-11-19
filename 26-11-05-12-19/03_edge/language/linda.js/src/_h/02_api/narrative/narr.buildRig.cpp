#include "03_edge/language/linda.js/src/_h/02_api/narrative/_narr.buildRig.h"

#include <string>
#include <vector>

#include "01_standard/phase01/ioFile.hpp"
#include "02_shared/_utility/sourceMap.hpp"
#include "03_edge/language/linda.js/src/_h/00_file/_lindaJsFile.h"
#include "03_edge/language/linda.js/src/_h/01_runtime/_lindaJsRuntime.h"
#include "03_edge/language/linda.js/src/_h/02_api/_lindaJs.h"

using namespace std;
using namespace lindaJs::narr::buildRig;
using namespace lindaJs::runtime;

//====----====----====----====----====----====----====----====---- FDEC

namespace lindaJs::narr::buildRig
{
  namespace /* private */
  {
    TYPE_rc _narr_rc_replaceSemanticBack(STRUCT_context& context);
    TYPE_rc _narr_rc_removeSemanticFront(STRUCT_context& context);
    TYPE_rc _narr_rc_inlineSemanticFx(STRUCT_context& context);
  }
}

//====----====----====----====----====----====----====----====---- FDEF

namespace lindaJs::narr::buildRig
{
  // [1]
  TYPE_rc narr_rc_generateFeatures(STRUCT_context& context)
  {
    // NULLIFY : rc, context.state
    context.state.contextMapping = {};
    context.state.jsonSourceMap.clear();
    TYPE_rc rce = 0;

    // [1-A] Initialize A and B with the source
    rce = to_abs(sourceMap::rc_initSourceMap(context.input.strSource, context.state.contextMapping));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed initializing the source map");
      return rce;
    }

    // [1-B] Replace runtime calls in B
    rce = to_abs(_narr_rc_replaceSemanticBack(context));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed replacing runtime calls");
      return rce;
    }

    // [1-C] Remove IN and OUT declarations from B
    rce = to_abs(_narr_rc_removeSemanticFront(context));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed removing IN and OUT declarations");
      return rce;
    }

    // [1-D] Replace the fx declaration with the runtime process wrapper
    rce = to_abs(_narr_rc_inlineSemanticFx(context));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed inlining the fx declaration");
      return rce;
    }

    // [1-E] Record the A-to-B source map
    context.state.jsonSourceMap = sourceMap::json_sourceMapInfo(context.state.contextMapping);

    // END
    return 0;
  }

  // [2]
  TYPE_rc narr_rc_validateSyntax(const STRUCT_context& context)
  {
    // NULLIFY : rc
    TYPE_rc rci = 0;

    // RCI : create a JavaScript runtime
    JSRuntime* jsRuntime = JS_NewRuntime();
    rci--;
    if (jsRuntime == nullptr)
    {
      vd_printErrorRc(rci, "Failed creating JavaScript runtime for syntax validation");
      return rci;
    }

    // RCI : create a JavaScript context
    JSContext* jsContext = JS_NewContext(jsRuntime);
    rci--;
    if (jsContext == nullptr)
    {
      JS_FreeRuntime(jsRuntime);
      vd_printErrorRc(rci, "Failed creating JavaScript context for syntax validation");
      return rci;
    }

    // RCI : compile generated JavaScript without executing it
    JSValue jsResult = JS_Eval(
      jsContext,
      context.state.contextMapping.strB.data(),
      context.state.contextMapping.strB.size(),
      lindaJs::file::const_nameFilePayloadCore.c_str(),
      JS_EVAL_TYPE_GLOBAL | JS_EVAL_FLAG_COMPILE_ONLY
    );
    rci--;
    if (JS_IsException(jsResult))
    {
      vd_printJsExceptionInfo(json_exceptionInfo(jsContext));
      JS_FreeValue(jsContext, jsResult);
      JS_FreeContext(jsContext);
      JS_FreeRuntime(jsRuntime);
      vd_printErrorRc(rci, "Generated payload has invalid JavaScript syntax");
      return rci;
    }

    // END : release syntax-check state
    JS_FreeValue(jsContext, jsResult);
    JS_FreeContext(jsContext);
    JS_FreeRuntime(jsRuntime);
    return 0;
  }

  // [3]
  TYPE_rc narr_rc_writeRigPackage(const STRUCT_context& context)
  {
    // NULLIFY : rc, output semantic information
    TYPE_rc rce = 0;
    TYPE_json jsonSemanticNonVerbose;

    // SCAN : create the package's non-verbose semantic information
    rce = to_abs(lindaJs::api::rc_scanSemantic(context.input.strSource, false, jsonSemanticNonVerbose));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed scanning non-verbose semantic information");
      return rce;
    }

    // WRITE : generated JavaScript payload
    rce = to_abs(rc_writeFileText(
      lindaJs::file::pathFile_payloadCore(context.input.requestBuildRig.pathDirPackage),
      context.state.contextMapping.strB
    ));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed writing JavaScript payload");
      return rce;
    }

    // WRITE : source-map information
    rce = to_abs(rc_writeFileJson(
      lindaJs::file::pathFile_sourceMap(context.input.requestBuildRig.pathDirPackage),
      context.state.jsonSourceMap
    ));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed writing source-map information");
      return rce;
    }

    // WRITE : non-verbose semantic information
    rce = to_abs(rc_writeFileJson(
      lindaJs::file::pathFile_semantic(context.input.requestBuildRig.pathDirPackage),
      jsonSemanticNonVerbose
    ));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed writing semantic information");
      return rce;
    }

    // WRITE : common RIG package manifest
    const TYPE_json jsonManifest =
    {
      { "type", lindaJs::file::const_typeRigPackage },
      { "schemaVersion", lindaJs::file::const_schemaVersionRigPackage },
      { "languageSupport", lindaJs::file::const_nameLanguageSupport },
      { "semantic", lindaJs::file::const_nameFileSemantic },
      { "error", lindaJs::file::const_pathRelativeFileError }
    };
    rce = to_abs(rc_writeFileJson(lindaJs::file::pathFile_manifest(context.input.requestBuildRig.pathDirPackage), jsonManifest));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed writing RIG package manifest");
      return rce;
    }

    // END
    return 0;
  }
}

//====----====----====----====----====----====----====----====---- FDEF

namespace lindaJs::narr::buildRig
{
  namespace /* private */
  {
    TYPE_rc _narr_rc_replaceSemanticBack(STRUCT_context& context)
    {
      // NULLIFY : rc
      TYPE_rc rce = 0;

      // REPLACE : SPS calls with the runtime sample-rate value
      for (std::size_t idxSps = context.input.jsonSemanticVerbose["back"]["listSps"].size(); idxSps > 0; idxSps--)
      {
        const TYPE_json& semanticSps = context.input.jsonSemanticVerbose["back"]["listSps"][idxSps - 1];
        const sourceMap::REQUEST_rc_replaceText requestReplaceText =
        {
          .rangeA =
          {
            semanticSps["range"]["startByte"].get<std::size_t>(),
            semanticSps["range"]["endByte"].get<std::size_t>()
          },
          .text = const_nameRuntimeSps
        };

        rce = to_abs(sourceMap::rc_replaceText(requestReplaceText, context.state.contextMapping));
        if (rce != 0)
        {
          vd_printErrorRc(rce, "Failed replacing an SPS call");
          return rce;
        }
      }

      // END
      return 0;
    }

    TYPE_rc _narr_rc_removeSemanticFront(STRUCT_context& context)
    {
      // NULLIFY : rc
      TYPE_rc rce = 0;

      // REMOVE : IN declarations
      for (std::size_t idxIn = context.input.jsonSemanticVerbose["front"]["listIn"].size(); idxIn > 0; idxIn--)
      {
        const TYPE_json& semanticIn = context.input.jsonSemanticVerbose["front"]["listIn"][idxIn - 1];
        const sourceMap::REQUEST_rc_replaceText requestRemoveIn =
        {
          .rangeA =
          {
            semanticIn["range"]["startByte"].get<std::size_t>(),
            semanticIn["range"]["endByte"].get<std::size_t>()
          },
          .text = ""
        };

        rce = to_abs(sourceMap::rc_replaceText(requestRemoveIn, context.state.contextMapping));
        if (rce != 0)
        {
          vd_printErrorRc(rce, "Failed removing an IN declaration");
          return rce;
        }
      }

      // REMOVE : OUT declarations
      for (std::size_t idxOut = context.input.jsonSemanticVerbose["front"]["listOut"].size(); idxOut > 0; idxOut--)
      {
        const TYPE_json& semanticOut = context.input.jsonSemanticVerbose["front"]["listOut"][idxOut - 1];
        const sourceMap::REQUEST_rc_replaceText requestRemoveOut =
        {
          .rangeA =
          {
            semanticOut["range"]["startByte"].get<std::size_t>(),
            semanticOut["range"]["endByte"].get<std::size_t>()
          },
          .text = ""
        };

        rce = to_abs(sourceMap::rc_replaceText(requestRemoveOut, context.state.contextMapping));
        if (rce != 0)
        {
          vd_printErrorRc(rce, "Failed removing an OUT declaration");
          return rce;
        }
      }

      // END
      return 0;
    }

    TYPE_rc _narr_rc_inlineSemanticFx(STRUCT_context& context)
    {
      // NULLIFY : rc
      TYPE_rc rci = 0;
      TYPE_rc rce = 0;

      const TYPE_json& jsonFx = context.input.jsonSemanticVerbose["fx"];
      const std::size_t startByteFx = jsonFx["range"]["startByte"].get<std::size_t>();
      const std::size_t startByteBody = jsonFx["body"]["startByte"].get<std::size_t>();
      const std::size_t endByteBody = jsonFx["body"]["endByte"].get<std::size_t>();

      // RCI : validate the fx body boundaries needed for the wrapper
      rci--;
      if (startByteFx >= startByteBody
        || startByteBody >= endByteBody
        || endByteBody > context.input.strSource.size()
        || context.input.strSource[startByteBody] != '{'
        || context.input.strSource[endByteBody - 1] != '}')
      {
        vd_printErrorRc(rci, "The fx declaration has invalid source boundaries");
        return rci;
      }

      // REPLACE : function declaration and opening brace with the processing wrapper
      string strWrapper =
        "globalThis." + const_nameRuntimeProcess
        + " = function(spb, " + const_nameRuntimeSps + ", " + const_nameRuntimeInput + ", " + const_nameRuntimeOutput + ") {\n"
        + "  for (let " + const_nameRuntimeIndex + " = 0; " + const_nameRuntimeIndex + " < spb; " + const_nameRuntimeIndex + "++) {\n";

      for (const TYPE_json& semanticIn : context.input.jsonSemanticVerbose["front"]["listIn"])
      {
        const std::size_t idxSemantic = semanticIn["id"].get<std::size_t>() - 1;
        strWrapper += "    let " + semanticIn["name"].get<string>()
          + " = " + const_nameRuntimeInput + "[" + to_string(idxSemantic)
          + " * spb + " + const_nameRuntimeIndex + "];\n";
      }

      for (const TYPE_json& semanticOut : context.input.jsonSemanticVerbose["front"]["listOut"])
      {
        strWrapper += "    let " + semanticOut["name"].get<string>() + " = 0;\n";
      }

      strWrapper += "    {\n";
      const sourceMap::REQUEST_rc_replaceText requestReplaceFxStart =
      {
        .rangeA = { startByteFx, startByteBody + 1 },
        .text = strWrapper
      };
      rce = to_abs(sourceMap::rc_replaceText(requestReplaceFxStart, context.state.contextMapping));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed replacing the fx declaration start");
        return rce;
      }

      // REPLACE : fx closing brace with output writes and wrapper closure
      string strWrapperEnd = "\n    }\n";
      for (const TYPE_json& semanticOut : context.input.jsonSemanticVerbose["front"]["listOut"])
      {
        const std::size_t idxSemantic = semanticOut["id"].get<std::size_t>() - 1;
        strWrapperEnd += "    " + const_nameRuntimeOutput + "[" + to_string(idxSemantic)
          + " * spb + " + const_nameRuntimeIndex + "] = "
          + semanticOut["name"].get<string>() + ";\n";
      }
      strWrapperEnd += "  }\n};";

      const sourceMap::REQUEST_rc_replaceText requestReplaceFxEnd =
      {
        .rangeA = { endByteBody - 1, endByteBody },
        .text = strWrapperEnd
      };
      rce = to_abs(sourceMap::rc_replaceText(requestReplaceFxEnd, context.state.contextMapping));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed replacing the fx declaration end");
        return rce;
      }

      // END
      return 0;
    }
  }
}
