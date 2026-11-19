#include "03_edge/audio/linda.asio/src/_h/00_runtime/_lindaAsioRuntime.h"

#include <string>

#include "01_standard/phase00/error.hpp"

//====----====----====----====----====----====----====----====---- TYPE

// from : 00_vendored/asio_sdk/host/asiodrivers.cpp
extern AsioDrivers* asioDrivers;

//====----====----====----====----====----====----====----====---- FDEF

namespace lindaAsio::runtime
{
  void vd_bindAsioDriverManager(AsioDrivers* driverManager)
  {
    asioDrivers = driverManager;

    // END
    return;
  }

  void vd_unbindAsioDriverManager()
  {
    asioDrivers = nullptr;

    // END
    return;
  }

  void vd_releaseAsioRuntime(HANDLE_asioRuntime*& handle_asioRuntime)
  {
    if (handle_asioRuntime == nullptr)
    {
      return;
    }

    // Stop the ASIO stream before releasing its buffers.
    if (handle_asioRuntime->asioState >= const_asioStateRunning)
    {
      const ASIOError asioErrorStop = ASIOStop();
      if (asioErrorStop != ASE_OK)
      {
        vd_printErrorVendor("ASIO", "STOP failed (" + std::to_string(static_cast<int32_t>(asioErrorStop)) + ")");
      }
      handle_asioRuntime->asioState = const_asioStateBuffersCreated;
    }

    // Release all buffers owned by the ASIO driver.
    if (handle_asioRuntime->asioState >= const_asioStateBuffersCreated)
    {
      const ASIOError asioErrorDisposeBuffers = ASIODisposeBuffers();
      if (asioErrorDisposeBuffers != ASE_OK)
      {
        vd_printErrorVendor("ASIO", "DISPOSEBUFFERS failed (" + std::to_string(static_cast<int32_t>(asioErrorDisposeBuffers)) + ")");
      }
      handle_asioRuntime->asioState = const_asioStateInitialized;
    }

    // Exit the ASIO driver before unloading it.
    if (handle_asioRuntime->asioState >= const_asioStateInitialized)
    {
      const ASIOError asioErrorExit = ASIOExit();
      if (asioErrorExit != ASE_OK)
      {
        vd_printErrorVendor("ASIO", "EXIT failed (" + std::to_string(static_cast<int32_t>(asioErrorExit)) + ")");
      }
      handle_asioRuntime->asioState = const_asioStateDriverLoaded;
    }

    // Unload the ASIO driver.
    if (handle_asioRuntime->asioState >= const_asioStateDriverLoaded)
    {
      handle_asioRuntime->driverManager.removeCurrentDriver();
      handle_asioRuntime->asioState = const_asioStateDriverUnloaded;
    }

    handle_asioRuntime->listBufferInfo.clear();
    handle_asioRuntime->listIdxBufferInput.clear();
    handle_asioRuntime->listIdxBufferOutput.clear();
    handle_asioRuntime->listReadBuffer.clear();
    handle_asioRuntime->listWriteBuffer.clear();
    handle_asioRuntime->llistInputToSemanticIn.clear();
    handle_asioRuntime->llistOutputFromSemanticOut.clear();
    handle_asioRuntime->listBufferFloat.clear();
    handle_asioRuntime->binding = {};
    vd_unbindAsioDriverManager();
    delete handle_asioRuntime;
    handle_asioRuntime = nullptr;

    // END
    return;
  }

  bool is_spbCapabilityValid(const STRUCT_spbCapability& capability)
  {
    if (capability.min <= 0 || capability.max < capability.min)
    {
      return false;
    }
    if (capability.preferred < capability.min || capability.preferred > capability.max)
    {
      return false;
    }

    if (capability.min == capability.max)
    {
      return capability.preferred == capability.min && capability.granularity == 0;
    }
    if (capability.granularity != -1 && capability.granularity <= 0)
    {
      return false;
    }

    return is_spbSupported(capability, capability.preferred);
  }

  bool is_spbSupported(const STRUCT_spbCapability& capability, long spb)
  {
    if (spb < capability.min || spb > capability.max)
    {
      return false;
    }
    if (capability.min == capability.max)
    {
      return spb == capability.min;
    }
    if (capability.granularity == -1)
    {
      long supportedSpb = capability.min;
      while (supportedSpb < spb && supportedSpb <= capability.max / 2)
      {
        supportedSpb *= 2;
      }
      return supportedSpb == spb;
    }

    return capability.granularity > 0 && (spb - capability.min) % capability.granularity == 0;
  }
}
