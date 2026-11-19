#pragma once

/*
  Narrative functions : lindaJs.buildRig

  [-1] Read source and scan verbose semantic information
  [~] START
  [1] Generate payload and its source map
    [1-A] Initialize A and B with the source
    [1-B] Replace runtime calls in B
    [1-C] Remove IN and OUT declarations from B
    [1-D] Replace the fx declaration with the runtime process wrapper
    [1-E] Record the A-to-B source map
  [2] Validate generated JavaScript syntax
  [3] Write the RIG package files
  [~] END
*/

#include <string>

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/json.hpp"
#include "02_shared/_utility/sourceMap.hpp"
#include "02_shared/lsm/02_language/__lsmLanguage.h"

//====----====----====----====----====----====----====----====---- TYPE

namespace lindaJs::narr::buildRig
{
  struct STRUCT_context
  {
    struct STRUCT_input
    {
      const lsm::language::REQUEST_rc_buildRig& requestBuildRig;
      const std::string& strSource;
      const TYPE_json& jsonSemanticVerbose;
    } input;

    struct STRUCT_state
    {
      sourceMap::STRUCT_contextMapping contextMapping = {};
      TYPE_json jsonSourceMap = {};
    } state;
  };
}

//====----====----====----====----====----====----====----====---- FDEC

namespace lindaJs::narr::buildRig
{
  TYPE_rc narr_rc_generateFeatures(STRUCT_context& context);
  TYPE_rc narr_rc_validateSyntax(const STRUCT_context& context);
  TYPE_rc narr_rc_writeRigPackage(const STRUCT_context& context);
}
