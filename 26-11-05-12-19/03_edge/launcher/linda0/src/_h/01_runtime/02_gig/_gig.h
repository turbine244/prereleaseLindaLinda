#pragma once

/*
  Definition of GIG interface
  : a stream session that bridges the language runtime & the audio runtime
*/

#include <atomic>
#include <cstdint>
#include <vector>

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/json.hpp"
#include "03_edge/launcher/linda0/src/_h/01_runtime/00_audioSupport/_audioSupport.h"
#include "03_edge/launcher/linda0/src/_h/01_runtime/01_languageSupport/_languageSupport.h"

//====----====----====----====----====----====----====----====---- TYPE

namespace linda0::gig
{
  struct HANDLE_gigRuntime
  {
    const audioSupport::INTF_audioSupport* ptr_audioSupport = nullptr; // Borrowed while this GIG is open.
    void* handle_audioRuntime = nullptr;

    const languageSupport::INTF_languageSupport* ptr_languageSupport = nullptr; // Borrowed while this GIG is open.
    void* handle_languageRuntime = nullptr;

    uint32_t sps = 0;
    uint32_t spb = 0;
    uint32_t numSemanticIn = 0;
    uint32_t numSemanticOut = 0;
    std::vector<float> listSemanticIn = {};
    std::vector<float> listSemanticOut = {};

    std::atomic<TYPE_rc> rcProcessBuffer = 0;
  };

  struct REQUEST_rc_openGigRuntime
  {
    const TYPE_json& jsonRunner;
    const audioSupport::INTF_audioSupport& audioSupport;
    const languageSupport::INTF_languageSupport& languageSupport;
  };
}

//====----====----====----====----====----====----====----====---- FDEC

namespace linda0::gig
{
  TYPE_rc rc_openGigRuntime(const REQUEST_rc_openGigRuntime& request, HANDLE_gigRuntime*& out_handle_gigRuntime);
  TYPE_rc rc_startGigRuntime(HANDLE_gigRuntime* handle_gigRuntime);
  TYPE_rc rc_stopGigRuntime(HANDLE_gigRuntime* handle_gigRuntime);
  TYPE_rc rc_setSps(HANDLE_gigRuntime* handle_gigRuntime, uint32_t sps);
  void vd_closeGigRuntime(HANDLE_gigRuntime*& handle_gigRuntime);
}
