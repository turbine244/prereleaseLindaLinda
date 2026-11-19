#pragma once

/*
  Audio support DLL discovery, loading & release.
*/

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "01_standard/phase00/error.hpp"
#include "02_shared/lsm/00_common/__lsmInfo.h"
#include "02_shared/lsm/01_audio/__lsmAudio.h"

//====----====----====----====----====----====----====----====---- TYPE

namespace linda0::audioSupport
{
  struct STRUCT_audioSupportInfo
  {
    std::filesystem::path pathFileDll = "";
    std::string name = "";
    uint32_t versionMajor = 0;
    uint32_t versionMinor = 0;
    uint32_t versionPatch = 0;
    std::string description = "";
  };

  struct INTF_audioSupport
  {
    void* handle = nullptr;
    STRUCT_audioSupportInfo info = {};

    lsm::audio::CALLBACK_rc_listDevice rc_listDevice = nullptr;
    lsm::audio::CALLBACK_rc_scanChannel rc_scanChannel = nullptr;
    lsm::audio::CALLBACK_rc_openAudioRuntime rc_openAudioRuntime = nullptr;
    lsm::audio::CALLBACK_rc_bindAudioRuntime rc_bindAudioRuntime = nullptr;
    lsm::audio::CALLBACK_rc_setSps rc_setSps = nullptr;
    lsm::audio::CALLBACK_rc_startAudioRuntime rc_startAudioRuntime = nullptr;
    lsm::audio::CALLBACK_rc_stopAudioRuntime rc_stopAudioRuntime = nullptr;
    lsm::audio::CALLBACK_vd_closeAudioRuntime vd_closeAudioRuntime = nullptr;
  };
}

//====----====----====----====----====----====----====----====---- FDEC

namespace linda0::audioSupport
{
  TYPE_rc rc_list(const std::filesystem::path& pathDirLsm, std::vector<STRUCT_audioSupportInfo>& out_listInfo);
  TYPE_rc rc_load(const std::filesystem::path& pathDirLsm, const std::string& nameFileDll, INTF_audioSupport& out_interface);
  TYPE_rc rc_free(INTF_audioSupport& edit_interface);
}

