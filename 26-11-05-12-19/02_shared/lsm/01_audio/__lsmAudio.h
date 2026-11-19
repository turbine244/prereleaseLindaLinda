#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/json.hpp"

//====----====----====----====----====----====----====----====---- TYPE

namespace lsm::audio
{ 
  struct STRUCT_audioDeviceInfo
  {
    std::string name = "";
    uint32_t numInput = 0;
    uint32_t numOutput = 0;
  };

  // Both callbacks run on the audio thread and must not block.
  using CALLBACK_rc_processBuffer =
    TYPE_rc (*)();

  using CALLBACK_rc_notifySpsChange =
    TYPE_rc (*)(uint32_t sps);

  struct REQUEST_rc_bindAudioRuntime
  {
    uint32_t spb = 0; // Fixed until runtime close.
    uint32_t numSemanticIn = 0;
    uint32_t numSemanticOut = 0;

    float* ptrSemanticIn = nullptr; // Borrowed until runtime close.
    float* ptrSemanticOut = nullptr; // Borrowed until runtime close.

    CALLBACK_rc_processBuffer rc_processBuffer = nullptr;
    CALLBACK_rc_notifySpsChange rc_notifySpsChange = nullptr;
  };

  using CALLBACK_rc_listDevice =
    TYPE_rc (*)(std::vector<STRUCT_audioDeviceInfo>& out_listAudioDeviceInfo);
  
  using CALLBACK_rc_scanChannel =
    TYPE_rc (*)(const std::string& nameDevice, bool isVerbose, TYPE_json& out_jsonScanResult);

  using CALLBACK_rc_openAudioRuntime =
    TYPE_rc (*)(const TYPE_json& jsonRunner, uint32_t& out_spb, void*& out_handle_audioRuntime);

  using CALLBACK_rc_bindAudioRuntime =
    TYPE_rc (*)(const REQUEST_rc_bindAudioRuntime& request, void* handle_audioRuntime);
  
  using CALLBACK_rc_setSps =
    TYPE_rc (*)(void* handle_audioRuntime, uint32_t newSps, uint32_t& out_sps);

  using CALLBACK_rc_startAudioRuntime =
    TYPE_rc (*)(void* handle_audioRuntime);

  using CALLBACK_rc_stopAudioRuntime =
    TYPE_rc (*)(void* handle_audioRuntime);

  using CALLBACK_vd_closeAudioRuntime =
    void (*)(void*& handle_audioRuntime);

  
}

