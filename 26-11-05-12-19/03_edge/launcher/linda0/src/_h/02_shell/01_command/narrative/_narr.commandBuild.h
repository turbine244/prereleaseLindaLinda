#pragma once

/*
  Narrative functions : linda.command.build

  [~] START
  [1] Prepare the build request and package state
    [1-A] Resolve source, package, and runner paths
    [1-B] Recover and clean interrupted package state
    [1-C] Preserve the existing runner wire configuration
  [2] Build the RIG artifact through the selected language support
  [3] Write the runner into the staging package
    [3-A] Write runner paths and wire configuration
    [3-B] Preserve the runner backup
  [4] Publish the RIG package and runner
  [~] END
*/

#include <filesystem>
#include <string>

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/json.hpp"
#include "03_edge/launcher/linda0/src/_h/01_runtime/01_languageSupport/_languageSupport.h"
#include "03_edge/launcher/linda0/src/_h/02_shell/00_cli/_cli.h"

//====----====----====----====----====----====----====----====---- PUBLIC : TYPE

namespace linda0::command::build::narr::commandBuild
{
  struct STRUCT_context
  {
    struct STRUCT_input
    {
      const linda0::cli::STRUCT_lineParsed& line;
    } input;

    struct STRUCT_state
    {
      std::filesystem::path pathFileSource;
      std::string nameLanguageSupport;
      std::filesystem::path pathDirPackage;
      std::filesystem::path pathFileRunner;
      std::filesystem::path pathDirPackageComplete;
      std::filesystem::path pathFileSourceAbsolute;
      std::filesystem::path pathFileRunnerAbsolute;
      std::filesystem::path pathDirPackageStaging;
      std::filesystem::path pathDirPackagePrevious;
      std::filesystem::path pathFileBuildError;

      bool isAbsolute = false;
      TYPE_json jsonWire = {};
      bool hasWire = false;
      linda0::languageSupport::INTF_languageSupport supportLanguage = {};
      TYPE_json jsonRunner = {};
      bool runnerInsidePackage = false;
      std::filesystem::path pathFileRunnerStaging;
      bool hadPreviousPackage = false;
    } state;
  };
}

//====----====----====----====----====----====----====----====---- PUBLIC : FDEC

namespace linda0::command::build::narr::commandBuild
{
  // [1]
  TYPE_rc narr_rc_prepareBuild(STRUCT_context& edit_context);

  // [2]
  TYPE_rc narr_rc_buildRigByLanguageSupport(STRUCT_context& edit_context);

  // [3]
  TYPE_rc narr_rc_stageRunner(STRUCT_context& edit_context);

  // [4]
  TYPE_rc narr_rc_publishRigPackage(STRUCT_context& edit_context);
}
