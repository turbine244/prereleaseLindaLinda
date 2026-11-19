#include "02_shared/lsm/01_audio/_dllExport.h"

#include "03_edge/audio/linda.asio/src/__env.h"
#include "03_edge/audio/linda.asio/src/_h/01_api/_lindaAsio.h"

using namespace lindaAsio;

//====----====----====----====----====----====----====----====---- FDEF

namespace lsm::audio
{
  extern "C" void vd_getInfo(STRUCT_lsmInfo& out_info)
  {
    out_info =
    {
      .name = ENV_WHOAMI,
      .versionMajor = ENV_VERSION_MAJOR,
      .versionMinor = ENV_VERSION_MINOR,
      .versionPatch = ENV_VERSION_PATCH,
      .kind = ENUM_lsmKind::audio,
      .description = ENV_DESCRIPTION
    };
    return;
  }

  extern "C" TYPE_rc rc_listDevice(std::vector<STRUCT_audioDeviceInfo>& out_listAudioDeviceInfo)
  {
    return api::rc_listDevice(out_listAudioDeviceInfo);
  }

  extern "C" TYPE_rc rc_scanChannel(const std::string& nameDevice, bool isVerbose, TYPE_json& out_jsonScanResult)
  {
    return api::rc_scanChannel(nameDevice, isVerbose, out_jsonScanResult);
  }

  extern "C" TYPE_rc rc_openAudioRuntime(const TYPE_json& jsonRunner, uint32_t& out_spb, void*& out_handle_audioRuntime)
  {
    return api::rc_openAudioRuntime(jsonRunner, out_spb, out_handle_audioRuntime);
  }

  extern "C" TYPE_rc rc_bindAudioRuntime(const REQUEST_rc_bindAudioRuntime& request, void* handle_audioRuntime)
  {
    return api::rc_bindAudioRuntime(request, handle_audioRuntime);
  }

  extern "C" TYPE_rc rc_setSps(void* handle_audioRuntime, uint32_t newSps, uint32_t& out_sps)
  {
    return api::rc_setSps(handle_audioRuntime, newSps, out_sps);
  }

  extern "C" TYPE_rc rc_startAudioRuntime(void* handle_audioRuntime)
  {
    return api::rc_startAudioRuntime(handle_audioRuntime);
  }

  extern "C" TYPE_rc rc_stopAudioRuntime(void* handle_audioRuntime)
  {
    return api::rc_stopAudioRuntime(handle_audioRuntime);
  }

  extern "C" void vd_closeAudioRuntime(void*& handle_audioRuntime)
  {
    api::vd_closeAudioRuntime(handle_audioRuntime);
    return;
  }
}

