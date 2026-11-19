#include "03_edge/audio/linda.asio/src/_h/01_api/narrative/_narr.scanChannel.h"

#include <cstdint>
#include <vector>

#include "00_vendored/asio_sdk/common/asiosys.h"
#include "00_vendored/asio_sdk/common/asio.h"
#include "00_vendored/asio_sdk/host/asiodrivers.h"

using namespace std;

//====----====----====----====----====----====----====----====---- FDEC

namespace lindaAsio::narr::scanChannel
{
  namespace /* private */
  {
    void _narr_vd_releaseScan(lindaAsio::narr::scanChannel::STRUCT_context& edit_context);
  }
}

//====----====----====----====----====----====----====----====---- FDEF

namespace lindaAsio::narr::scanChannel
{
  // [1]
  TYPE_rc narr_rc_openDevice(STRUCT_context& edit_context)
  {
    TYPE_rc rci = 0;

    rci--;
    if (edit_context.input.isRuntimeActive)
    {
      vd_printErrorRc(rci, "Cannot scan channels while an ASIO runtime is active");
      return rci;
    }

    lindaAsio::runtime::vd_bindAsioDriverManager(&edit_context.state.driverManager);
    vector<char> listCharNameDevice(edit_context.input.nameDevice.begin(), edit_context.input.nameDevice.end());
    listCharNameDevice.push_back('\0');
    rci--;
    if (!edit_context.state.driverManager.loadDriver(listCharNameDevice.data()))
    {
      vd_printErrorRc(rci, "Failed loading ASIO driver: " + edit_context.input.nameDevice);
      lindaAsio::runtime::vd_unbindAsioDriverManager();
      return rci;
    }
    edit_context.state.isDriverLoaded = true;

    // END
    return 0;
  }

  // [2]
  TYPE_rc narr_rc_initializeDevice(STRUCT_context& edit_context)
  {
    TYPE_rc rci = 0;
    ASIODriverInfo infoDriver = {};
    infoDriver.asioVersion = 2;
    infoDriver.sysRef = GetDesktopWindow();
    const ASIOError asioErrorInit = ASIOInit(&infoDriver);
    rci--;
    if (asioErrorInit != ASE_OK)
    {
      const string messageErrorVendor =
        "INIT failed for "
        + edit_context.input.nameDevice
        + " ("
        + to_string(static_cast<int32_t>(asioErrorInit))
        + "): "
        + string(infoDriver.errorMessage);
      vd_printErrorVendor("ASIO", messageErrorVendor);
      vd_printErrorRc(rci, "Failed initializing ASIO driver: " + edit_context.input.nameDevice);
      _narr_vd_releaseScan(edit_context);
      return rci;
    }
    edit_context.state.isDriverInitialized = true;

    const ASIOError asioErrorGetChannels = ASIOGetChannels(&edit_context.state.numInput, &edit_context.state.numOutput);
    rci--;
    if (asioErrorGetChannels != ASE_OK || edit_context.state.numInput < 0 || edit_context.state.numOutput < 0)
    {
      if (asioErrorGetChannels != ASE_OK)
      {
        vd_printErrorVendor("ASIO", "GETCHANNELS failed (" + to_string(static_cast<int32_t>(asioErrorGetChannels)) + ")");
      }
      vd_printErrorRc(rci, "Failed reading ASIO channel counts");
      _narr_vd_releaseScan(edit_context);
      return rci;
    }

    edit_context.state.jsonScanResult =
    {
      {"support", "asio"},
      {"device", edit_context.input.nameDevice},
      {"numInput", static_cast<uint32_t>(edit_context.state.numInput)},
      {"numOutput", static_cast<uint32_t>(edit_context.state.numOutput)},
      {"listInput", TYPE_json::array()},
      {"listOutput", TYPE_json::array()}
    };
    edit_context.state.jsonCapability = {};

    // END
    return 0;
  }

  // [3]
  TYPE_rc narr_rc_readCapability(STRUCT_context& edit_context)
  {
    if (!edit_context.input.isVerbose)
    {
      return 0;
    }

    TYPE_rc rci = 0;
    runtime::STRUCT_spbCapability spbCapability = {};
    const ASIOError asioErrorGetBufferSize = ASIOGetBufferSize(&spbCapability.min, &spbCapability.max, &spbCapability.preferred, &spbCapability.granularity);
    rci--;
    if (asioErrorGetBufferSize != ASE_OK || !runtime::is_spbCapabilityValid(spbCapability))
    {
      if (asioErrorGetBufferSize != ASE_OK)
      {
        vd_printErrorVendor("ASIO", "GETBUFFERSIZE failed (" + to_string(static_cast<int32_t>(asioErrorGetBufferSize)) + ")");
      }
      vd_printErrorRc(rci, "Failed reading ASIO SPB capability");
      edit_context.state.jsonScanResult = {};
      _narr_vd_releaseScan(edit_context);
      return rci;
    }

    TYPE_json listRuleSpb = TYPE_json::array();
    if (spbCapability.min == spbCapability.max)
    {
      listRuleSpb.push_back(
        {
          {"idx", listRuleSpb.size()},
          {"type", "list"},
          {"listValSpb", {static_cast<uint32_t>(spbCapability.min)}}
        }
      );
    }
    else if (spbCapability.granularity == -1)
    {
      listRuleSpb.push_back(
        {
          {"idx", listRuleSpb.size()},
          {"type", "geometric"},
          {"min", static_cast<uint32_t>(spbCapability.min)},
          {"max", static_cast<uint32_t>(spbCapability.max)},
          {"factor", 2}
        }
      );
    }
    else
    {
      listRuleSpb.push_back(
        {
          {"idx", listRuleSpb.size()},
          {"type", "linear"},
          {"min", static_cast<uint32_t>(spbCapability.min)},
          {"max", static_cast<uint32_t>(spbCapability.max)},
          {"step", static_cast<uint32_t>(spbCapability.granularity)}
        }
      );
    }

    rci--;
    const ASIOError asioErrorGetSampleRate = ASIOGetSampleRate(&edit_context.state.currentSps);
    if (asioErrorGetSampleRate != ASE_OK || edit_context.state.currentSps <= 0)
    {
      if (asioErrorGetSampleRate != ASE_OK)
      {
        vd_printErrorVendor("ASIO", "GETSAMPLERATE failed (" + to_string(static_cast<int32_t>(asioErrorGetSampleRate)) + ")");
      }
      vd_printErrorRc(rci, "Failed reading the current ASIO SPS");
      edit_context.state.jsonScanResult = {};
      _narr_vd_releaseScan(edit_context);
      return rci;
    }

    const uint32_t arrCandidateSps[3] = { 44100, 48000, 96000 };
    vector<uint32_t> listSupportedSps;
    for (uint32_t sps : arrCandidateSps)
    {
      if (ASIOCanSampleRate(static_cast<ASIOSampleRate>(sps)) == ASE_OK)
      {
        listSupportedSps.push_back(sps);
      }
    }

    TYPE_json listRuleSps = TYPE_json::array();
    listRuleSps.push_back(
      {
        {"idx", listRuleSps.size()},
        {"type", "list"},
        {"listValSps", listSupportedSps}
      }
    );

    edit_context.state.jsonCapability =
    {
      {
        "spb",
        {
          {"preferred", static_cast<uint32_t>(spbCapability.preferred)},
          {"listRule", listRuleSpb}
        }
      },
      {
        "sps",
        {
          {"current", static_cast<uint32_t>(edit_context.state.currentSps)},
          {"listRule", listRuleSps}
        }
      }
    };

    // END
    return 0;
  }

  // [4]
  TYPE_rc narr_rc_recordInputChannels(STRUCT_context& edit_context)
  {
    TYPE_rc rci = 0;
    rci--;
    for (long idx = 0; idx < edit_context.state.numInput; ++idx)
    {
      ASIOChannelInfo infoChannel = {};
      infoChannel.channel = idx;
      infoChannel.isInput = ASIOTrue;
      const ASIOError asioErrorGetChannelInfo = ASIOGetChannelInfo(&infoChannel);
      if (asioErrorGetChannelInfo != ASE_OK)
      {
        vd_printErrorVendor("ASIO", "GETCHANNELINFO failed (" + to_string(static_cast<int32_t>(asioErrorGetChannelInfo)) + ")");
        vd_printErrorRc(rci, "Failed reading ASIO input channel " + to_string(idx + 1));
        edit_context.state.jsonScanResult = {};
        _narr_vd_releaseScan(edit_context);
        return rci;
      }

      edit_context.state.jsonScanResult["listInput"].push_back(
        {
          {"id", static_cast<uint32_t>(idx) + 1},
          {"name", infoChannel.name}
        }
      );
    }

    // END
    return 0;
  }

  // [5]
  TYPE_rc narr_rc_recordOutputChannels(STRUCT_context& edit_context)
  {
    TYPE_rc rci = 0;
    rci--;
    for (long idx = 0; idx < edit_context.state.numOutput; ++idx)
    {
      ASIOChannelInfo infoChannel = {};
      infoChannel.channel = idx;
      infoChannel.isInput = ASIOFalse;
      const ASIOError asioErrorGetChannelInfo = ASIOGetChannelInfo(&infoChannel);
      if (asioErrorGetChannelInfo != ASE_OK)
      {
        vd_printErrorVendor("ASIO", "GETCHANNELINFO failed (" + to_string(static_cast<int32_t>(asioErrorGetChannelInfo)) + ")");
        vd_printErrorRc(rci, "Failed reading ASIO output channel " + to_string(idx + 1));
        edit_context.state.jsonScanResult = {};
        _narr_vd_releaseScan(edit_context);
        return rci;
      }

      edit_context.state.jsonScanResult["listOutput"].push_back(
        {
          {"id", static_cast<uint32_t>(idx) + 1},
          {"name", infoChannel.name}
        }
      );
    }

    if (edit_context.input.isVerbose)
    {
      edit_context.state.jsonScanResult["capability"] = edit_context.state.jsonCapability;
    }
    _narr_vd_releaseScan(edit_context);

    // END
    return 0;
  }

//====----====----====----====----====----====----====----====---- FDEF

  namespace /* private */
  {
    void _narr_vd_releaseScan(lindaAsio::narr::scanChannel::STRUCT_context& edit_context)
    {
      if (edit_context.state.isDriverInitialized)
      {
        ASIOExit();
        edit_context.state.isDriverInitialized = false;
      }
      if (edit_context.state.isDriverLoaded)
      {
        edit_context.state.driverManager.removeCurrentDriver();
        edit_context.state.isDriverLoaded = false;
      }
      lindaAsio::runtime::vd_unbindAsioDriverManager();

      // END
      return;
    }
  }
}
