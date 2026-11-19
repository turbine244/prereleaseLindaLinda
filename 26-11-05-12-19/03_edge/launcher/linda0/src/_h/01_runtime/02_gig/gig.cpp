#include "03_edge/launcher/linda0/src/_h/01_runtime/02_gig/_gig.h"

#include <atomic>
#include <utility>

#include "03_edge/launcher/linda0/src/_h/01_runtime/02_gig/narrative/_narr.openGigRuntime.h"

using namespace std;

//====----====----====----====----====----====----====----====---- FDEC

namespace linda0::gig
{
  namespace /* private */
  {
    void _vd_releaseGigRuntime(HANDLE_gigRuntime*& handle_gigRuntime);
    TYPE_rc _rc_processBuffer();
    TYPE_rc _rc_notifySpsChange(uint32_t sps);
  }
}

//====----====----====----====----====----====----====----====---- VAR

namespace linda0::gig
{
  namespace /* private */
  {
    HANDLE_gigRuntime* _handle_activeGigRuntime = nullptr;
  }
}

//====----====----====----====----====----====----====----====---- FDEF

namespace linda0::gig
{
  TYPE_rc rc_openGigRuntime(const REQUEST_rc_openGigRuntime& request, HANDLE_gigRuntime*& out_handle_gigRuntime)
  {
    // NULLIFY : rc
    TYPE_rc rce = 0;
    TYPE_rc rci = 0;

    // RCI : require an empty output handle
    rci--;
    if (out_handle_gigRuntime != nullptr)
    {
      vd_printErrorRc(rci, "GIG runtime output handle is already set");
      return rci;
    }

    // NULLIFY : out
    out_handle_gigRuntime = nullptr;

    // NARRATIVE [0]
    narr::openGigRuntime::STRUCT_context narrativeContext =
    {
      .input =
      {
        .jsonRunner = request.jsonRunner,
        .audioSupport = request.audioSupport,
        .languageSupport = request.languageSupport,
        .isRuntimeActive = (_handle_activeGigRuntime != nullptr),
        .callbackProcessBuffer = _rc_processBuffer,
        .callbackNotifySpsChange = _rc_notifySpsChange
      }
    };

    // NARRATIVE [1]
    if (true)
    {
      rce = to_abs(narr::openGigRuntime::narr_rc_validateInputs(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed validating GIG runtime inputs");
        return rce;
      }
    }

    // NARRATIVE [2]
    if (true)
    {
      rce = to_abs(narr::openGigRuntime::narr_rc_initializeRuntime(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed initializing GIG runtime");
        _vd_releaseGigRuntime(narrativeContext.state.handle_gigRuntime);
        return rce;
      }

      _handle_activeGigRuntime = narrativeContext.state.handle_gigRuntime;
    }

    // NARRATIVE [3]
    if (true)
    {
      rce = to_abs(narr::openGigRuntime::narr_rc_openAudioRuntime(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed opening audio runtime");
        _vd_releaseGigRuntime(narrativeContext.state.handle_gigRuntime);
        return rce;
      }
    }

    // NARRATIVE [4]
    if (true)
    {
      rce = to_abs(narr::openGigRuntime::narr_rc_openLanguageRuntime(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed opening language runtime");
        _vd_releaseGigRuntime(narrativeContext.state.handle_gigRuntime);
        return rce;
      }
    }

    // NARRATIVE [5]
    if (true)
    {
      rce = to_abs(narr::openGigRuntime::narr_rc_allocateSemanticBuffers(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed allocating semantic buffers");
        _vd_releaseGigRuntime(narrativeContext.state.handle_gigRuntime);
        return rce;
      }
    }

    // NARRATIVE [6]
    if (true)
    {
      rce = to_abs(narr::openGigRuntime::narr_rc_bindLanguageRuntime(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed binding language runtime");
        _vd_releaseGigRuntime(narrativeContext.state.handle_gigRuntime);
        return rce;
      }
    }

    // NARRATIVE [7]
    if (true)
    {
      rce = to_abs(narr::openGigRuntime::narr_rc_bindAudioRuntime(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed binding audio runtime");
        _vd_releaseGigRuntime(narrativeContext.state.handle_gigRuntime);
        return rce;
      }
    }

    // NARRATIVE [8]
    if (true)
    {
      rce = to_abs(narr::openGigRuntime::narr_rc_setSps(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed setting SPS");
        _vd_releaseGigRuntime(narrativeContext.state.handle_gigRuntime);
        return rce;
      }
    }

    // END
    out_handle_gigRuntime = narrativeContext.state.handle_gigRuntime;
    narrativeContext.state.handle_gigRuntime = nullptr;
    return 0;
  }

  TYPE_rc rc_startGigRuntime(HANDLE_gigRuntime* handle_gigRuntime)
  {
    // NULLIFY : rc
    TYPE_rc rce = 0;
    TYPE_rc rci = 0;

    // RCI : require the active GIG runtime
    rci--;
    const bool isGigRuntimeActive = handle_gigRuntime != nullptr && handle_gigRuntime == _handle_activeGigRuntime;
    if (!isGigRuntimeActive)
    {
      vd_printErrorRc(rci, "GIG runtime is not active");
      return rci;
    }

    // RCI : validate the audio runtime
    rci--;
    const bool isAudioRuntimeReady = handle_gigRuntime->handle_audioRuntime != nullptr && handle_gigRuntime->ptr_audioSupport != nullptr && handle_gigRuntime->ptr_audioSupport->rc_startAudioRuntime != nullptr;
    if (!isAudioRuntimeReady)
    {
      vd_printErrorRc(rci, "Audio runtime handle or start callback is not set");
      return rci;
    }

    // RCE : start the audio runtime
    rce = to_abs(handle_gigRuntime->ptr_audioSupport->rc_startAudioRuntime(handle_gigRuntime->handle_audioRuntime));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed starting audio runtime");
      return rce;
    }

    // END
    return 0;
  }

  TYPE_rc rc_stopGigRuntime(HANDLE_gigRuntime* handle_gigRuntime)
  {
    // NULLIFY : rc
    TYPE_rc rce = 0;
    TYPE_rc rci = 0;

    // RCI : require the active GIG runtime
    rci--;
    const bool isGigRuntimeActive = handle_gigRuntime != nullptr && handle_gigRuntime == _handle_activeGigRuntime;
    if (!isGigRuntimeActive)
    {
      vd_printErrorRc(rci, "GIG runtime is not active");
      return rci;
    }

    // RCI : validate the audio runtime
    rci--;
    const bool isAudioRuntimeReady = handle_gigRuntime->handle_audioRuntime != nullptr && handle_gigRuntime->ptr_audioSupport != nullptr && handle_gigRuntime->ptr_audioSupport->rc_stopAudioRuntime != nullptr;
    if (!isAudioRuntimeReady)
    {
      vd_printErrorRc(rci, "Audio runtime handle or stop callback is not set");
      return rci;
    }

    // RCE : stop the audio runtime
    rce = to_abs(handle_gigRuntime->ptr_audioSupport->rc_stopAudioRuntime(handle_gigRuntime->handle_audioRuntime));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed stopping audio runtime");
      return rce;
    }

    // END
    return 0;
  }

  TYPE_rc rc_setSps(HANDLE_gigRuntime* handle_gigRuntime, uint32_t sps)
  {
    // NULLIFY : rc
    TYPE_rc rce = 0;
    TYPE_rc rci = 0;

    // RCI : require the active GIG runtime
    rci--;
    const bool isGigRuntimeActive = handle_gigRuntime != nullptr && handle_gigRuntime == _handle_activeGigRuntime;
    if (!isGigRuntimeActive)
    {
      vd_printErrorRc(rci, "GIG runtime is not active");
      return rci;
    }

    // RCI : validate the audio runtime
    rci--;
    const bool isAudioRuntimeReady = handle_gigRuntime->handle_audioRuntime != nullptr && handle_gigRuntime->ptr_audioSupport != nullptr && handle_gigRuntime->ptr_audioSupport->rc_setSps != nullptr;
    if (!isAudioRuntimeReady)
    {
      vd_printErrorRc(rci, "Audio runtime handle or SPS callback is not set");
      return rci;
    }

    uint32_t spsAccepted = 0;

    // RCE : request the SPS change from the audio runtime
    rce = to_abs(handle_gigRuntime->ptr_audioSupport->rc_setSps(handle_gigRuntime->handle_audioRuntime, sps, spsAccepted));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed setting audio SPS");
      return rce;
    }

    // RCI : require an accepted SPS
    rci--;
    if (spsAccepted == 0)
    {
      vd_printErrorRc(rci, "Audio runtime returned an invalid SPS");
      return rci;
    }

    handle_gigRuntime->sps = spsAccepted;

    // END
    return 0;
  }

  void vd_closeGigRuntime(HANDLE_gigRuntime*& handle_gigRuntime)
  {
    _vd_releaseGigRuntime(handle_gigRuntime);

    //END
    return;
  }
}

//====----====----====----====----====----====----====----====---- FDEF

namespace linda0::gig
{
  namespace /* private */
  {
    void _vd_releaseGigRuntime(HANDLE_gigRuntime*& handle_gigRuntime)
    {
      if (handle_gigRuntime == nullptr)
      {
        //END
        return;
      }

      // Close audio before releasing the language runtime and shared buffers.
      const bool isAudioRuntimeClosable = handle_gigRuntime->handle_audioRuntime != nullptr && handle_gigRuntime->ptr_audioSupport != nullptr && handle_gigRuntime->ptr_audioSupport->vd_closeAudioRuntime != nullptr;
      if (isAudioRuntimeClosable)
      {
        handle_gigRuntime->ptr_audioSupport->vd_closeAudioRuntime(handle_gigRuntime->handle_audioRuntime);
      }
      handle_gigRuntime->handle_audioRuntime = nullptr;

      if (_handle_activeGigRuntime == handle_gigRuntime)
      {
        _handle_activeGigRuntime = nullptr;
      }

      // Close the language runtime after audio callbacks have stopped.
      const bool isLanguageRuntimeClosable = handle_gigRuntime->handle_languageRuntime != nullptr && handle_gigRuntime->ptr_languageSupport != nullptr && handle_gigRuntime->ptr_languageSupport->vd_closeLanguageRuntime != nullptr;
      if (isLanguageRuntimeClosable)
      {
        handle_gigRuntime->ptr_languageSupport->vd_closeLanguageRuntime(handle_gigRuntime->handle_languageRuntime);
      }
      handle_gigRuntime->handle_languageRuntime = nullptr;

      delete handle_gigRuntime;
      handle_gigRuntime = nullptr;

      //END
      return;
    }

    TYPE_rc _rc_processBuffer()
    {
      TYPE_rc rci = 0;

      // RCI : validate the active GIG runtime
      rci--;
      if (_handle_activeGigRuntime == nullptr)
      {
        vd_printErrorRc(rci, "GIG runtime is inactive");
        return rci;
      }

      // RCI : return the first buffer processing error
      const TYPE_rc rcePrevious = to_abs(_handle_activeGigRuntime->rcProcessBuffer.load(memory_order_acquire));
      if (rcePrevious != 0)
      {
        vd_printErrorRc(rcePrevious, "Previous buffer processing failed");
        return rcePrevious;
      }

      // RCI : validate the language runtime
      rci--;
      const bool isLanguageRuntimeReady = _handle_activeGigRuntime->handle_languageRuntime != nullptr && _handle_activeGigRuntime->ptr_languageSupport != nullptr && _handle_activeGigRuntime->ptr_languageSupport->rc_processBuffer != nullptr;
      if (!isLanguageRuntimeReady)
      {
        _handle_activeGigRuntime->rcProcessBuffer.store(rci, memory_order_release);
        vd_printErrorRc(rci, "Language runtime handle or process callback is not set");
        return rci;
      }

      // RCE : process one semantic buffer
      const TYPE_rc rce = to_abs(_handle_activeGigRuntime->ptr_languageSupport->rc_processBuffer(_handle_activeGigRuntime->handle_languageRuntime));
      if (rce != 0)
      {
        _handle_activeGigRuntime->rcProcessBuffer.store(rce, memory_order_release);
        vd_printErrorRc(rce, "Language buffer processing failed");
        return rce;
      }

      //END
      return 0;
    }

    TYPE_rc _rc_notifySpsChange(uint32_t sps)
    {
      TYPE_rc rci = 0;

      // RCI : validate the active GIG runtime and SPS
      rci--;
      const bool isSpsChangeValid = _handle_activeGigRuntime != nullptr && sps != 0;
      if (!isSpsChangeValid)
      {
        vd_printErrorRc(rci, "GIG runtime or SPS is invalid");
        return rci;
      }

      _handle_activeGigRuntime->sps = sps;

      //END
      return 0;
    }
  }
}



