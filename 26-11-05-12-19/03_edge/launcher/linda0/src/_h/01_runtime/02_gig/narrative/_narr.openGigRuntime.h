#pragma once

/*
  Narrative functions : linda0.gig.openGigRuntime

  [~] START
  [1] Validate the runner and loaded support interfaces
  [2] Initialize the GIG runtime handle
  [3] Open the audio runtime
  [4] Open the language runtime
  [5] Allocate semantic buffers
  [6] Bind the language runtime
  [7] Bind the audio runtime
  [8] Apply the requested SPS
  [~] END
*/

#include "03_edge/launcher/linda0/src/_h/01_runtime/02_gig/_gig.h"

//====----====----====----====----====----====----====----====---- TYPE

namespace linda0::gig::narr::openGigRuntime
{
  struct STRUCT_context
  {
    struct STRUCT_input
    {
      const TYPE_json& jsonRunner;
      const audioSupport::INTF_audioSupport& audioSupport;
      const languageSupport::INTF_languageSupport& languageSupport;
      bool isRuntimeActive = false;
      lsm::audio::CALLBACK_rc_processBuffer callbackProcessBuffer = nullptr;
      lsm::audio::CALLBACK_rc_notifySpsChange callbackNotifySpsChange = nullptr;
    } input;

    struct STRUCT_state
    {
      HANDLE_gigRuntime* handle_gigRuntime = nullptr;
      uint32_t requestedSps = 0;
    } state = {};
  };
}

//====----====----====----====----====----====----====----====---- FDEC

namespace linda0::gig::narr::openGigRuntime
{
  // [1]
  TYPE_rc narr_rc_validateInputs(STRUCT_context& edit_context);

  // [2]
  TYPE_rc narr_rc_initializeRuntime(STRUCT_context& edit_context);

  // [3]
  TYPE_rc narr_rc_openAudioRuntime(STRUCT_context& edit_context);

  // [4]
  TYPE_rc narr_rc_openLanguageRuntime(STRUCT_context& edit_context);

  // [5]
  TYPE_rc narr_rc_allocateSemanticBuffers(STRUCT_context& edit_context);

  // [6]
  TYPE_rc narr_rc_bindLanguageRuntime(STRUCT_context& edit_context);

  // [7]
  TYPE_rc narr_rc_bindAudioRuntime(STRUCT_context& edit_context);

  // [8]
  TYPE_rc narr_rc_setSps(STRUCT_context& edit_context);
}
