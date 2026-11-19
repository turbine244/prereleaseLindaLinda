#pragma once

/*
  Narrative functions : lindaAsio.scanChannel

  [~] START
  [1] Open the requested ASIO device
  [2] Initialize the device and record its channel counts
  [3] Read optional device capabilities
    [3-A] Read SPB rules
    [3-B] Read SPS rules
  [4] Record physical input channels
  [5] Record physical output channels and release the device
  [~] END
*/

#include <string>

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/json.hpp"
#include "03_edge/audio/linda.asio/src/_h/00_runtime/_lindaAsioRuntime.h"

//====----====----====----====----====----====----====----====---- TYPE

namespace lindaAsio::narr::scanChannel
{
  struct STRUCT_context
  {
    struct STRUCT_input
    {
      std::string nameDevice = "";
      bool isVerbose = false;
      bool isRuntimeActive = false;
    } input;

    struct STRUCT_state
    {
      AsioDrivers driverManager;
      bool isDriverLoaded = false;
      bool isDriverInitialized = false;

      long numInput = 0;
      long numOutput = 0;
      ASIOSampleRate currentSps = 0;
      TYPE_json jsonCapability = {};
      TYPE_json jsonScanResult = {};
    } state;
  };
}

//====----====----====----====----====----====----====----====---- FDEC

namespace lindaAsio::narr::scanChannel
{
  // [1]
  TYPE_rc narr_rc_openDevice(STRUCT_context& edit_context);

  // [2]
  TYPE_rc narr_rc_initializeDevice(STRUCT_context& edit_context);

  // [3]
  TYPE_rc narr_rc_readCapability(STRUCT_context& edit_context);

  // [4]
  TYPE_rc narr_rc_recordInputChannels(STRUCT_context& edit_context);

  // [5]
  TYPE_rc narr_rc_recordOutputChannels(STRUCT_context& edit_context);
}
