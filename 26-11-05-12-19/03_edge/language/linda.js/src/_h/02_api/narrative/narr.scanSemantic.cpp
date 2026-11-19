#include "03_edge/language/linda.js/src/_h/02_api/narrative/_narr.scanSemantic.h"

#include <cctype>
#include <string>
#include <utility>
#include <vector>

#include "02_shared/_utility/syntaxQuery.hpp"

using namespace std;
using namespace lindaJs::narr::scanSemantic;

//====----====----====----====----====----====----====----====---- FDEC

// 00_vendored/tree-sitter-javascript/src/parser.c
extern "C" const TSLanguage* tree_sitter_javascript(void);

//====----====----====----====----====----====----====----====---- FDEC

namespace lindaJs::narr::scanSemantic
{
  namespace /* private */
  {
    inline void _narr_vd_initJsonSemanticInfo(STRUCT_context& context);
    inline void _narr_vd_recordSemanticFx(STRUCT_context& context);
    inline void _narr_vd_recordSemanticFront(STRUCT_context& context);
    inline void _narr_vd_recordSemanticBack(STRUCT_context& context);
    inline void _narr_vd_recordReservedRuntimeIdentifierErrors(STRUCT_context& context);
    inline void _narr_vd_buildJsonSemanticOutput(STRUCT_context& context);
    inline string _str_quoteRemoved(const string& text);
  }
}
//====----====----====----====----====----====----====----====---- FDEF

namespace lindaJs::narr::scanSemantic
{
  TYPE_rc narr_rc_getJsonMatchInfo(STRUCT_context& context)
  {
    // NULLIFY : rc, context
    context.state.jsonMatchInfo.clear();
    TYPE_rc rce = 0;

    // QUERY : match the semantic elements used by linda.js
    const string strQuery = R"SCM(
(
  (function_declaration
    name: (identifier) @semantic_fx_name
    body: (statement_block) @semantic_fx_body
  ) @semantic_fx
)

(
  (lexical_declaration
    (variable_declarator
      name: (identifier) @semantic_front_variable
      value: (call_expression
        function: (identifier) @semantic_front_identifier
        arguments: (arguments
          .
          (string) @semantic_front_label?
          .
        )
      )
    )
  ) @semantic_front
)

(
  (call_expression
    function: (identifier) @semantic_back_identifier
    arguments: (arguments) @semantic_back_arguments
  ) @semantic_back
)

(
  (identifier) @semantic_reserved_runtime_identifier
  (#match? @semantic_reserved_runtime_identifier "^__linda_")
)
)SCM";

    TYPE_json temp_out_jsonMatchInfo;
    const vector<pair<string, string>> listAllowedCaptureText =
    {
      {"semantic_front_identifier", "IN"},
      {"semantic_front_identifier", "OUT"},
      {"semantic_fx_name", "fx"},
      {"semantic_back_identifier", "SPS"}
    };
    const vector<string> listReservedPrefix = {"__linda_"};
    const syntaxQuery::REQUEST_rc_querySyntax requestSyntaxQuery =
    {
      .strSource = context.input.strSource,
      .strQuery = strQuery,
      .language = tree_sitter_javascript(),
      .listAllowedCaptureText = listAllowedCaptureText,
      .listReservedPrefix = listReservedPrefix
    };

    rce = to_abs(syntaxQuery::rc_querySyntax(requestSyntaxQuery, temp_out_jsonMatchInfo));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Tree-sitter failed querying the JavaScript source");
      return rce;
    }

    // END
    context.state.jsonMatchInfo = temp_out_jsonMatchInfo;
    return 0;
  }

  void narr_vd_getJsonSemanticInfo(STRUCT_context& context)
  {
    // NULLIFY : rc, context
    context.state.jsonSemanticInfo.clear();

    // [2-A]
    _narr_vd_initJsonSemanticInfo(context);

    // [2-B]
    _narr_vd_recordSemanticFx(context);

    // [2-C]
    _narr_vd_recordSemanticFront(context);

    // [2-D]
    _narr_vd_recordSemanticBack(context);

    // [2-E]
    _narr_vd_recordReservedRuntimeIdentifierErrors(context);

    // [2-F]
    _narr_vd_buildJsonSemanticOutput(context);

    // END
    return;
  }
}

//====----====----====----====----====----====----====----====---- FDEF

namespace lindaJs::narr::scanSemantic
{
  namespace /* private */
  {
    inline string _str_quoteRemoved(const string& text)
    {
      if (text.size() >= 2)
      {
        const char first = text.front();
        const char last = text.back();

        if (true)
        {
          const bool isQuotePairMatched =
          {
            (first == '"' && last == '"')
            || (first == '\'' && last == '\'')
            || (first == '`' && last == '`')
          };
          if (isQuotePairMatched)
          {
            return text.substr(1, text.size() - 2);
          }
        }
      }

      return text;
    }

    inline void _narr_vd_initJsonSemanticInfo(STRUCT_context& context)
    {
      // NULLIFY : context.state.jsonSemanticInfo
      context.state.jsonSemanticInfo.clear();

      // Initialize
      if (true)
      {
        context.state.jsonSemanticInfo["front"] =
        {        
          { "numIn", 0 },
          { "numOut", 0 },
          { "listIn", TYPE_json::array() },
          { "listOut", TYPE_json::array() },
        };
        context.state.jsonSemanticInfo["fx"] =
        {
          { "ok", false },
          {
            "range",
            {
              { "startByte", nullptr },
              { "endByte", nullptr }
            }
          },
          {
            "body",
            {
              { "startByte", nullptr },
              { "endByte", nullptr }
            }
          },
          { "location", nullptr }
        };

        context.state.jsonSemanticInfo["back"] =
        {
          { "numSps", 0 },
          { "listSps", TYPE_json::array() }
        };
        
        context.state.jsonSemanticInfo["problem"] =
        {
          { "numWarning", 0},
          { "numError", 0},
          { "listWarning", TYPE_json::array() },
          { "listError", TYPE_json::array() }
        };
      }

      // END
      return;
    }

    inline void _narr_vd_recordSemanticFx(STRUCT_context& context)
    {
      // track the first fx
      bool isFxRecorded = false;

      // read every match
      auto it = context.state.jsonMatchInfo["listMatch"].begin();
      while (it != context.state.jsonMatchInfo["listMatch"].end())
      {
        const TYPE_json& match = *it;
        const TYPE_json& listCapture = match["listCapture"];

        // CONTINUE
        // check type
        if (true)
        {
          // check "semantic_fx" capture
          if (listCapture[0]["captureName"] != "semantic_fx")
          {
            ++it;
            continue;
          }

          // check function name
          if (listCapture[1]["nodeText"] != "fx")
          {
            ++it;
            continue;
          }
        }

        // CONTINUE
        // check duplicate fx
        if (true)
        {
          if (isFxRecorded)
          {
            TYPE_json error =
            {
              { "id", context.state.jsonSemanticInfo["problem"]["listError"].size() + 1 },
              { "case", "duplicate fx" },
              { "detail", "fx function is declared more than once." },
              { "location", listCapture[0]["startPoint"] }
            };
            context.state.jsonSemanticInfo["problem"]["listError"].push_back(error);
            context.state.jsonSemanticInfo["problem"]["numError"] = context.state.jsonSemanticInfo["problem"]["listError"].size();

            // discard the duplicate without changing the first fx
            it = context.state.jsonMatchInfo["listMatch"].erase(it);
            continue;
          }
          isFxRecorded = true;
        }

        // FOUND
        // record the fx
        if (true)
        {
          context.state.jsonSemanticInfo["fx"]["ok"] = !listCapture[0]["nodeHasError"].get<bool>();
          context.state.jsonSemanticInfo["fx"]["range"]["startByte"] = listCapture[0]["startByte"];
          context.state.jsonSemanticInfo["fx"]["range"]["endByte"] = listCapture[0]["endByte"];
          context.state.jsonSemanticInfo["fx"]["location"] = listCapture[0]["startPoint"];

          // RECORD : fx body range
          for (const TYPE_json& capture : listCapture)
          {
            if (capture["captureName"] == "semantic_fx_body")
            {
              context.state.jsonSemanticInfo["fx"]["body"]["startByte"] = capture["startByte"];
              context.state.jsonSemanticInfo["fx"]["body"]["endByte"] = capture["endByte"];
              break;
            }
          }
        }
        
        // pop out the found
        it = context.state.jsonMatchInfo["listMatch"].erase(it);
      }

      // put error if fx declared
      if (!isFxRecorded)
      {
        TYPE_json error =
        {
          { "id", context.state.jsonSemanticInfo["problem"]["listError"].size() + 1 },
          { "case", "fx missing" },
          { "detail", "No fx function is declared." },
          { "location", nullptr }
        };
        context.state.jsonSemanticInfo["problem"]["listError"].push_back(error);
        context.state.jsonSemanticInfo["problem"]["numError"] = context.state.jsonSemanticInfo["problem"]["listError"].size();
      }

      // END
      return;
    }

    inline void _narr_vd_recordSemanticFront(STRUCT_context& context)
    {
      // read every match
      auto it = context.state.jsonMatchInfo["listMatch"].begin();
      while (it != context.state.jsonMatchInfo["listMatch"].end())
      {
        const TYPE_json& match = *it;
        const TYPE_json& listCapture = match["listCapture"];

        // CONTINUE
        // check type
        string typeFront;
        if (true)
        {
          // check "semantic_front" capture
          if (listCapture[0]["captureName"] != "semantic_front")
          {
            ++it;
            continue;
          }

          // check type validity (IN, OUT)
          typeFront = listCapture[2]["nodeText"].get<string>();
          if((typeFront != "IN") && (typeFront != "OUT"))
          {
            ++it;
            continue;
          }
        }

        // ERROR
        // declaration inside fx
        if (!context.state.jsonSemanticInfo["fx"]["range"]["startByte"].is_null())
        {
          bool isInsideFx = 
            (context.state.jsonSemanticInfo["fx"]["range"]["startByte"] <= listCapture[0]["startByte"])
            && (listCapture[0]["endByte"] <= context.state.jsonSemanticInfo["fx"]["range"]["endByte"]);
          
          if(isInsideFx)
          {
            TYPE_json error =
            {
              { "id", context.state.jsonSemanticInfo["problem"]["listError"].size() + 1 },
              { "case", "semantic " + typeFront + " declared inside fx" },
              { "detail", "semantic " + typeFront + " '" + listCapture[1]["nodeText"].get<string>() + "' is declared inside fx." },
              { "location", listCapture[0]["startPoint"] }
            };
            context.state.jsonSemanticInfo["problem"]["listError"].push_back(error);
            context.state.jsonSemanticInfo["problem"]["numError"] = context.state.jsonSemanticInfo["problem"]["listError"].size();

            // discard the invalid declaration
            it = context.state.jsonMatchInfo["listMatch"].erase(it);
            continue;
          }
        }

        // ERROR
        // duplicate semantic front
        if (true)
        {
          const string nameFront = listCapture[1]["nodeText"].get<string>();
          bool isDuplicate = false;

          for (const TYPE_json& front : context.state.jsonSemanticInfo["front"]["listIn"])
          {
            if (front["name"] == nameFront)
            {
              isDuplicate = true;
              break;
            }
          }

          if (!isDuplicate)
          {
            for (const TYPE_json& front : context.state.jsonSemanticInfo["front"]["listOut"])
            {
              if (front["name"] == nameFront)
              {
                isDuplicate = true;
                break;
              }
            }
          }

          if (isDuplicate)
          {
            TYPE_json error =
            {
              { "id", context.state.jsonSemanticInfo["problem"]["listError"].size() + 1 },
              { "case", "duplicate semantic " + typeFront },
              { "detail", "semantic " + typeFront + " '" + nameFront + "' is declared more than once." },
              { "location", listCapture[0]["startPoint"] }
            };
            context.state.jsonSemanticInfo["problem"]["listError"].push_back(error);
            context.state.jsonSemanticInfo["problem"]["numError"] = context.state.jsonSemanticInfo["problem"]["listError"].size();

            // discard the duplicated declaration
            it = context.state.jsonMatchInfo["listMatch"].erase(it);
            continue;
          }
        }

        // FOUND
        // init semantic front element
        TYPE_json item;
        if (true)
        {
          item =
          {
            { "id", 0 },
            { "name", "" },
            { "meta", TYPE_json::object() }
          };
          item["range"]["startByte"] = listCapture[0]["startByte"];
          item["range"]["endByte"] = listCapture[0]["endByte"];
          item["location"] = listCapture[0]["startPoint"];
        }      

        // set name & meta
        if (true)
        {
          item["name"] = listCapture[1]["nodeText"];

          // meta.label
          item["meta"]["label"] = item["name"];
          for (const TYPE_json& capture : listCapture)
          {
            if (capture["captureName"] == "semantic_front_label")
            {
              item["meta"]["label"] = _str_quoteRemoved(capture["nodeText"].get<string>());
              break;
            }
          }
        }

        // type route
        if (typeFront == "IN")
        {
          item["id"] = context.state.jsonSemanticInfo["front"]["listIn"].size() + 1;
          context.state.jsonSemanticInfo["front"]["listIn"].push_back(item);
          context.state.jsonSemanticInfo["front"]["numIn"] = context.state.jsonSemanticInfo["front"]["listIn"].size();
        }
        else if (typeFront == "OUT")
        {
          item["id"] = context.state.jsonSemanticInfo["front"]["listOut"].size() + 1;
          context.state.jsonSemanticInfo["front"]["listOut"].push_back(item);
          context.state.jsonSemanticInfo["front"]["numOut"] = context.state.jsonSemanticInfo["front"]["listOut"].size();
        }

        // pop out the found
        it = context.state.jsonMatchInfo["listMatch"].erase(it);
      }

      // WARNING
      // duplicate semantic label
      if (true)
      {
        TYPE_json listFront = TYPE_json::array();
        for (const TYPE_json& front : context.state.jsonSemanticInfo["front"]["listIn"])
        {
          listFront.push_back(front);
        }
        for (const TYPE_json& front : context.state.jsonSemanticInfo["front"]["listOut"])
        {
          listFront.push_back(front);
        }

        for (size_t idxFront = 0; idxFront < listFront.size(); idxFront++)
        {
          const TYPE_json& front = listFront[idxFront];
          const string label = front["meta"]["label"].get<string>();
          bool isDuplicate = false;

          for (size_t idxPrevious = 0; idxPrevious < idxFront; idxPrevious++)
          {
            if (listFront[idxPrevious]["meta"]["label"] == label)
            {
              isDuplicate = true;
              break;
            }
          }

          if (isDuplicate)
          {
            TYPE_json warning =
            {
              { "id", context.state.jsonSemanticInfo["problem"]["listWarning"].size() + 1 },
              { "case", "duplicate label" },
              { "detail", "label '" + label + "' is used more than once." },
              { "location", front.value("location", TYPE_json(nullptr)) }
            };
            context.state.jsonSemanticInfo["problem"]["listWarning"].push_back(warning);
            context.state.jsonSemanticInfo["problem"]["numWarning"] = context.state.jsonSemanticInfo["problem"]["listWarning"].size();
          }
        }
      }

      // END
      return;
    }

    inline void _narr_vd_recordSemanticBack(STRUCT_context& context)
    {
      // read every match
      auto it = context.state.jsonMatchInfo["listMatch"].begin();
      while (it != context.state.jsonMatchInfo["listMatch"].end())
      {
        const TYPE_json& match = *it;
        const TYPE_json& listCapture = match["listCapture"];

        // type check
        if (true)
        {
          // check "semantic_back" capture
          if (listCapture[0]["captureName"] != "semantic_back")
          {
            ++it;
            continue;
          }

          // check function name
          if (listCapture[1]["nodeText"] != "SPS")
          {
            ++it;
            continue;
          }
        }

        // FOUND
        // init semantic back element
        TYPE_json item;
        if (true)
        {
          item =
          {
            { "id", context.state.jsonSemanticInfo["back"]["listSps"].size() + 1 },
            {
              "range",
              {
                { "startByte", listCapture[0]["startByte"] },
                { "endByte", listCapture[0]["endByte"] }
              }
            },
            { "location", listCapture[0]["startPoint"] }
          };
        }

        // record semantic back
        if (true)
        {
          context.state.jsonSemanticInfo["back"]["listSps"].push_back(item);
          context.state.jsonSemanticInfo["back"]["numSps"] = context.state.jsonSemanticInfo["back"]["listSps"].size();
        }

        // WARNING
        // SPS() has arguments
        if (true)
        {
          string arguments = listCapture[2]["nodeText"].get<string>();
          for (auto itArgument = arguments.begin(); itArgument != arguments.end();)
          {
            if (isspace(static_cast<unsigned char>(*itArgument)) != 0)
            {
              itArgument = arguments.erase(itArgument);
            }
            else
            {
              ++itArgument;
            }
          }

          if (arguments != "()")
          {
            TYPE_json warning =
            {
              { "id", context.state.jsonSemanticInfo["problem"]["listWarning"].size() + 1 },
              { "case", "SPS() has arguments" },
              { "detail", "SPS() does not accept arguments; arguments are ignored." },
              { "location", listCapture[0]["startPoint"] }
            };
            context.state.jsonSemanticInfo["problem"]["listWarning"].push_back(warning);
            context.state.jsonSemanticInfo["problem"]["numWarning"] = context.state.jsonSemanticInfo["problem"]["listWarning"].size();
          }
        }

        // pop out the found
        it = context.state.jsonMatchInfo["listMatch"].erase(it);
      }

      // END
      return;
    }

    inline void _narr_vd_recordReservedRuntimeIdentifierErrors(STRUCT_context& context)
    {
      // read every match
      auto it = context.state.jsonMatchInfo["listMatch"].begin();
      while (it != context.state.jsonMatchInfo["listMatch"].end())
      {
        const TYPE_json& match = *it;
        const TYPE_json& listCapture = match["listCapture"];

        // CONTINUE
        // check capture type
        if (listCapture[0]["captureName"] != "semantic_reserved_runtime_identifier")
        {
          ++it;
          continue;
        }

        // ERROR
        // reserved runtime identifier
        TYPE_json error =
        {
          { "id", context.state.jsonSemanticInfo["problem"]["listError"].size() + 1 },
          { "case", "reserved runtime identifier" },
          { "detail", "Runtime identifier '" + listCapture[0]["nodeText"].get<string>() + "' is reserved." },
          { "location", listCapture[0]["startPoint"] }
        };
        context.state.jsonSemanticInfo["problem"]["listError"].push_back(error);
        context.state.jsonSemanticInfo["problem"]["numError"] = context.state.jsonSemanticInfo["problem"]["listError"].size();

        it = context.state.jsonMatchInfo["listMatch"].erase(it);
      }

      // END
      return;
    }

    inline void _narr_vd_buildJsonSemanticOutput(STRUCT_context& context)
    {
      if (context.input.isVerbose)
      {
        // END
        return;
      }

      // READ : preserve verbose semantic information
      TYPE_json verbose_jsonSemanticInfo = context.state.jsonSemanticInfo;
      context.state.jsonSemanticInfo.clear();

      // INIT : non-verbose semantic output
      context.state.jsonSemanticInfo["front"] =
      {
        { "numIn", verbose_jsonSemanticInfo["front"]["numIn"] },
        { "numOut", verbose_jsonSemanticInfo["front"]["numOut"] },
        { "listIn", TYPE_json::array() },
        { "listOut", TYPE_json::array() }
      };
      context.state.jsonSemanticInfo["fx"] =
      {
        { "ok", verbose_jsonSemanticInfo["fx"]["ok"] }
      };
      context.state.jsonSemanticInfo["problem"] = verbose_jsonSemanticInfo["problem"];

      // RECORD : non-verbose input elements
      for (const TYPE_json& item : verbose_jsonSemanticInfo["front"]["listIn"])
      {
        context.state.jsonSemanticInfo["front"]["listIn"].push_back
        (
          {
            { "id", item["id"] },
            { "name", item["name"] },
            { "meta", item["meta"] }
          }
        );
      }

      // RECORD : non-verbose output elements
      for (const TYPE_json& item : verbose_jsonSemanticInfo["front"]["listOut"])
      {
        context.state.jsonSemanticInfo["front"]["listOut"].push_back
        (
          {
            { "id", item["id"] },
            { "name", item["name"] },
            { "meta", item["meta"] }
          }
        );
      }

      // END
      return;
    }
  }
}
