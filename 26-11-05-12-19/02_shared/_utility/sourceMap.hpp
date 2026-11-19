#pragma once

/*
  Utilities for modifying text A into text B while recording element traces.
  String edits and source-map segment updates are applied together.
*/

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/json.hpp"

//====----====----====----====----====----====----====----====---- TYPE

namespace sourceMap
{
  struct STRUCT_range
  {
    std::size_t startByte = 0;
    std::size_t endByte = 0;
  };

  struct STRUCT_mapSegment
  {
    STRUCT_range rangeA = { std::string::npos, std::string::npos };
    STRUCT_range rangeB = { 0, 0 };
    bool isContentReplaced = false;
  };

  struct REQUEST_rc_insertText
  {
    std::size_t startByteB = 0;
    std::string text = "";
  };

  struct REQUEST_rc_replaceText
  {
    STRUCT_range rangeA = { 0, 0 };
    std::string text = "";
  };

  struct STRUCT_contextMapping
  {
    std::string strA = "";
    std::string strB = "";
    std::vector<STRUCT_mapSegment> listMapSegment = {};
  };

}

//====----====----====----====----====----====----====----====---- FDEC

namespace sourceMap
{
  static inline TYPE_rc rc_initSourceMap(const std::string& strA, STRUCT_contextMapping& out_contextMapping);
  static inline TYPE_rc rc_insertText(const REQUEST_rc_insertText& request, STRUCT_contextMapping& edit_contextMapping);
  static inline TYPE_rc rc_replaceText(const REQUEST_rc_replaceText& request, STRUCT_contextMapping& edit_contextMapping);
  static inline TYPE_json json_sourceMapInfo(const STRUCT_contextMapping& contextMapping);

  namespace /* private */
  {
    inline TYPE_rc _rc_validateMapSegment(const std::string& strA, const std::string& strB, const STRUCT_mapSegment& mapSegment);
    inline TYPE_rc _rc_validateSourceMap(const std::string& strA, const std::string& strB, const std::vector<STRUCT_mapSegment>& listMapSegment);
    inline TYPE_rc _rc_resolveRangeAtoB(const STRUCT_contextMapping& contextMapping, const STRUCT_range& rangeA, STRUCT_range& out_rangeB);
    inline TYPE_rc _rc_applyTextModification(const std::string& text, const STRUCT_range& rangeA, const STRUCT_range& rangeB, STRUCT_contextMapping& edit_contextMapping);
    inline void _vd_appendMapSegment(const STRUCT_mapSegment& newMapSegment, std::vector<STRUCT_mapSegment>& edit_listMapSegment);
    inline void _vd_appendRetainedRange(const STRUCT_contextMapping& contextMapping, const STRUCT_range& rangeB, std::string& edit_strB, std::vector<STRUCT_mapSegment>& edit_listMapSegment);
  }
}

//====----====----====----====----====----====----====----====---- FDEF

namespace sourceMap
{
  static inline TYPE_rc rc_initSourceMap(const std::string& strA, STRUCT_contextMapping& out_sourceMap)
  {
    STRUCT_contextMapping sourceMapTemp;
    sourceMapTemp.strA = strA;
    sourceMapTemp.strB = strA;

    if (!strA.empty())
    {
      sourceMapTemp.listMapSegment.push_back(
        {
          { 0, strA.size() },
          { 0, strA.size() },
          false
        }
      );
    }

    out_sourceMap = std::move(sourceMapTemp);

    // END
    return 0;
  }

  static inline TYPE_rc rc_insertText(const REQUEST_rc_insertText& request, STRUCT_contextMapping& edit_sourceMap)
  {
    // NULLIFY : rc
    TYPE_rc rci = 0;
    TYPE_rc rce = 0;

    // RCE : validate the source-map state
    rce = to_abs(_rc_validateSourceMap(edit_sourceMap.strA, edit_sourceMap.strB, edit_sourceMap.listMapSegment));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Cannot insert text into an invalid source-map");
      return rce;
    }

    // RCI : validate the insertion point
    rci--;
    if (request.startByteB > edit_sourceMap.strB.size())
    {
      vd_printErrorRc(rci, "B insertion point exceeds B text");
      return rci;
    }

    if (request.text.empty())
    {
      // END : an empty insertion leaves the source-map unchanged
      return 0;
    }

    // RCI : reject points that would split a replaced map segment
    rci--;
    for (const STRUCT_mapSegment& mapSegment : edit_sourceMap.listMapSegment)
    {
      if (mapSegment.isContentReplaced
        && mapSegment.rangeB.startByte < request.startByteB
        && request.startByteB < mapSegment.rangeB.endByte)
      {
        vd_printErrorRc(rci, "Cannot insert inside replaced B text");
        return rci;
      }
    }

    // RCE : insert unmapped text at the B position
    const STRUCT_range rangeA = { std::string::npos, std::string::npos };
    const STRUCT_range rangeB = { request.startByteB, request.startByteB };
    rce = to_abs(_rc_applyTextModification(request.text, rangeA, rangeB, edit_sourceMap));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed inserting text");
      return rce;
    }

    // END
    return 0;
  }

  static inline TYPE_rc rc_replaceText(const REQUEST_rc_replaceText& request, STRUCT_contextMapping& edit_sourceMap)
  {
    // NULLIFY : rc
    TYPE_rc rce = 0;

    // RCE : validate the source-map state
    rce = to_abs(_rc_validateSourceMap(edit_sourceMap.strA, edit_sourceMap.strB, edit_sourceMap.listMapSegment));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Cannot replace text in an invalid source-map");
      return rce;
    }

    // RCE : resolve the A range to B coordinates
    STRUCT_range rangeB = { std::string::npos, std::string::npos };
    rce = to_abs(_rc_resolveRangeAtoB(edit_sourceMap, request.rangeA, rangeB));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed resolving the A range to replace");
      return rce;
    }

    // RCE : replace the range and map new text to its A source
    rce = to_abs(_rc_applyTextModification(request.text, request.rangeA, rangeB, edit_sourceMap));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed replacing text");
      return rce;
    }

    // END
    return 0;
  }

  static inline TYPE_json json_sourceMapInfo(const STRUCT_contextMapping& sourceMap)
  {
    TYPE_json jsonSourceMapInfoTemp =
    {
      {"sizeByteA", sourceMap.strA.size()},
      {"sizeByteB", sourceMap.strB.size()},
      {"listMapSegment", TYPE_json::array()}
    };

    for (const STRUCT_mapSegment& mapSegment : sourceMap.listMapSegment)
    {
      TYPE_json jsonA = nullptr;
      if (mapSegment.rangeA.startByte != std::string::npos)
      {
        jsonA =
        {
          {"startByte", mapSegment.rangeA.startByte},
          {"endByte", mapSegment.rangeA.endByte}
        };
      }

      jsonSourceMapInfoTemp["listMapSegment"].push_back(
        {
          {"A", jsonA},
          {"B",
            {
              {"startByte", mapSegment.rangeB.startByte},
              {"endByte", mapSegment.rangeB.endByte}
            }
          },
          {"isContentReplaced", mapSegment.isContentReplaced}
        }
      );
    }

    // END : provide the source-map information
    return jsonSourceMapInfoTemp;
  }

//====----====----====----====----====----====----====----====---- FDEF

  namespace /* private */
  {
    inline TYPE_rc _rc_validateMapSegment(const std::string& strA, const std::string& strB, const STRUCT_mapSegment& mapSegment)
    {
      // NULLIFY : rc
      TYPE_rc rci = 0;

      // RCI : check the B range
      rci--;
      if (mapSegment.rangeB.startByte >= mapSegment.rangeB.endByte
        || mapSegment.rangeB.endByte > strB.size())
      {
        vd_printErrorRc(rci, "Invalid B map-segment range");
        return rci;
      }

      const bool isARangeEmpty =
      {
        mapSegment.rangeA.startByte == std::string::npos
        && mapSegment.rangeA.endByte == std::string::npos
      };
      const bool isARangeComplete =
      {
        mapSegment.rangeA.startByte != std::string::npos
        && mapSegment.rangeA.endByte != std::string::npos
      };

      // RCI : check A range endpoints
      rci--;
      if (!isARangeEmpty && !isARangeComplete)
      {
        vd_printErrorRc(rci, "A map-segment range has only one endpoint");
        return rci;
      }

      // RCI : check the A range
      rci--;
      if (isARangeComplete
        && (mapSegment.rangeA.startByte >= mapSegment.rangeA.endByte
          || mapSegment.rangeA.endByte > strA.size()))
      {
        vd_printErrorRc(rci, "Invalid A map-segment range");
        return rci;
      }

      // RCI : reject replaced content without an A range
      rci--;
      if (isARangeEmpty && mapSegment.isContentReplaced)
      {
        vd_printErrorRc(rci, "Unmapped B text cannot be marked as replaced");
        return rci;
      }

      // RCI : check unchanged range lengths
      rci--;
      if (isARangeComplete
        && !mapSegment.isContentReplaced
        && mapSegment.rangeA.endByte - mapSegment.rangeA.startByte != mapSegment.rangeB.endByte - mapSegment.rangeB.startByte)
      {
        vd_printErrorRc(rci, "Unchanged map segment has different A and B lengths");
        return rci;
      }

      // END
      return 0;
    }

    inline TYPE_rc _rc_validateSourceMap(const std::string& strA, const std::string& strB, const std::vector<STRUCT_mapSegment>& listMapSegment)
    {
      // NULLIFY : rc
      TYPE_rc rci = 0;
      TYPE_rc rce = 0;

      // RCE : validate each map segment
      for (std::size_t idxSegment = 0; idxSegment < listMapSegment.size(); idxSegment++)
      {
        const STRUCT_mapSegment& mapSegment = listMapSegment[idxSegment];
        rce = to_abs(_rc_validateMapSegment(strA, strB, mapSegment));
        if (rce != 0)
        {
          vd_printErrorRc(rce, "Invalid map segment at idx " + std::to_string(idxSegment));
          return rce;
        }
      }

      // RCI : check B segment connections
      std::size_t byteBCursor = 0;
      rci--;
      for (const STRUCT_mapSegment& mapSegment : listMapSegment)
      {
        if (mapSegment.rangeB.startByte != byteBCursor)
        {
          vd_printErrorRc(rci, "Source-map segment connection mismatch at byte " + std::to_string(byteBCursor));
          return rci;
        }

        byteBCursor = mapSegment.rangeB.endByte;
      }

      // RCI : check complete B coverage
      rci--;
      if (byteBCursor != strB.size())
      {
        vd_printErrorRc(rci, "Source-map segments do not cover the B source");
        return rci;
      }

      // END
      return 0;
    }

    inline TYPE_rc _rc_resolveRangeAtoB(const STRUCT_contextMapping& contextMapping, const STRUCT_range& rangeA, STRUCT_range& out_rangeB)
    {
      // NULLIFY : rc, out
      out_rangeB = { std::string::npos, std::string::npos };
      TYPE_rc rci = 0;

      // RCI : check the A range order
      rci--;
      if (rangeA.startByte > rangeA.endByte)
      {
        vd_printErrorRc(rci, "Requested A range is reversed");
        return rci;
      }

      // RCI : check the A range bounds
      rci--;
      if (rangeA.endByte > contextMapping.strA.size())
      {
        vd_printErrorRc(rci, "Requested A range exceeds text A");
        return rci;
      }

      // RCI : reject an empty A range
      rci--;
      if (rangeA.startByte == rangeA.endByte)
      {
        vd_printErrorRc(rci, "Replacement requires a non-empty A range");
        return rci;
      }

      // RCI : resolve the A range start to B
      rci--;
      for (const STRUCT_mapSegment& mapSegment : contextMapping.listMapSegment)
      {
        if (mapSegment.rangeA.startByte == std::string::npos
          || rangeA.startByte < mapSegment.rangeA.startByte
          || rangeA.startByte > mapSegment.rangeA.endByte)
        {
          continue;
        }

        std::size_t byteB = 0;
        if (mapSegment.isContentReplaced)
        {
          byteB = rangeA.startByte >= mapSegment.rangeA.endByte
            ? mapSegment.rangeB.endByte
            : mapSegment.rangeB.startByte;
        }
        else
        {
          byteB = mapSegment.rangeB.startByte + rangeA.startByte - mapSegment.rangeA.startByte;
        }

        if (out_rangeB.startByte == std::string::npos)
        {
          out_rangeB.startByte = byteB;
        }
        else if (out_rangeB.startByte != byteB)
        {
          out_rangeB.startByte = std::string::npos;
          vd_printErrorRc(rci, "A range start maps to multiple B positions");
          return rci;
        }
      }

      // RCI : require the A range start to resolve
      rci--;
      if (out_rangeB.startByte == std::string::npos)
      {
        vd_printErrorRc(rci, "Failed resolving A range start to B");
        return rci;
      }

      // RCI : resolve the A range end to B
      rci--;
      for (const STRUCT_mapSegment& mapSegment : contextMapping.listMapSegment)
      {
        if (mapSegment.rangeA.startByte == std::string::npos
          || rangeA.endByte < mapSegment.rangeA.startByte
          || rangeA.endByte > mapSegment.rangeA.endByte)
        {
          continue;
        }

        std::size_t byteB = 0;
        if (mapSegment.isContentReplaced)
        {
          byteB = rangeA.endByte <= mapSegment.rangeA.startByte
            ? mapSegment.rangeB.startByte
            : mapSegment.rangeB.endByte;
        }
        else
        {
          byteB = mapSegment.rangeB.startByte + rangeA.endByte - mapSegment.rangeA.startByte;
        }

        if (out_rangeB.endByte == std::string::npos)
        {
          out_rangeB.endByte = byteB;
        }
        else if (out_rangeB.endByte != byteB)
        {
          out_rangeB = { std::string::npos, std::string::npos };
          vd_printErrorRc(rci, "A range end maps to multiple B positions");
          return rci;
        }
      }

      // RCI : require the A range end to resolve
      rci--;
      if (out_rangeB.endByte == std::string::npos)
      {
        out_rangeB.startByte = std::string::npos;
        vd_printErrorRc(rci, "Failed resolving A range end to B");
        return rci;
      }

      // RCI : reject ranges removed by earlier edits
      rci--;
      if (out_rangeB.startByte >= out_rangeB.endByte)
      {
        out_rangeB = { std::string::npos, std::string::npos };
        vd_printErrorRc(rci, "A range has no B bytes");
        return rci;
      }

      // END
      return 0;
    }

    inline void _vd_appendMapSegment(const STRUCT_mapSegment& newMapSegment, std::vector<STRUCT_mapSegment>& edit_listMapSegment)
    {
      // END : skip an empty B range
      if (newMapSegment.rangeB.startByte == newMapSegment.rangeB.endByte)
      {
        return;
      }

      if (edit_listMapSegment.empty())
      {
        edit_listMapSegment.push_back(newMapSegment);
        return;
      }

      STRUCT_mapSegment& previous = edit_listMapSegment.back();
      if (previous.rangeB.endByte != newMapSegment.rangeB.startByte)
      {
        edit_listMapSegment.push_back(newMapSegment);
        return;
      }

      const bool isPreviousMapped = previous.rangeA.startByte != std::string::npos;
      const bool isCurrentMapped = newMapSegment.rangeA.startByte != std::string::npos;

      if (!isPreviousMapped && !isCurrentMapped)
      {
        previous.rangeB.endByte = newMapSegment.rangeB.endByte;
        return;
      }

      if (isPreviousMapped && isCurrentMapped
        && previous.isContentReplaced == newMapSegment.isContentReplaced)
      {
        if (previous.isContentReplaced
          && previous.rangeA.startByte == newMapSegment.rangeA.startByte
          && previous.rangeA.endByte == newMapSegment.rangeA.endByte)
        {
          previous.rangeB.endByte = newMapSegment.rangeB.endByte;
          return;
        }

        if (!previous.isContentReplaced
          && previous.rangeA.endByte == newMapSegment.rangeA.startByte)
        {
          previous.rangeB.endByte = newMapSegment.rangeB.endByte;
          previous.rangeA.endByte = newMapSegment.rangeA.endByte;
          return;
        }
      }

      edit_listMapSegment.push_back(newMapSegment);
      return;
    }

    inline void _vd_appendRetainedRange(const STRUCT_contextMapping& contextMapping, const STRUCT_range& rangeB, std::string& edit_strB, std::vector<STRUCT_mapSegment>& edit_listMapSegment)
    {
      // Append existing B text and carry its A-to-B segment links into the rebuilt result.
      // END : skip an empty B range
      if (rangeB.startByte == rangeB.endByte)
      {
        return;
      }

      const std::size_t startByteBDest = edit_strB.size();
      edit_strB.append(contextMapping.strB, rangeB.startByte, rangeB.endByte - rangeB.startByte);

      for (const STRUCT_mapSegment& mapSegment : contextMapping.listMapSegment)
      {
        if (mapSegment.rangeB.endByte <= rangeB.startByte)
        {
          continue;
        }

        if (mapSegment.rangeB.startByte >= rangeB.endByte)
        {
          break;
        }

        const std::size_t startByteOverlap = (std::max)(rangeB.startByte, mapSegment.rangeB.startByte);
        const std::size_t endByteOverlap = (std::min)(rangeB.endByte, mapSegment.rangeB.endByte);
        STRUCT_mapSegment mapSegmentSlice = mapSegment;
        mapSegmentSlice.rangeB.startByte = startByteBDest + startByteOverlap - rangeB.startByte;
        mapSegmentSlice.rangeB.endByte = startByteBDest + endByteOverlap - rangeB.startByte;

        if (mapSegmentSlice.rangeA.startByte != std::string::npos && !mapSegmentSlice.isContentReplaced)
        {
          mapSegmentSlice.rangeA.startByte = mapSegment.rangeA.startByte + startByteOverlap - mapSegment.rangeB.startByte;
          mapSegmentSlice.rangeA.endByte = mapSegment.rangeA.startByte + endByteOverlap - mapSegment.rangeB.startByte;
        }

        _vd_appendMapSegment(mapSegmentSlice, edit_listMapSegment);
      }

      // END
      return;
    }

    inline TYPE_rc _rc_applyTextModification(const std::string& text, const STRUCT_range& rangeA, const STRUCT_range& rangeB, STRUCT_contextMapping& edit_contextMapping)
    {
      // NULLIFY : rc
      TYPE_rc rci = 0;
      TYPE_rc rce = 0;

      // RCI : check the resulting B size
      const std::size_t sizeBRetained = edit_contextMapping.strB.size() - (rangeB.endByte - rangeB.startByte);
      rci--;
      if (text.size() > edit_contextMapping.strB.max_size() - sizeBRetained)
      {
        vd_printErrorRc(rci, "B source size overflows after text modification");
        return rci;
      }

      // REBUILD : prepare B text and map segments together
      std::string strBUpdated;
      std::vector<STRUCT_mapSegment> listMapSegmentUpdated;
      strBUpdated.reserve(sizeBRetained + text.size());
      listMapSegmentUpdated.reserve(edit_contextMapping.listMapSegment.size() + 2);

      // RETAIN : B text and mapping before the target
      const STRUCT_range rangeBPrefix = { 0, rangeB.startByte };
      _vd_appendRetainedRange(edit_contextMapping, rangeBPrefix, strBUpdated, listMapSegmentUpdated);

      // MODIFY : add the requested text and map it to A when it replaces A text
      if (!text.empty())
      {
        const std::size_t startByteBText = strBUpdated.size();
        strBUpdated += text;
        _vd_appendMapSegment(
          {
            rangeA,
            { startByteBText, strBUpdated.size() },
            rangeA.startByte != std::string::npos
          },
          listMapSegmentUpdated
        );
      }

      // RETAIN : B text and mapping after the target
      const STRUCT_range rangeBSuffix = { rangeB.endByte, edit_contextMapping.strB.size() };
      _vd_appendRetainedRange(edit_contextMapping, rangeBSuffix, strBUpdated, listMapSegmentUpdated);

      // RCE : validate the rebuilt state before committing it
      rce = to_abs(_rc_validateSourceMap(edit_contextMapping.strA, strBUpdated, listMapSegmentUpdated));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Text modification produced an invalid source-map state");
        return rce;
      }

      // END : commit B text and map segments together
      edit_contextMapping.strB = std::move(strBUpdated);
      edit_contextMapping.listMapSegment = std::move(listMapSegmentUpdated);
      return 0;
    }
  }
}
