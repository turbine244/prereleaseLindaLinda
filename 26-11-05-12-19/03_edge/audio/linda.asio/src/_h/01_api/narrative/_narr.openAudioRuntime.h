#pragma once

/*
  Narrative functions : lindaAsio.openAudioRuntime

  [~] START
  [1] Resolve the audio runtime configuration
    [1-A] Reject an already active runtime
    [1-B] Validate the runner and collect driver, SPB, and channel requests
  [2] Initialize the ASIO driver and resolve its fixed runtime values
    [2-A] Allocate the runtime handle and load the driver
    [2-B] Initialize the driver and read SPS, SPB, and channel counts
    [2-C] Remove unavailable requested channels
  [3] Build selected physical channel buffers and PCM callbacks
  [4] Build semantic input/output channel routes
  [5] Create ASIO buffers and initialize output silence
  [~] END
*/

#include <cstdint>
#include <set>
#include <string>

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/json.hpp"
#include "03_edge/audio/linda.asio/src/_h/00_runtime/_lindaAsioRuntime.h"

//====----====----====----====----====----====----====----====---- TYPE

namespace lindaAsio::narr::openAudioRuntime
{
  struct STRUCT_context
  {
    struct STRUCT_input
    {
      const TYPE_json& jsonRunner;
      bool isRuntimeActive = false;
    } input;

    struct STRUCT_state
    {
      runtime::HANDLE_asioRuntime* handle_asioRuntime = nullptr;
      std::string nameDriver = "";
      bool usePreferredSpb = false;
      uint32_t spb = 0;
      std::set<uint32_t> setIdInputChannel;
      std::set<uint32_t> setIdOutputChannel;
    } state;
  };
}

//====----====----====----====----====----====----====----====---- FDEC

namespace lindaAsio::narr::openAudioRuntime
{
  // [1]
  TYPE_rc narr_rc_resolveConfig(STRUCT_context& edit_context);

  // [2]
  TYPE_rc narr_rc_initializeRuntime(STRUCT_context& edit_context);

  // [3]
  TYPE_rc narr_rc_configureChannels(STRUCT_context& edit_context);

  // [4]
  TYPE_rc narr_rc_buildRoutes(STRUCT_context& edit_context);

  // [5]
  TYPE_rc narr_rc_createBuffers(STRUCT_context& edit_context);
}
