#pragma once

/*
  Tree-sitter syntax-query utilities.

  Compiles a query, parses source code and returns matches with their captures.

  vendored backend:
    https://github.com/tree-sitter/tree-sitter
*/

#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/json.hpp"
#include "00_vendored/tree-sitter/lib/include/tree_sitter/api.h"

//====----====----====----====----====----====----====----====---- TYPE

namespace syntaxQuery
{
  struct REQUEST_rc_querySyntax
  {
    const std::string& strSource;
    const std::string& strQuery;
    const TSLanguage* language = nullptr;
    const std::vector<std::pair<std::string, std::string>>& listAllowedCaptureText;
    const std::vector<std::string>& listReservedPrefix;
  };
}

//====----====----====----====----====----====----====----====---- FDEC

namespace syntaxQuery
{
  static inline TYPE_rc rc_querySyntax(const REQUEST_rc_querySyntax& request, TYPE_json& out_jsonMatchInfo);

  namespace /* private */
  {
    inline const char* _str_nameQueryError(TSQueryError errorType);
    inline TSNode _node_firstSyntaxError(TSNode nodeRoot);
  }
}

//====----====----====----====----====----====----====----====---- FDEF

namespace syntaxQuery
{
  static inline TYPE_rc rc_querySyntax(const REQUEST_rc_querySyntax& request, TYPE_json& out_jsonMatchInfo)
  {
    // NULLIFY : rc, out
    out_jsonMatchInfo = nullptr;
    TYPE_rc rci = 0;

    // RCI : validate Tree-sitter input sizes
    rci--;
    if (request.strSource.size() > (std::numeric_limits<uint32_t>::max)()
      || request.strQuery.size() > (std::numeric_limits<uint32_t>::max)())
    {
      vd_printErrorRc(rci, "Tree-sitter source or query exceeds the supported byte size");
      return rci;
    }

    // RCI : validate the language
    rci--;
    if (request.language == nullptr)
    {
      vd_printErrorRc(rci, "Tree-sitter language is null");
      return rci;
    }

    // COMPILE : validate the query before parsing source
    uint32_t errorByte = 0;
    TSQueryError errorType = TSQueryErrorNone;
    TSQuery* query = ts_query_new(
      request.language,
      request.strQuery.c_str(),
      static_cast<uint32_t>(request.strQuery.size()),
      &errorByte,
      &errorType
    );

    rci--;
    if (query == nullptr)
    {
      vd_printErrorVendor(
        "Tree-sitter",
        std::string(_str_nameQueryError(errorType))
        + " query error at byte "
        + std::to_string(errorByte)
      );
      vd_printErrorRc(rci, "Failed compiling the Tree-sitter query");
      return rci;
    }

    // CREATE : parser and assign its language
    TSParser* parser = ts_parser_new();
    rci--;
    if (parser == nullptr)
    {
      ts_query_delete(query);
      vd_printErrorRc(rci, "Failed creating the Tree-sitter parser");
      return rci;
    }

    rci--;
    if (!ts_parser_set_language(parser, request.language))
    {
      ts_query_delete(query);
      ts_parser_delete(parser);
      vd_printErrorRc(rci, "Tree-sitter parser rejected the requested language");
      return rci;
    }

    // PARSE : build the source syntax tree
    TSTree* tree = ts_parser_parse_string(
      parser,
      nullptr,
      request.strSource.c_str(),
      static_cast<uint32_t>(request.strSource.size())
    );

    rci--;
    if (tree == nullptr)
    {
      ts_query_delete(query);
      ts_parser_delete(parser);
      vd_printErrorRc(rci, "Tree-sitter failed parsing the source");
      return rci;
    }

    // RCI : report source syntax errors before running the query
    const TSNode nodeRoot = ts_tree_root_node(tree);
    const bool isSourceSyntaxValid = !ts_node_has_error(nodeRoot);
    rci--;
    if (!isSourceSyntaxValid)
    {
      const TSNode nodeError = _node_firstSyntaxError(nodeRoot);
      const uint32_t startByteError = ts_node_is_null(nodeError) ? 0 : ts_node_start_byte(nodeError);
      const uint32_t endByteError = ts_node_is_null(nodeError) ? 0 : ts_node_end_byte(nodeError);
      const std::string nameErrorNode = ts_node_is_null(nodeError) ? "Unknown" : ts_node_type(nodeError);

      ts_tree_delete(tree);
      ts_query_delete(query);
      ts_parser_delete(parser);

      vd_printErrorVendor(
        "Tree-sitter",
        "Source syntax error (" + nameErrorNode + ") at byte "
        + std::to_string(startByteError) + "-" + std::to_string(endByteError)
      );
      vd_printErrorRc(rci, "Source contains syntax errors");
      return rci;
    }

    // QUERY : collect matching nodes and captures
    TSQueryCursor* cursor = ts_query_cursor_new();
    rci--;
    if (cursor == nullptr)
    {
      ts_tree_delete(tree);
      ts_query_delete(query);
      ts_parser_delete(parser);
      vd_printErrorRc(rci, "Failed creating the Tree-sitter query cursor");
      return rci;
    }

    ts_query_cursor_exec(cursor, query, nodeRoot);

    TYPE_json jsonMatchInfoTemp =
    {
      {"numMatch", 0},
      {"listMatch", TYPE_json::array()}
    };

    TSQueryMatch match = {};
    while (ts_query_cursor_next_match(cursor, &match))
    {
      TYPE_json jsonMatch =
      {
        {"numCapture", 0},
        {"listCapture", TYPE_json::array()}
      };
      bool isMatchAllowed = request.listAllowedCaptureText.empty()
        && request.listReservedPrefix.empty();

      for (uint32_t idxCapture = 0; idxCapture < match.capture_count; idxCapture++)
      {
        const TSQueryCapture& queryCapture = match.captures[idxCapture];
        uint32_t sizeNameCapture = 0;
        const char* nameCapture = ts_query_capture_name_for_id(query, queryCapture.index, &sizeNameCapture);
        const uint32_t startByte = ts_node_start_byte(queryCapture.node);
        const uint32_t endByte = ts_node_end_byte(queryCapture.node);
        const std::string strNameCapture(nameCapture, sizeNameCapture);
        const std::string strNodeText = request.strSource.substr(startByte, endByte - startByte);
        const TSPoint pointStart = ts_node_start_point(queryCapture.node);
        const TSPoint pointEnd = ts_node_end_point(queryCapture.node);

        for (const std::pair<std::string, std::string>& allowedCaptureText : request.listAllowedCaptureText)
        {
          if (allowedCaptureText.first == strNameCapture && allowedCaptureText.second == strNodeText)
          {
            isMatchAllowed = true;
            break;
          }
        }

        for (const std::string& reservedPrefix : request.listReservedPrefix)
        {
          if (!reservedPrefix.empty()
            && strNodeText.compare(0, reservedPrefix.size(), reservedPrefix) == 0)
          {
            isMatchAllowed = true;
            break;
          }
        }

        jsonMatch["listCapture"].push_back(
          {
            {"captureIdx", queryCapture.index},
            {"captureName", strNameCapture},
            {"nodeType", ts_node_type(queryCapture.node)},
            {"nodeText", strNodeText},
            {"nodeHasError", ts_node_has_error(queryCapture.node)},
            {"startByte", startByte},
            {"endByte", endByte},
            {"lengthByte", endByte - startByte},
            {"startPoint", {{"row", pointStart.row}, {"column", pointStart.column}}},
            {"endPoint", {{"row", pointEnd.row}, {"column", pointEnd.column}}}
          }
        );
      }

      if (isMatchAllowed)
      {
        jsonMatch["numCapture"] = match.capture_count;
        jsonMatchInfoTemp["listMatch"].push_back(jsonMatch);
      }
    }

    jsonMatchInfoTemp["numMatch"] = jsonMatchInfoTemp["listMatch"].size();

    // END : release parser state and return match information
    ts_query_cursor_delete(cursor);
    ts_tree_delete(tree);
    ts_query_delete(query);
    ts_parser_delete(parser);
    out_jsonMatchInfo = std::move(jsonMatchInfoTemp);
    return 0;
  }

//====----====----====----====----====----====----====----====---- FDEF

  namespace /* private */
  {
    inline const char* _str_nameQueryError(TSQueryError errorType)
    {
      switch (errorType)
      {
        case TSQueryErrorSyntax:
          return "Syntax";
        case TSQueryErrorNodeType:
          return "NodeType";
        case TSQueryErrorField:
          return "Field";
        case TSQueryErrorCapture:
          return "Capture";
        case TSQueryErrorStructure:
          return "Structure";
        case TSQueryErrorLanguage:
          return "Language";
        default:
          return "Unknown";
      }
    }

    inline TSNode _node_firstSyntaxError(TSNode nodeRoot)
    {
      TSTreeCursor cursorError = ts_tree_cursor_new(nodeRoot);
      TSNode nodeError = {};
      bool isTraversalComplete = false;

      while (!isTraversalComplete)
      {
        const TSNode nodeCurrent = ts_tree_cursor_current_node(&cursorError);
        if (ts_node_is_error(nodeCurrent) || ts_node_is_missing(nodeCurrent))
        {
          nodeError = nodeCurrent;
          break;
        }

        if (ts_tree_cursor_goto_first_child(&cursorError))
        {
          continue;
        }

        while (!ts_tree_cursor_goto_next_sibling(&cursorError))
        {
          if (!ts_tree_cursor_goto_parent(&cursorError))
          {
            isTraversalComplete = true;
            break;
          }
        }
      }

      ts_tree_cursor_delete(&cursorError);
      return nodeError;
    }
  }
}
