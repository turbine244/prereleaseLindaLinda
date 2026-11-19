#pragma once

/*
  linda.asio runtime utilities
*/

#include <atomic>
#include <cstdint>
#include <vector>

#include "00_vendored/asio_sdk/common/asiosys.h"
#include "00_vendored/asio_sdk/common/asio.h"
#include "00_vendored/asio_sdk/host/asiodrivers.h"
#include "02_shared/_utility/pcm2Float4.hpp"
#include "02_shared/lsm/01_audio/__lsmAudio.h"

//====----====----====----====----====----====----====----====---- TYPE

namespace lindaAsio::runtime
{
  inline const int32_t const_asioStateDriverUnloaded = 0;
  inline const int32_t const_asioStateDriverLoaded = 1;
  inline const int32_t const_asioStateInitialized = 2;
  inline const int32_t const_asioStateBuffersCreated = 3;
  inline const int32_t const_asioStateRunning = 4;

  struct HANDLE_asioRuntime
  {
    AsioDrivers driverManager;

    std::vector<ASIOBufferInfo> listBufferInfo;
    std::vector<uint32_t> listIdxBufferInput;
    std::vector<uint32_t> listIdxBufferOutput;

    std::vector<pcm2Float4::CALLBACK_vd_readPcmBuffer> listReadBuffer;
    std::vector<pcm2Float4::CALLBACK_vd_writePcmBuffer> listWriteBuffer;

    std::vector<std::vector<uint32_t>> llistInputToSemanticIn;
    std::vector<std::vector<uint32_t>> llistOutputFromSemanticOut;

    std::vector<float> listBufferFloat;
    ASIOCallbacks asioCallbacks = {};

    lsm::audio::REQUEST_rc_bindAudioRuntime binding = {};

    std::atomic<uint32_t> sps = 0;
    std::atomic<uint32_t> pendingSps = 0;
    int32_t asioState = const_asioStateDriverUnloaded;
  };

  struct STRUCT_spbCapability
  {
    long min = 0;
    long max = 0;
    long preferred = 0;
    long granularity = 0;
  };
}

//====----====----====----====----====----====----====----====---- FDEC

namespace lindaAsio::runtime
{
  void vd_bindAsioDriverManager(AsioDrivers* driverManager);
  void vd_unbindAsioDriverManager();
  void vd_releaseAsioRuntime(HANDLE_asioRuntime*& handle_asioRuntime);
  bool is_spbCapabilityValid(const STRUCT_spbCapability& capability);
  bool is_spbSupported(const STRUCT_spbCapability& capability, long spb);
}
