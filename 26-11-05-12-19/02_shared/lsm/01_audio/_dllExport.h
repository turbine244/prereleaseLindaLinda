#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/json.hpp"

#include "02_shared/lsm/00_common/__dllMacro.h"
#include "02_shared/lsm/00_common/__lsmInfo.h"
#include "02_shared/lsm/01_audio/__lsmAudio.h"

//====----====----====----====----====----====----====----====---- FDEC

namespace lsm::audio
{
  extern "C" LINDA_SUPPORT_EXPORT void vd_getInfo(STRUCT_lsmInfo& out_info);

  extern "C" LINDA_SUPPORT_EXPORT TYPE_rc rc_listDevice(std::vector<STRUCT_audioDeviceInfo>& out_listAudioDeviceInfo);
  extern "C" LINDA_SUPPORT_EXPORT TYPE_rc rc_scanChannel(const std::string& nameDevice, bool isVerbose, TYPE_json& out_jsonScanResult);

  extern "C" LINDA_SUPPORT_EXPORT TYPE_rc rc_openAudioRuntime(const TYPE_json& jsonRunner, uint32_t& out_spb, void*& out_handle_audioRuntime);
  extern "C" LINDA_SUPPORT_EXPORT TYPE_rc rc_bindAudioRuntime(const REQUEST_rc_bindAudioRuntime& request, void* handle_audioRuntime);
  extern "C" LINDA_SUPPORT_EXPORT TYPE_rc rc_setSps(void* handle_audioRuntime, uint32_t newSps, uint32_t& out_sps);
  extern "C" LINDA_SUPPORT_EXPORT TYPE_rc rc_startAudioRuntime(void* handle_audioRuntime);
  extern "C" LINDA_SUPPORT_EXPORT TYPE_rc rc_stopAudioRuntime(void* handle_audioRuntime);
  extern "C" LINDA_SUPPORT_EXPORT void vd_closeAudioRuntime(void*& handle_audioRuntime);
}
