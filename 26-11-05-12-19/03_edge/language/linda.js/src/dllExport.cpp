#include "02_shared/lsm/02_language/_dllExport.h"

#include "03_edge/language/linda.js/src/__env.h"
#include "03_edge/language/linda.js/src/_h/02_api/_lindaJs.h"

using namespace lindaJs;

//====----====----====----====----====----====----====----====---- FDEF

namespace lsm::language
{
  extern "C" void vd_getInfo(STRUCT_lsmInfo& out_info)
  {
    out_info =
    {
      .name = ENV_WHOAMI,
      .versionMajor = ENV_VERSION_MAJOR,
      .versionMinor = ENV_VERSION_MINOR,
      .versionPatch = ENV_VERSION_PATCH,
      .kind = ENUM_lsmKind::language,
      .description = ENV_DESCRIPTION
    };
    return;
  }

  extern "C" TYPE_rc rc_scanSemantic(const std::string& strSource, bool isVerbose, TYPE_json& out_jsonScanResult)
  {
    return lindaJs::api::rc_scanSemantic(strSource, isVerbose, out_jsonScanResult);
  }

  extern "C" TYPE_rc rc_buildRig(const REQUEST_rc_buildRig& request)
  {
    return lindaJs::api::rc_buildRig(request);
  }

  extern "C" TYPE_rc rc_openLanguageRuntime(const TYPE_json& jsonRunner, void*& out_handle_languageRuntime)
  {
    return lindaJs::api::rc_openLanguageRuntime(jsonRunner, out_handle_languageRuntime);
  }

  extern "C" TYPE_rc rc_bindLanguageRuntime(const REQUEST_rc_bindLanguageRuntime& request, void* handle_languageRuntime)
  {
    return lindaJs::api::rc_bindLanguageRuntime(request, handle_languageRuntime);
  }

  extern "C" TYPE_rc rc_processBuffer(void* handle_languageRuntime)
  {
    return lindaJs::api::rc_processBuffer(handle_languageRuntime);
  }

  extern "C" void vd_closeLanguageRuntime(void*& handle_languageRuntime)
  {
    lindaJs::api::vd_closeLanguageRuntime(handle_languageRuntime);
    return;
  }
}
