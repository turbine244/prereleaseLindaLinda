#pragma once

/*
  Narrative functions : linda.command.run

  [~] START
  [1] Prepare and validate the runner and its RIG package
    [1-A] Load and resolve the runner
    [1-B] Validate the RIG manifest and semantic contract
    [1-C] Filter unavailable wire links
  [2] Run the GIG runtime and collect its process outcome
    [2-A] Create and start the GIG runtime from the validated runner
    [2-B] Monitor process completion and the user's stop request
    [2-C] Stop the GIG runtime and record process and stop diagnostics
  [3] Persist the run result and optional runner backup
  [~] END
*/

#include <filesystem>

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/json.hpp"
#include "03_edge/launcher/linda0/src/_h/02_shell/00_cli/_cli.h"

//====----====----====----====----====----====----====----====---- PUBLIC : TYPE

namespace linda0::command::run::narr::commandRun
{
  struct STRUCT_context
  {
    struct STRUCT_input
    {
      const linda0::cli::STRUCT_lineParsed& line;
    } input;

    struct STRUCT_state
    {
      std::filesystem::path pathFileRunner;
      bool isBackupEnabled = false;
      bool isDebugEnabled = false;
      TYPE_json jsonRunner = {};
      TYPE_json jsonRunnerRecorded = {};
      TYPE_rc rceRun = 0;
      TYPE_rc rceDiagnostic = 0;
    } state;
  };
}

//====----====----====----====----====----====----====----====---- PUBLIC : FDEC

namespace linda0::command::run::narr::commandRun
{
  // [1]
  TYPE_rc narr_rc_prepareRun(STRUCT_context& edit_context);

  // [2]
  TYPE_rc narr_rc_executeRun(STRUCT_context& edit_context);

  // [3]
  TYPE_rc narr_rc_recordRunResult(STRUCT_context& edit_context);
}
