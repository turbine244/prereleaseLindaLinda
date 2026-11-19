#include "03_edge/language/linda.js/src/_h/01_runtime/_lindaJsRuntime.h"

#include <algorithm>
#include <charconv>
#include <regex>
#include <sstream>
#include <string>
#include <system_error>

using namespace std;

//====----====----====----====----====----====----====----====---- FDEC

namespace lindaJs::runtime
{
  namespace /* private */
  {
    string _str_textFromJsValue(JSContext* jsContext, JSValueConst jsValue);
    TYPE_json _json_listFrameFromStack(const string& stack);
    size_t _to_byteFromPoint(const string& text, const TYPE_json& point);
    TYPE_json _json_pointFromByte(const string& text, size_t byte);
  }
}

//====----====----====----====----====----====----====----====---- FDEF

namespace lindaJs::runtime
{
  TYPE_json json_exceptionInfo(JSContext* jsContext)
  {
    TYPE_json jsonExceptionInfo =
    {
      {"kind", "Error"},
      {"message", "Unknown JavaScript exception"},
      {"stack", ""},
      {"listFrame", TYPE_json::array()},
      {"listCause", TYPE_json::array()}
    };

    JSValue jsCurrent = JS_GetException(jsContext);
    for (uint32_t depth = 0; ; depth++)
    {
      const string text = _str_textFromJsValue(jsContext, jsCurrent);
      string stack;
      if (JS_IsError(jsCurrent))
      {
        JSValue jsStack = JS_GetPropertyStr(jsContext, jsCurrent, "stack");
        if (JS_IsException(jsStack))
        {
          JS_FreeValue(jsContext, JS_GetException(jsContext));
        }
        else
        {
          if (!JS_IsUndefined(jsStack))
          {
            stack = _str_textFromJsValue(jsContext, jsStack);
          }
          JS_FreeValue(jsContext, jsStack);
        }
      }

      if (depth == 0)
      {
        const size_t separator = text.find(": ");
        jsonExceptionInfo["kind"] = separator == string::npos ? "Error" : text.substr(0, separator);
        jsonExceptionInfo["message"] = separator == string::npos ? text : text.substr(separator + 2);
        jsonExceptionInfo["stack"] = stack;
        jsonExceptionInfo["listFrame"] = _json_listFrameFromStack(stack);
      }
      else
      {
        jsonExceptionInfo["listCause"].push_back({{"text", text}, {"stack", stack}, {"listFrame", _json_listFrameFromStack(stack)}});
      }

      if (!JS_IsError(jsCurrent) || depth >= 10)
      {
        break;
      }

      JSValue jsCause = JS_GetPropertyStr(jsContext, jsCurrent, "cause");
      if (JS_IsException(jsCause))
      {
        JS_FreeValue(jsContext, JS_GetException(jsContext));
        break;
      }
      if (JS_IsUndefined(jsCause))
      {
        JS_FreeValue(jsContext, jsCause);
        break;
      }

      JS_FreeValue(jsContext, jsCurrent);
      jsCurrent = jsCause;
    }

    JS_FreeValue(jsContext, jsCurrent);
    return jsonExceptionInfo;
  }

  void vd_printJsExceptionInfo(const TYPE_json& jsonExceptionInfo)
  {
    string message =
      jsonExceptionInfo.value("kind", string("Error"))
      + ": "
      + jsonExceptionInfo.value("message", string("Unknown JavaScript exception"));

    const string stack = jsonExceptionInfo.value("stack", string(""));
    if (!stack.empty())
    {
      message += "\n" + stack;
    }

    if (jsonExceptionInfo.contains("listCause") && jsonExceptionInfo["listCause"].is_array())
    {
      for (const TYPE_json& jsonCause : jsonExceptionInfo["listCause"])
      {
        message += "\nCaused by: " + jsonCause.value("text", string("Unknown JavaScript exception"));
        const string causeStack = jsonCause.value("stack", string(""));
        if (!causeStack.empty())
        {
          message += "\n" + causeStack;
        }
      }
    }

    vd_printErrorVendor("QuickJS", message);

    // END
    return;
  }

  void vd_resolveExceptionInfo(const TYPE_json& jsonExceptionInfo, const STRUCT_debugInfo& debugInfo, TYPE_json& out_jsonExceptionInfoResolved)
  {
    // NULLIFY : out
    out_jsonExceptionInfoResolved = nullptr;

    // INIT : temp out from input
    TYPE_json temp_out_jsonExceptionInfoResolved = jsonExceptionInfo;

    // RETURN : source-map lookup requires complete source, payload, and segment data.
    if (true)
    {
      const bool isSourceResolvable =
      {
        !debugInfo.strPayload.empty()
        && !debugInfo.strSource.empty()
        && debugInfo.jsonSourceMap.is_object()
        && debugInfo.jsonSourceMap.contains("listMapSegment")
        && debugInfo.jsonSourceMap["listMapSegment"].is_array()
        && temp_out_jsonExceptionInfoResolved.contains("listFrame")
        && temp_out_jsonExceptionInfoResolved["listFrame"].is_array()
      };
      if (!isSourceResolvable)
      {
        out_jsonExceptionInfoResolved = temp_out_jsonExceptionInfoResolved;
        return;
      }
    }

    // RESOLVE : map payload frames to their original source locations.
    for (TYPE_json& jsonFrame : temp_out_jsonExceptionInfoResolved["listFrame"])
    {
      // SKIP : malformed payload-frame entries cannot be mapped.
      if (true)
      {
        const bool isFrameValid =
        {
          jsonFrame.is_object()
          && jsonFrame.contains("payload")
          && jsonFrame["payload"].is_object()
        };
        if (!isFrameValid)
        {
          continue;
        }
      }

      TYPE_json& jsonPayloadFrame = jsonFrame["payload"];
      jsonPayloadFrame["path"] = debugInfo.pathFilePayloadCore.generic_string();

      // SKIP : a payload location without a point cannot be mapped.
      if (true)
      {
        const bool isPointContained =
        {
          jsonPayloadFrame.contains("point")
          && jsonPayloadFrame["point"].is_object()
        };
        if (!isPointContained)
        {
          continue;
        }
      }

      const size_t bytePayload = _to_byteFromPoint(debugInfo.strPayload, jsonPayloadFrame["point"]);

      for (const TYPE_json& jsonSegment : debugInfo.jsonSourceMap["listMapSegment"])
      {
        // SKIP : inserted payload text has no original source range.
        if (true)
        {
          const bool isSegmentValid =
          {
            jsonSegment.is_object()
            && jsonSegment.contains("A")
            && jsonSegment["A"].is_object()
            && jsonSegment.contains("B")
            && jsonSegment["B"].is_object()
          };
          if (!isSegmentValid)
          {
            continue;
          }
        }

        const TYPE_json& jsonRangeB = jsonSegment["B"];
        const TYPE_json& jsonRangeA = jsonSegment["A"];

        // SKIP : ranges without numeric byte boundaries cannot be compared.
        if (true)
        {
          const bool isByteRangeInfoValid =
          {
            jsonRangeB.contains("startByte")
            && jsonRangeB.contains("endByte")
            && jsonRangeA.contains("startByte")
            && jsonRangeA.contains("endByte")
            && jsonRangeB["startByte"].is_number()
            && jsonRangeB["endByte"].is_number()
            && jsonRangeA["startByte"].is_number()
            && jsonRangeA["endByte"].is_number()
          };
          if (!isByteRangeInfoValid)
          {
            continue;
          }
        }

        const size_t startByteB = jsonRangeB["startByte"].get<size_t>();
        const size_t endByteB = jsonRangeB["endByte"].get<size_t>();
        const size_t startByteA = jsonRangeA["startByte"].get<size_t>();
        const size_t endByteA = jsonRangeA["endByte"].get<size_t>();

        // SKIP : continue until this segment contains the payload byte.
        if (true)
        {
          const bool isSegmentMatch =
          {
            startByteB < endByteB
            && startByteA < endByteA
            && bytePayload >= startByteB
            && bytePayload < endByteB
          };
          if (!isSegmentMatch)
          {
            continue;
          }
        }

        const size_t sizeRangeB = endByteB - startByteB;
        const size_t sizeRangeA = endByteA - startByteA;
        const size_t offsetB = bytePayload - startByteB;
        const bool isContentReplaced = jsonSegment.value("isContentReplaced", false);
        if (!isContentReplaced && sizeRangeA != sizeRangeB)
        {
          continue;
        }

        const size_t offsetA = sizeRangeA == sizeRangeB
          ? offsetB
          : min((offsetB * sizeRangeA) / sizeRangeB, sizeRangeA - 1);
        jsonFrame["source"] =
        {
          {"path", debugInfo.pathFileSource.generic_string()},
          {"point", _json_pointFromByte(debugInfo.strSource, startByteA + offsetA)}
        };
        break;
      }
    }

    out_jsonExceptionInfoResolved = temp_out_jsonExceptionInfoResolved;

    // END
    return;
  }

  TYPE_json json_runtimeError(const string& stage, TYPE_rc rc, const TYPE_json& jsonExceptionInfo, const filesystem::path& pathFileSourceMap)
  {
    TYPE_json jsonError =
    {
      {"stage", stage},
      {"rc", static_cast<int>(rc)},
      {"kind", jsonExceptionInfo.value("kind", string("Error"))},
      {"message", jsonExceptionInfo.value("message", string("Unknown JavaScript exception"))},
      {"sourceMap", pathFileSourceMap.generic_string()},
      {"stack", jsonExceptionInfo.value("stack", string(""))},
      {"listFrame", jsonExceptionInfo.value("listFrame", TYPE_json::array())},
      {"listCause", jsonExceptionInfo.value("listCause", TYPE_json::array())}
    };

    if (jsonError["listFrame"].is_array() && !jsonError["listFrame"].empty())
    {
      jsonError["payload"] = jsonError["listFrame"][0]["payload"];
      if (jsonError["listFrame"][0].contains("source"))
      {
        jsonError["source"] = jsonError["listFrame"][0]["source"];
      }
    }

    return jsonError;
  }

  void vd_releaseJavascriptRuntime(HANDLE_lindaJsRuntime* handle_lindaJsRuntime)
  {
    if (handle_lindaJsRuntime == nullptr)
    {
      return;
    }

    if (handle_lindaJsRuntime->jsContext != nullptr)
    {
      JS_FreeValue(handle_lindaJsRuntime->jsContext, handle_lindaJsRuntime->jsVarBufferOut);
      JS_FreeValue(handle_lindaJsRuntime->jsContext, handle_lindaJsRuntime->jsVarBufferIn);
      JS_FreeValue(handle_lindaJsRuntime->jsContext, handle_lindaJsRuntime->jsFunctionProcess);
      JS_FreeContext(handle_lindaJsRuntime->jsContext);
      handle_lindaJsRuntime->jsContext = nullptr;
    }

    if (handle_lindaJsRuntime->jsRuntime != nullptr)
    {
      JS_FreeRuntime(handle_lindaJsRuntime->jsRuntime);
      handle_lindaJsRuntime->jsRuntime = nullptr;
    }

    delete handle_lindaJsRuntime;

    // END
    return;
  }
}

//====----====----====----====----====----====----====----====---- FDEF

namespace lindaJs::runtime
{
  namespace /* private */
  {
    string _str_textFromJsValue(JSContext* jsContext, JSValueConst jsValue)
    {
      const char* text = JS_ToCString(jsContext, jsValue);
      if (text == nullptr)
      {
        JS_FreeValue(jsContext, JS_GetException(jsContext));
        return "[unprintable JavaScript value]";
      }

      string result = text;
      JS_FreeCString(jsContext, text);
      return result;
    }

    TYPE_json _json_listFrameFromStack(const string& stack)
    {
      TYPE_json listFrame = TYPE_json::array();
      const regex pattern(R"(^\s*at\s+(?:(.*?)\s+\()?(.+):([0-9]+):([0-9]+)\)?\s*$)");
      istringstream stream(stack);
      string line;
      while (getline(stream, line))
      {
        smatch match;
        if (!regex_match(line, match, pattern))
        {
          continue;
        }

        const string strRow = match[3].str();
        const string strColumn = match[4].str();
        uint32_t rowValue = 0;
        uint32_t columnValue = 0;
        const from_chars_result resultRow = from_chars(strRow.data(), strRow.data() + strRow.size(), rowValue);
        const from_chars_result resultColumn = from_chars(strColumn.data(), strColumn.data() + strColumn.size(), columnValue);
        if (resultRow.ec != errc{} || resultRow.ptr != strRow.data() + strRow.size()
          || resultColumn.ec != errc{} || resultColumn.ptr != strColumn.data() + strColumn.size())
        {
          continue;
        }

        const uint32_t row = rowValue == 0 ? 0 : rowValue - 1;
        const uint32_t column = columnValue == 0 ? 0 : columnValue - 1;
        listFrame.push_back(
          {
            {"name", match[1].matched ? match[1].str() : ""},
            {
              "payload",
              {
                {"path", match[2].str()},
                {"point", {{"row", row}, {"column", column}}}
              }
            }
          }
        );
      }
      return listFrame;
    }

    size_t _to_byteFromPoint(const string& text, const TYPE_json& point)
    {
      const uint32_t targetRow = point.value("row", 0U);
      const uint32_t targetColumn = point.value("column", 0U);
      size_t lineStart = 0;
      for (uint32_t row = 0; row < targetRow; row++)
      {
        const size_t lineEnd = text.find('\n', lineStart);
        if (lineEnd == string::npos)
        {
          return text.size();
        }
        lineStart = lineEnd + 1;
      }

      const size_t lineEnd = text.find('\n', lineStart);
      const size_t contentEnd = lineEnd == string::npos ? text.size() : lineEnd;
      return min(lineStart + targetColumn, contentEnd);
    }

    TYPE_json _json_pointFromByte(const string& text, size_t byte)
    {
      byte = min(byte, text.size());
      const size_t lineStart = byte == 0 ? 0 : text.rfind('\n', byte - 1) + 1;
      return TYPE_json
      {
        {"row", static_cast<uint32_t>(count(text.begin(), text.begin() + byte, '\n'))},
        {"column", static_cast<uint32_t>(byte - lineStart)}
      };
    }
  }
}
