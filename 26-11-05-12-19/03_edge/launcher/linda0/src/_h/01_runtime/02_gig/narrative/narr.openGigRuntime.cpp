#include "03_edge/launcher/linda0/src/_h/01_runtime/02_gig/narrative/_narr.openGigRuntime.h"

#include <exception>
#include <new>
#include <utility>
#include <vector>

using namespace std;

//====----====----====----====----====----====----====----====---- FDEF

namespace linda0::gig::narr::openGigRuntime
{
  // [1]
  TYPE_rc narr_rc_validateInputs(STRUCT_context& edit_context)
  {
    // NULLIFY : rc, context.state.requestedSps
    edit_context.state.requestedSps = 0;
    TYPE_rc rci = 0;

    // RCI : validate the runner fields used by GIG
    rci--;
    const bool isRunnerFormatValid =
    {
      edit_context.input.jsonRunner.is_object()
      && edit_context.input.jsonRunner.contains("wire")
      && edit_context.input.jsonRunner["wire"].is_object()
      && edit_context.input.jsonRunner["wire"].contains("audio")
      && edit_context.input.jsonRunner["wire"]["audio"].is_object()
      && edit_context.input.jsonRunner["wire"]["audio"].contains("sps")
      && edit_context.input.jsonRunner.contains("semantic")
      && edit_context.input.jsonRunner["semantic"].is_object()
      && edit_context.input.jsonRunner["semantic"].contains("front")
      && edit_context.input.jsonRunner["semantic"]["front"].is_object()
    };
    if (!isRunnerFormatValid)
    {
      vd_printErrorRc(rci, "GIG runner format is invalid");
      return rci;
    }

    // RCI : validate the requested SPS
    rci--;
    if (!is_jsonValueUint32(edit_context.input.jsonRunner["wire"]["audio"]["sps"]))
    {
      vd_printErrorRc(rci, "GIG SPS is invalid");
      return rci;
    }

    edit_context.state.requestedSps = edit_context.input.jsonRunner["wire"]["audio"]["sps"].get<uint32_t>();

    // RCI : validate the loaded audio support interface
    rci--;
    const bool isAudioSupportInterfaceComplete =
    {
      edit_context.input.audioSupport.handle != nullptr
      && edit_context.input.audioSupport.rc_openAudioRuntime != nullptr
      && edit_context.input.audioSupport.rc_bindAudioRuntime != nullptr
      && edit_context.input.audioSupport.rc_setSps != nullptr
      && edit_context.input.audioSupport.rc_startAudioRuntime != nullptr
      && edit_context.input.audioSupport.rc_stopAudioRuntime != nullptr
      && edit_context.input.audioSupport.vd_closeAudioRuntime != nullptr
    };
    if (!isAudioSupportInterfaceComplete)
    {
      vd_printErrorRc(rci, "Audio support interface is incomplete");
      return rci;
    }

    // RCI : validate the loaded language support interface
    rci--;
    const bool isLanguageSupportInterfaceComplete =
    {
      edit_context.input.languageSupport.handle != nullptr
      && edit_context.input.languageSupport.rc_openLanguageRuntime != nullptr
      && edit_context.input.languageSupport.rc_bindLanguageRuntime != nullptr
      && edit_context.input.languageSupport.rc_processBuffer != nullptr
      && edit_context.input.languageSupport.vd_closeLanguageRuntime != nullptr
    };
    if (!isLanguageSupportInterfaceComplete)
    {
      vd_printErrorRc(rci, "Language support interface is incomplete");
      return rci;
    }

    // END
    return 0;
  }

  // [2]
  TYPE_rc narr_rc_initializeRuntime(STRUCT_context& edit_context)
  {
    // NULLIFY : rc, context.state.handle_gigRuntime
    edit_context.state.handle_gigRuntime = nullptr;
    TYPE_rc rci = 0;

    // RCI : reject another active GIG runtime
    rci--;
    if (edit_context.input.isRuntimeActive)
    {
      vd_printErrorRc(rci, "GIG runtime is already active");
      return rci;
    }

    // RCI : allocate the GIG runtime handle
    rci--;
    edit_context.state.handle_gigRuntime = new (nothrow) HANDLE_gigRuntime();
    if (edit_context.state.handle_gigRuntime == nullptr)
    {
      vd_printErrorRc(rci, "Failed allocating GIG runtime");
      return rci;
    }

    edit_context.state.handle_gigRuntime->ptr_audioSupport = &edit_context.input.audioSupport;
    edit_context.state.handle_gigRuntime->ptr_languageSupport = &edit_context.input.languageSupport;

    // END
    return 0;
  }

  // [3]
  TYPE_rc narr_rc_openAudioRuntime(STRUCT_context& edit_context)
  {
    TYPE_rc rce = to_abs(edit_context.input.audioSupport.rc_openAudioRuntime(edit_context.input.jsonRunner, edit_context.state.handle_gigRuntime->spb, edit_context.state.handle_gigRuntime->handle_audioRuntime));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed opening audio runtime");
      return rce;
    }

    // RCI : require a valid audio runtime handle and SPB
    TYPE_rc rci = 0;
    rci--;
    const bool isAudioRuntimeValid = edit_context.state.handle_gigRuntime->handle_audioRuntime != nullptr && edit_context.state.handle_gigRuntime->spb != 0;
    if (!isAudioRuntimeValid)
    {
      vd_printErrorRc(rci, "Audio runtime returned an invalid handle or SPB");
      return rci;
    }

    // END
    return 0;
  }

  // [4]
  TYPE_rc narr_rc_openLanguageRuntime(STRUCT_context& edit_context)
  {
    TYPE_rc rce = to_abs(edit_context.input.languageSupport.rc_openLanguageRuntime(edit_context.input.jsonRunner, edit_context.state.handle_gigRuntime->handle_languageRuntime));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed opening language runtime");
      return rce;
    }

    // RCI : require a valid language runtime handle
    TYPE_rc rci = 0;
    rci--;
    if (edit_context.state.handle_gigRuntime->handle_languageRuntime == nullptr)
    {
      vd_printErrorRc(rci, "Language runtime returned an invalid handle");
      return rci;
    }

    // END
    return 0;
  }

  // [5]
  TYPE_rc narr_rc_allocateSemanticBuffers(STRUCT_context& edit_context)
  {
    // NULLIFY : rc
    TYPE_rc rci = 0;

    // RCI : validate the semantic channel fields
    rci--;
    const TYPE_json& front = edit_context.input.jsonRunner["semantic"]["front"];
    const bool isSemanticFormatValid =
    {
      front.contains("numIn")
      && front.contains("numOut")
      && front.contains("listIn")
      && front["listIn"].is_array()
      && front.contains("listOut")
      && front["listOut"].is_array()
    };
    if (!isSemanticFormatValid)
    {
      vd_printErrorRc(rci, "Semantic format is invalid");
      return rci;
    }

    // RCI : validate the semantic channel counts
    rci--;
    if (!is_jsonValueUint32(front["numIn"]) || !is_jsonValueUint32(front["numOut"]))
    {
      vd_printErrorRc(rci, "Semantic channel count is invalid");
      return rci;
    }

    const uint32_t numSemanticIn = front["numIn"].get<uint32_t>();
    const uint32_t numSemanticOut = front["numOut"].get<uint32_t>();

    // RCI : validate the semantic channel list sizes
    rci--;
    if (front["listIn"].size() != numSemanticIn || front["listOut"].size() != numSemanticOut)
    {
      vd_printErrorRc(rci, "Semantic channel count does not match its list");
      return rci;
    }

    // RCI : validate the semantic buffer sizes
    rci--;
    const size_t maxBufferSize = vector<float>().max_size();
    const bool isSemanticBufferSizeValid =
    {
      edit_context.state.handle_gigRuntime->spb != 0
      && static_cast<size_t>(numSemanticIn) <= maxBufferSize / edit_context.state.handle_gigRuntime->spb
      && static_cast<size_t>(numSemanticOut) <= maxBufferSize / edit_context.state.handle_gigRuntime->spb
    };
    if (!isSemanticBufferSizeValid)
    {
      vd_printErrorRc(rci, "Semantic buffer size is out of range");
      return rci;
    }

    vector<float> temp_listSemanticIn;
    vector<float> temp_listSemanticOut;
    bool isSemanticBufferAllocated = true;
    try
    {
      temp_listSemanticIn.resize(static_cast<size_t>(numSemanticIn) * edit_context.state.handle_gigRuntime->spb);
      temp_listSemanticOut.resize(static_cast<size_t>(numSemanticOut) * edit_context.state.handle_gigRuntime->spb);
    }
    catch (const exception&)
    {
      isSemanticBufferAllocated = false;
    }

    rci--;
    if (!isSemanticBufferAllocated)
    {
      vd_printErrorRc(rci, "Failed allocating semantic buffers");
      return rci;
    }

    edit_context.state.handle_gigRuntime->numSemanticIn = numSemanticIn;
    edit_context.state.handle_gigRuntime->numSemanticOut = numSemanticOut;
    edit_context.state.handle_gigRuntime->listSemanticIn = move(temp_listSemanticIn);
    edit_context.state.handle_gigRuntime->listSemanticOut = move(temp_listSemanticOut);

    // END
    return 0;
  }

  // [6]
  TYPE_rc narr_rc_bindLanguageRuntime(STRUCT_context& edit_context)
  {
    lsm::language::REQUEST_rc_bindLanguageRuntime requestLanguage = {};
    requestLanguage.spb = edit_context.state.handle_gigRuntime->spb;
    requestLanguage.numSemanticIn = edit_context.state.handle_gigRuntime->numSemanticIn;
    requestLanguage.numSemanticOut = edit_context.state.handle_gigRuntime->numSemanticOut;
    requestLanguage.ptrSemanticIn = edit_context.state.handle_gigRuntime->listSemanticIn.data();
    requestLanguage.ptrSemanticOut = edit_context.state.handle_gigRuntime->listSemanticOut.data();
    requestLanguage.ptrSps = &edit_context.state.handle_gigRuntime->sps;

    TYPE_rc rce = to_abs(edit_context.input.languageSupport.rc_bindLanguageRuntime(requestLanguage, edit_context.state.handle_gigRuntime->handle_languageRuntime));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed binding language runtime");
      return rce;
    }

    // END
    return 0;
  }

  // [7]
  TYPE_rc narr_rc_bindAudioRuntime(STRUCT_context& edit_context)
  {
    lsm::audio::REQUEST_rc_bindAudioRuntime requestAudio = {};
    requestAudio.spb = edit_context.state.handle_gigRuntime->spb;
    requestAudio.numSemanticIn = edit_context.state.handle_gigRuntime->numSemanticIn;
    requestAudio.numSemanticOut = edit_context.state.handle_gigRuntime->numSemanticOut;
    requestAudio.ptrSemanticIn = edit_context.state.handle_gigRuntime->listSemanticIn.data();
    requestAudio.ptrSemanticOut = edit_context.state.handle_gigRuntime->listSemanticOut.data();
    requestAudio.rc_processBuffer = edit_context.input.callbackProcessBuffer;
    requestAudio.rc_notifySpsChange = edit_context.input.callbackNotifySpsChange;

    TYPE_rc rce = to_abs(edit_context.input.audioSupport.rc_bindAudioRuntime(requestAudio, edit_context.state.handle_gigRuntime->handle_audioRuntime));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed binding audio runtime");
      return rce;
    }

    // END
    return 0;
  }

  // [8]
  TYPE_rc narr_rc_setSps(STRUCT_context& edit_context)
  {
    uint32_t spsAccepted = 0;
    TYPE_rc rce = to_abs(edit_context.input.audioSupport.rc_setSps(edit_context.state.handle_gigRuntime->handle_audioRuntime, edit_context.state.requestedSps, spsAccepted));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed setting SPS");
      return rce;
    }

    // RCI : require the audio runtime to return a valid SPS
    TYPE_rc rci = 0;
    rci--;
    if (spsAccepted == 0)
    {
      vd_printErrorRc(rci, "Audio runtime returned an invalid SPS");
      return rci;
    }

    edit_context.state.handle_gigRuntime->sps = spsAccepted;

    // END
    return 0;
  }
}




