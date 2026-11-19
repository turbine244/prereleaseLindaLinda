#pragma once

/*
  Audio support module : linda.asio (ASIO)
*/

#include <cstdint>
#include <string>
#include <vector>

#include "02_shared/lsm/01_audio/__lsmAudio.h"
#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/json.hpp"

//====----====----====----====----====----====----====----====---- FDEC

namespace lindaAsio::api
{
  TYPE_rc rc_listDevice(std::vector<lsm::audio::STRUCT_audioDeviceInfo>& out_listAudioDeviceInfo);
  TYPE_rc rc_scanChannel(const std::string& nameDevice, bool isVerbose, TYPE_json& out_jsonScanResult);

  TYPE_rc rc_openAudioRuntime(const TYPE_json& jsonRunner, uint32_t& out_spb, void*& out_handle_audioRuntime);
  TYPE_rc rc_bindAudioRuntime(const lsm::audio::REQUEST_rc_bindAudioRuntime& request, void* handle_audioRuntime);
  TYPE_rc rc_setSps(void* handle_audioRuntime, uint32_t newSps, uint32_t& out_sps);
  TYPE_rc rc_startAudioRuntime(void* handle_audioRuntime);
  TYPE_rc rc_stopAudioRuntime(void* handle_audioRuntime);
  void vd_closeAudioRuntime(void*& handle_audioRuntime);
}
