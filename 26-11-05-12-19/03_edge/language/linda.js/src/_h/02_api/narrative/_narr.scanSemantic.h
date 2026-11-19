#pragma once

/*
  Narrative functions : lindaJs.scanSemantic

  [~] START
  [1] Query source syntax for semantic matches
  [2] Refine matches into semantic information
    [2-A] Initialize semantic information
    [2-B] Record the fx declaration
    [2-C] Record IN and OUT declarations
    [2-D] Record runtime calls
    [2-E] Record reserved runtime identifiers
    [2-F] Build output for the requested verbosity
  [~] END
*/

#include <string>

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/json.hpp"

//====----====----====----====----====----====----====----====---- TYPE

namespace lindaJs::narr::scanSemantic
{
  struct STRUCT_context
  {
    struct STRUCT_input
    {
      const std::string& strSource;
      bool isVerbose = false;
    } input;

    struct STRUCT_state
    {
      TYPE_json jsonMatchInfo = {};
      TYPE_json jsonSemanticInfo = {};
    } state;
  };
}

//====----====----====----====----====----====----====----====---- FDEC

namespace lindaJs::narr::scanSemantic
{
  TYPE_rc narr_rc_getJsonMatchInfo(STRUCT_context& context);
  void narr_vd_getJsonSemanticInfo(STRUCT_context& context);
}
