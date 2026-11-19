#include "03_edge/audio/linda.asio/src/_h/01_api/_lindaAsio.h"

#include <atomic>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "00_vendored/asio_sdk/common/asiosys.h"
#include "00_vendored/asio_sdk/common/asio.h"
#include "00_vendored/asio_sdk/host/asiodrivers.h"
#include "03_edge/audio/linda.asio/src/_h/00_runtime/_lindaAsioRuntime.h"
#include "03_edge/audio/linda.asio/src/_h/01_api/narrative/_narr.openAudioRuntime.h"
#include "03_edge/audio/linda.asio/src/_h/01_api/narrative/_narr.scanChannel.h"

using namespace std;

//====----====----====----====----====----====----====----====---- FDEC

namespace lindaAsio::api
{
  namespace /* private */
  {
    void _cb_bufferSwitch(long doubleBufferIndex, ASIOBool directProcess);
    void _cb_sampleRateDidChange(ASIOSampleRate sampleRate);
    long _cb_asioMessage(long selector, long value, void* message, double* option);
    ASIOTime* _cb_bufferSwitchTimeInfo(ASIOTime* timeInfo, long doubleBufferIndex, ASIOBool directProcess);
  }
}

//====----====----====----====----====----====----====----====---- TYPE

namespace lindaAsio::api
{
  namespace /* private */
  {
    lindaAsio::runtime::HANDLE_asioRuntime* _handle_activeAudioRuntime = nullptr;
  }
}

//====----====----====----====----====----====----====----====---- FDEF

namespace lindaAsio::api
{
  TYPE_rc rc_listDevice(std::vector<lsm::audio::STRUCT_audioDeviceInfo>& out_listAudioDeviceInfo)
  {
    out_listAudioDeviceInfo.clear();
    TYPE_rc rci = 0;

    // ASIO uses process-wide driver state, so enumeration must not interrupt
    // an active audio runtime.
    rci--;
    if (_handle_activeAudioRuntime != nullptr)
    {
      vd_printErrorRc(rci, "Cannot list devices while an ASIO runtime is active");
      return rci;
    }

    AsioDrivers driverManager;
    runtime::vd_bindAsioDriverManager(&driverManager);

    const long numDriver = driverManager.asioGetNumDev();
    for (long idxDriver = 0; idxDriver < numDriver; ++idxDriver)
    {
      char nameDriver[MAXDRVNAMELEN] = {};
      if (driverManager.asioGetDriverName(static_cast<int>(idxDriver), nameDriver, static_cast<int>(sizeof(nameDriver))) != 0)
      {
        continue;
      }

      if (!driverManager.loadDriver(nameDriver))
      {
        continue;
      }

      ASIODriverInfo infoDriver = {};
      infoDriver.asioVersion = 2;
      infoDriver.sysRef = GetDesktopWindow();

      if (ASIOInit(&infoDriver) != ASE_OK)
      {
        driverManager.removeCurrentDriver();
        continue;
      }

      long numInput = 0;
      long numOutput = 0;
      if (true)
      {
        const bool isChannelCountValid =
        {
          ASIOGetChannels(&numInput, &numOutput) == ASE_OK
          && numInput >= 0
          && numOutput >= 0
        };
        if (isChannelCountValid)
        {
          out_listAudioDeviceInfo.push_back(
            {
              nameDriver,
              static_cast<uint32_t>(numInput),
              static_cast<uint32_t>(numOutput)
            }
          );
        }
      }

      ASIOExit();
    }

    runtime::vd_unbindAsioDriverManager();

    // END
    return 0;
  }

  TYPE_rc rc_scanChannel(const string& nameDevice, bool isVerbose, TYPE_json& out_jsonScanResult)
  {
    // NULLIFY : rc, out
    out_jsonScanResult = nullptr;

    // NARRATIVE [0]
    narr::scanChannel::STRUCT_context context;
    context.input.nameDevice = nameDevice;
    context.input.isVerbose = isVerbose;
    context.input.isRuntimeActive = (_handle_activeAudioRuntime != nullptr);

    TYPE_rc rce = 0;

    // NARRATIVE [1]
    if (true)
    {
      rce = to_abs(narr::scanChannel::narr_rc_openDevice(context));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed opening ASIO device");
        return rce;
      }
    }

    // NARRATIVE [2]
    if (true)
    {
      rce = to_abs(narr::scanChannel::narr_rc_initializeDevice(context));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed initializing ASIO device");
        return rce;
      }
    }

    // NARRATIVE [3]
    if (true)
    {
      rce = to_abs(narr::scanChannel::narr_rc_readCapability(context));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed reading ASIO device capability");
        return rce;
      }
    }

    // NARRATIVE [4]
    if (true)
    {
      rce = to_abs(narr::scanChannel::narr_rc_recordInputChannels(context));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed recording ASIO input channels");
        return rce;
      }
    }

    // NARRATIVE [5]
    if (true)
    {
      rce = to_abs(narr::scanChannel::narr_rc_recordOutputChannels(context));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed recording ASIO output channels");
        return rce;
      }
    }

    out_jsonScanResult = context.state.jsonScanResult;

    // END
    return 0;
  }

  TYPE_rc rc_openAudioRuntime(const TYPE_json& jsonRunner, uint32_t& out_spb, void*& out_handle_audioRuntime)
  {
    // NULLIFY : rc, out
    out_spb = 0;
    out_handle_audioRuntime = nullptr;

    // NARRATIVE [0]
    narr::openAudioRuntime::STRUCT_context context =
    {
      .input =
      {
        .jsonRunner = jsonRunner,
        .isRuntimeActive = (_handle_activeAudioRuntime != nullptr)
      }
    };

    TYPE_rc rce = 0;

    // NARRATIVE [1]
    if (true)
    {
      rce = to_abs(narr::openAudioRuntime::narr_rc_resolveConfig(context));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed resolving ASIO runtime configuration");
        return rce;
      }
    }

    // NARRATIVE [2]
    if (true)
    {
      rce = to_abs(narr::openAudioRuntime::narr_rc_initializeRuntime(context));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed initializing ASIO runtime");
        return rce;
      }
    }

    // Register the private callbacks before the buffer creation stage.
    context.state.handle_asioRuntime->asioCallbacks =
    {
      _cb_bufferSwitch,
      _cb_sampleRateDidChange,
      _cb_asioMessage,
      _cb_bufferSwitchTimeInfo
    };

    // NARRATIVE [3]
    if (true)
    {
      rce = to_abs(narr::openAudioRuntime::narr_rc_configureChannels(context));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed configuring ASIO channels");
        return rce;
      }
    }

    // NARRATIVE [4]
    if (true)
    {
      rce = to_abs(narr::openAudioRuntime::narr_rc_buildRoutes(context));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed building ASIO routes");
        return rce;
      }
    }

    // NARRATIVE [5]
    if (true)
    {
      rce = to_abs(narr::openAudioRuntime::narr_rc_createBuffers(context));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed creating ASIO buffers");
        return rce;
      }
    }

    _handle_activeAudioRuntime = context.state.handle_asioRuntime;
    out_spb = context.state.spb;
    out_handle_audioRuntime = context.state.handle_asioRuntime;

    // END
    return 0;
  }

  TYPE_rc rc_bindAudioRuntime(const lsm::audio::REQUEST_rc_bindAudioRuntime& request, void* handle_audioRuntime)
  {
    TYPE_rc rci = 0;

    rci--;
    if ((handle_audioRuntime == nullptr) || (handle_audioRuntime != _handle_activeAudioRuntime))
    {
      vd_printErrorRc(rci, "ASIO runtime handle is null or inactive");
      return rci;
    }
    rci--;
    if (_handle_activeAudioRuntime->asioState != runtime::const_asioStateBuffersCreated)
    {
      vd_printErrorRc(rci, "ASIO is not ready for binding");
      return rci;
    }

    rci--;
    if ((request.rc_processBuffer == nullptr) || (request.rc_notifySpsChange == nullptr))
    {
      vd_printErrorRc(rci, "GIG audio callbacks are not set");
      return rci;
    }

    // Validate semantic buffer linkage.
    rci--;
    if (request.spb == 0 || _handle_activeAudioRuntime->listBufferFloat.size() != _handle_activeAudioRuntime->listBufferInfo.size() * static_cast<size_t>(request.spb))
    {
      vd_printErrorRc(rci, "SPB does not match ASIO buffers");
      return rci;
    }
    rci--;
    if ((request.numSemanticIn > 0) && (request.ptrSemanticIn == nullptr))
    {
      vd_printErrorRc(rci, "Semantic-In buffer is null");
      return rci;
    }
    rci--;
    if ((request.numSemanticOut > 0) && (request.ptrSemanticOut == nullptr))
    {
      vd_printErrorRc(rci, "Semantic-Out buffer is null");
      return rci;
    }
    rci--;
    for (const vector<uint32_t>& listIdxSemantic : _handle_activeAudioRuntime->llistInputToSemanticIn)
    {
      for (uint32_t idxSemantic : listIdxSemantic)
      {
        if (idxSemantic >= request.numSemanticIn)
        {
          vd_printErrorRc(rci, "Linda-In-dest out of range (" + to_string(idxSemantic) + ")");
          return rci;
        }
      }
    }
    rci--;
    for (const vector<uint32_t>& listIdxSemantic : _handle_activeAudioRuntime->llistOutputFromSemanticOut)
    {
      for (uint32_t idxSemantic : listIdxSemantic)
      {
        if (idxSemantic >= request.numSemanticOut)
        {
          vd_printErrorRc(rci, "Linda-Out-src out of range (" + to_string(idxSemantic) + ")");
          return rci;
        }
      }
    }

    // Store the validated semantic buffer linkage.
    _handle_activeAudioRuntime->binding = request;

    // END
    return 0;
  }

  TYPE_rc rc_setSps(void* handle_audioRuntime, uint32_t newSps, uint32_t& out_sps)
  {
    // Initialize the resolved SPS output.
    out_sps = 0;

    // Validate the active runtime.
    TYPE_rc rci = 0;
    rci--;
    if ((handle_audioRuntime == nullptr) || (handle_audioRuntime != _handle_activeAudioRuntime))
    {
      vd_printErrorRc(rci, "ASIO runtime handle is null or inactive");
      return rci;
    }

    // Validate the requested SPS.
    rci--;
    if (newSps == 0)
    {
      vd_printErrorRc(rci, "SPS is zero");
      return rci;
    }

    // Apply the requested SPS to the driver.
    rci--;
    const ASIOError asioErrorSetSampleRate = ASIOSetSampleRate(static_cast<ASIOSampleRate>(newSps));
    if (asioErrorSetSampleRate != ASE_OK)
    {
      vd_printErrorVendor("ASIO", "SETSAMPLERATE failed (" + to_string(static_cast<int32_t>(asioErrorSetSampleRate)) + ")");
      vd_printErrorRc(rci, "ASIO SETSAMPLERATE failed");
      return rci;
    }

    // Read the SPS accepted by the driver.
    ASIOSampleRate temp_sps = 0;
    rci--;
    const ASIOError asioErrorGetSampleRate = ASIOGetSampleRate(&temp_sps);
    if (asioErrorGetSampleRate != ASE_OK || temp_sps <= 0)
    {
      if (asioErrorGetSampleRate != ASE_OK)
      {
        vd_printErrorVendor("ASIO", "GETSAMPLERATE failed (" + to_string(static_cast<int32_t>(asioErrorGetSampleRate)) + ")");
      }
      vd_printErrorRc(rci, "ASIO GETSAMPLERATE failed");
      return rci;
    }

    // Store and return the accepted SPS.
    out_sps = static_cast<uint32_t>(temp_sps);
    _handle_activeAudioRuntime->sps.store(out_sps, memory_order_release);

    // END
    return 0;
  }

  TYPE_rc rc_startAudioRuntime(void* handle_audioRuntime)
  {
    TYPE_rc rci = 0;
    rci--;
    if (handle_audioRuntime == nullptr || handle_audioRuntime != _handle_activeAudioRuntime)
    {
      vd_printErrorRc(rci, "ASIO runtime handle is null or inactive");
      return rci;
    }
    
    // Validate the runtime state required for streaming.
    rci--;
    if (_handle_activeAudioRuntime->asioState != runtime::const_asioStateBuffersCreated || _handle_activeAudioRuntime->binding.rc_processBuffer == nullptr)
    {
      vd_printErrorRc(rci, "ASIO is not in correct state");
      return rci;
    }

    // Start the ASIO stream.
    rci--;
    const ASIOError asioErrorStart = ASIOStart();
    if (asioErrorStart != ASE_OK)
    {
      vd_printErrorVendor("ASIO", "START failed (" + to_string(static_cast<int32_t>(asioErrorStart)) + ")");
      vd_printErrorRc(rci, "ASIO START failed");
      return rci;
    }

    cout << "ASIO stream started" << endl;
    _handle_activeAudioRuntime->asioState = runtime::const_asioStateRunning;

    // END
    return 0;
  }

  TYPE_rc rc_stopAudioRuntime(void* handle_audioRuntime)
  {
    TYPE_rc rci = 0;
    rci--;
    if ((handle_audioRuntime == nullptr) || (handle_audioRuntime != _handle_activeAudioRuntime))
    {
      vd_printErrorRc(rci, "ASIO runtime handle is null or inactive");
      return rci;
    }

    rci--;
    if (_handle_activeAudioRuntime->asioState != runtime::const_asioStateRunning)
    {
      vd_printErrorRc(rci, "Audio runtime is not running");
      return rci;
    }

    // Stop the ASIO stream.
    rci--;
    const ASIOError asioErrorStop = ASIOStop();
    if (asioErrorStop != ASE_OK)
    {
      vd_printErrorVendor("ASIO", "STOP failed (" + to_string(static_cast<int32_t>(asioErrorStop)) + ")");
      vd_printErrorRc(rci, "ASIO STOP failed");
      return rci;
    }

    _handle_activeAudioRuntime->asioState = runtime::const_asioStateBuffersCreated;

    // END
    return 0;
  }

  void vd_closeAudioRuntime(void*& handle_audioRuntime)
  {
    if (handle_audioRuntime == nullptr)
    {
      return;
    }

    runtime::HANDLE_asioRuntime* handle_asioRuntime = static_cast<runtime::HANDLE_asioRuntime*>(handle_audioRuntime);

    if (handle_audioRuntime == _handle_activeAudioRuntime)
    {
      _handle_activeAudioRuntime = nullptr;
    }

    runtime::vd_releaseAsioRuntime(handle_asioRuntime);
    handle_audioRuntime = nullptr;

    // END
    return;
  }

//====----====----====----====----====----====----====----====---- FDEF

  namespace /* private */
  {
    void _cb_bufferSwitch(long doubleBufferIndex, ASIOBool directProcess)
    {
      if (_handle_activeAudioRuntime == nullptr)
      {
        return;
      }

      const lsm::audio::REQUEST_rc_bindAudioRuntime& binding = _handle_activeAudioRuntime->binding;
      if (binding.rc_processBuffer == nullptr)
      {
        return;
      }

      if (binding.numSemanticIn > 0)
      {
        std::memset(binding.ptrSemanticIn, 0, sizeof(float) * binding.numSemanticIn * binding.spb);
      }

      for (uint32_t i : _handle_activeAudioRuntime->listIdxBufferInput)
      {
        const void* src = _handle_activeAudioRuntime->listBufferInfo[i].buffers[doubleBufferIndex];
        float* bufferFloat = _handle_activeAudioRuntime->listBufferFloat.data() + static_cast<size_t>(i) * binding.spb;
        _handle_activeAudioRuntime->listReadBuffer[i](src, bufferFloat, binding.spb);

        for (uint32_t idxSemantic : _handle_activeAudioRuntime->llistInputToSemanticIn[i])
        {
          float* dest = binding.ptrSemanticIn + static_cast<size_t>(idxSemantic) * binding.spb;
          for (uint32_t j = 0; j < binding.spb; j++)
          {
            dest[j] += bufferFloat[j];
          }
        }
      }

      TYPE_rc rcProcess = 0;
      const uint32_t pendingSps = _handle_activeAudioRuntime->pendingSps.exchange(0, std::memory_order_acq_rel);
      if (pendingSps > 0)
      {
        rcProcess = binding.rc_notifySpsChange(pendingSps);
      }
      if (rcProcess == 0)
      {
        rcProcess = binding.rc_processBuffer();
      }
      if (rcProcess != 0 && binding.numSemanticOut > 0)
      {
        std::memset(binding.ptrSemanticOut, 0, sizeof(float) * binding.numSemanticOut * binding.spb);
      }

      for (uint32_t i : _handle_activeAudioRuntime->listIdxBufferOutput)
      {
        float* bufferFloat = _handle_activeAudioRuntime->listBufferFloat.data() + static_cast<size_t>(i) * binding.spb;
        std::memset(bufferFloat, 0, sizeof(float) * binding.spb);

        for (uint32_t idxSemantic : _handle_activeAudioRuntime->llistOutputFromSemanticOut[i])
        {
          const float* src = binding.ptrSemanticOut + static_cast<size_t>(idxSemantic) * binding.spb;
          for (uint32_t j = 0; j < binding.spb; j++)
          {
            bufferFloat[j] += src[j];
          }
        }

        void* dest = _handle_activeAudioRuntime->listBufferInfo[i].buffers[doubleBufferIndex];
        _handle_activeAudioRuntime->listWriteBuffer[i](bufferFloat, dest, binding.spb);
      }
    }

    void _cb_sampleRateDidChange(ASIOSampleRate sampleRate)
    {
      if (_handle_activeAudioRuntime == nullptr || sampleRate <= 0)
      {
        return;
      }

      const uint32_t sps = static_cast<uint32_t>(sampleRate);
      _handle_activeAudioRuntime->sps.store(sps, std::memory_order_release);
      _handle_activeAudioRuntime->pendingSps.store(sps, std::memory_order_release);
    }

    long _cb_asioMessage(long selector, long value, void*, double*)
    {
      switch (selector)
      {
        case kAsioSelectorSupported:
          return value == kAsioEngineVersion ? 1 : 0;

        case kAsioEngineVersion:
          return 2;

        default:
          return 0;
      }
    }

    ASIOTime* _cb_bufferSwitchTimeInfo(ASIOTime* timeInfo, long doubleBufferIndex, ASIOBool directProcess)
    {
      _cb_bufferSwitch(doubleBufferIndex, directProcess);
      return timeInfo;
    }
  }
}
