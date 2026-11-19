#include "03_edge/launcher/linda0/src/_h/02_shell/01_command/narrative/_narr.commandRun.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <system_error>
#include <thread>
#include <utility>

#if defined(_WIN32)
#include <conio.h>
#else
#include <sys/select.h>
#include <unistd.h>
#endif

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase00/now.hpp"
#include "01_standard/phase01/ioFile.hpp"
#include "03_edge/launcher/linda0/src/_h/01_runtime/02_gig/_gig.h"
#include "03_edge/launcher/linda0/src/_h/00_file/00_rig/_rigPackage.h"
#include "03_edge/launcher/linda0/src/_h/02_shell/01_command/_support.h"

using namespace std;

//====----====----====----====----====----====----====----====---- PRIVATE : FDEC

namespace
{
  string _str_runnerValidationError(const TYPE_json& runner);
  bool _is_positiveId(const TYPE_json& value);
  string _str_semanticValidationError(const TYPE_json& semantic);
  TYPE_rc _rc_loadRunner(const filesystem::path& pathFileRunner, TYPE_json& out_runner);
  TYPE_rc _rc_loadSemantic(const TYPE_json& runner, TYPE_json& out_semantic);
  TYPE_rc _rc_filterUnavailableSemanticLinks(TYPE_json& runner);
  TYPE_rc _rc_writeRunError(const TYPE_json& runner, const TYPE_json& jsonError);
  TYPE_rc _rc_updateLastRun(const filesystem::path& pathFileRunner, TYPE_json& runner, TYPE_rc rcRun);
  TYPE_rc _rc_printProcessError(TYPE_rc rceProcess, const TYPE_json& runner);
  void _vd_printProcessLocation(const char* nameLocation, const TYPE_json& location);
}

//====----====----====----====----====----====----====----====---- PUBLIC : FDEF

namespace linda0::command::run::narr::commandRun
{
  // [1]
  TYPE_rc narr_rc_prepareRun(STRUCT_context& edit_context)
  {
    // NULLIFY : rc
    TYPE_rc rci = 0;
    edit_context.state.rceRun = 0;
    edit_context.state.pathFileRunner = filesystem::absolute(edit_context.input.line.mapOperand.at("runner")).lexically_normal();
    edit_context.state.isBackupEnabled = edit_context.input.line.setOptionFlag.contains("--backup");
    edit_context.state.isDebugEnabled = edit_context.input.line.setOptionFlag.contains("--debug");

    // LOAD runner
    edit_context.state.jsonRunner = {};
    TYPE_rc rce = to_abs(_rc_loadRunner(edit_context.state.pathFileRunner, edit_context.state.jsonRunner));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed loading runner");
      return rce;
    }
    edit_context.state.jsonRunnerRecorded = edit_context.state.jsonRunner;

    // RESOLVE runner paths
    if (true)
    {
      const filesystem::path pathDirRunner = edit_context.state.pathFileRunner.parent_path();
      for (const string key : {"source", "rig"})
      {
        if (!edit_context.state.jsonRunner["paths"].contains(key))
        {
          continue;
        }
        filesystem::path path = edit_context.state.jsonRunner["paths"][key].get<string>();
        if (path.is_relative())
        {
          path = pathDirRunner / path;
        }
        edit_context.state.jsonRunner["paths"][key] = filesystem::absolute(path).lexically_normal().generic_string();
      }
    }

    const filesystem::path pathDirRig = edit_context.state.jsonRunner["paths"]["rig"].get<string>();
    // RCI : validate the runner RIG directory
    rci--;
    if (!filesystem::is_directory(pathDirRig))
    {
      vd_printErrorRc(rci, "Runner RIG path is not a directory");
      const TYPE_rc rceUpdate = to_abs(_rc_updateLastRun(edit_context.state.pathFileRunner, edit_context.state.jsonRunnerRecorded, rci));
      if (rceUpdate != 0)
      {
        return rceUpdate;
      }
      return rci;
    }

    TYPE_json jsonManifest;
    rce = to_abs(rc_readFileJson(rigPackage::pathFile_manifest(pathDirRig), jsonManifest));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed loading RIG package manifest");
      const TYPE_rc rceUpdate = to_abs(_rc_updateLastRun(edit_context.state.pathFileRunner, edit_context.state.jsonRunnerRecorded, rce));
      if (rceUpdate != 0)
      {
        return rceUpdate;
      }
      return rce;
    }

    // RCI : validate the RIG package manifest
    rci--;
    if (true)
    {
      const bool isManifestValid =
      {
        jsonManifest.is_object()
        && jsonManifest.contains("type")
        && jsonManifest["type"].is_string()
        && jsonManifest["type"] == rigPackage::const_typeManifest
        && jsonManifest.contains("schemaVersion")
        && is_jsonValueUint32(jsonManifest["schemaVersion"])
        && jsonManifest["schemaVersion"].get<uint32_t>() == rigPackage::const_schemaVersionCurrent
        && jsonManifest.contains("languageSupport")
        && jsonManifest["languageSupport"].is_string()
        && !jsonManifest["languageSupport"].get<string>().empty()
        && jsonManifest.contains("semantic")
        && jsonManifest["semantic"].is_string()
        && jsonManifest["semantic"] == rigPackage::const_nameFileSemantic
        && jsonManifest.contains("error")
        && jsonManifest["error"].is_string()
        && jsonManifest["error"] == rigPackage::const_pathRelativeFileError
      };
      if (!isManifestValid)
      {
        vd_printErrorRc(rci, "RIG package manifest is invalid");
        const TYPE_rc rceUpdate = to_abs(_rc_updateLastRun(edit_context.state.pathFileRunner, edit_context.state.jsonRunnerRecorded, rci));
        if (rceUpdate != 0)
        {
          return rceUpdate;
        }
        return rci;
      }
    }

    edit_context.state.jsonRunner["rigPackage"] = {
      {"languageSupport", jsonManifest["languageSupport"]},
      {"semantic", (pathDirRig / jsonManifest["semantic"].get<string>()).generic_string()},
      {"error", (pathDirRig / jsonManifest["error"].get<string>()).generic_string()}
    };
    edit_context.state.jsonRunner["runOption"] = {
      {"debug", edit_context.state.isDebugEnabled}
    };

    // LOAD the semantic contract owned by the RIG package.
    TYPE_json jsonSemantic;
    rce = to_abs(_rc_loadSemantic(edit_context.state.jsonRunner, jsonSemantic));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed loading RIG semantic contract");
      const TYPE_rc rceReport = to_abs(_rc_writeRunError(edit_context.state.jsonRunner, {{"stage", "semantic"}, {"rc", rce}, {"message", "Failed loading RIG semantic contract"}}));
      if (rceReport != 0)
      {
        return rceReport;
      }
      const TYPE_rc rceUpdate = to_abs(_rc_updateLastRun(edit_context.state.pathFileRunner, edit_context.state.jsonRunnerRecorded, rce));
      if (rceUpdate != 0)
      {
        return rceUpdate;
      }
      return rce;
    }
    edit_context.state.jsonRunner["semantic"] = move(jsonSemantic);

    // Start this run without a stale error report from an earlier attempt.
    if (true)
    {
      error_code ecRemoveError;
      filesystem::remove(edit_context.state.jsonRunner["rigPackage"]["error"].get<string>(), ecRemoveError);
      if (ecRemoveError)
      {
        cerr << "WARNING: Failed removing stale run error report" << endl;
      }
    }

    // FILTER unavailable semantic links
    rce = to_abs(_rc_filterUnavailableSemanticLinks(edit_context.state.jsonRunner));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed filtering semantic links");
      const TYPE_rc rceReport = to_abs(_rc_writeRunError(edit_context.state.jsonRunner, {{"stage", "wire"}, {"rc", rce}, {"message", "Failed filtering semantic links"}}));
      if (rceReport != 0)
      {
        return rceReport;
      }
      const TYPE_rc rceUpdate = to_abs(_rc_updateLastRun(edit_context.state.pathFileRunner, edit_context.state.jsonRunnerRecorded, rce));
      if (rceUpdate != 0)
      {
        return rceUpdate;
      }
      return rce;
    }


    // END
    return 0;
  }

  // [2]
  TYPE_rc narr_rc_executeRun(STRUCT_context& edit_context)
  {
    TYPE_rc rce = 0;
    TYPE_rc rci = 0;
    const string nameAudioSupport = edit_context.state.jsonRunner["wire"]["audio"]["name"].get<string>();
    const string nameLanguageSupport = edit_context.state.jsonRunner["rigPackage"]["languageSupport"].get<string>();
    linda0::audioSupport::INTF_audioSupport interfaceAudio = {};
    linda0::languageSupport::INTF_languageSupport interfaceLanguage = {};
    linda0::gig::HANDLE_gigRuntime* handle_gigRuntime = nullptr;

    rce = to_abs(linda0::command::_rc_loadAudioSupport(nameAudioSupport, interfaceAudio));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed loading audio support: " + nameAudioSupport);
      const TYPE_rc rceReport = to_abs(_rc_writeRunError(edit_context.state.jsonRunner, {{"stage", "audioSupport"}, {"rc", rce}, {"message", "Failed loading audio support"}}));
      if (rceReport != 0)
      {
        return rceReport;
      }
      const TYPE_rc rceUpdate = to_abs(_rc_updateLastRun(edit_context.state.pathFileRunner, edit_context.state.jsonRunnerRecorded, rce));
      if (rceUpdate != 0)
      {
        return rceUpdate;
      }
      return rce;
    }

    rce = to_abs(linda0::command::_rc_loadLanguageSupport(nameLanguageSupport, interfaceLanguage));
    if (rce != 0)
    {
      linda0::audioSupport::rc_free(interfaceAudio);
      vd_printErrorRc(rce, "Failed loading language support: " + nameLanguageSupport);
      const TYPE_rc rceReport = to_abs(_rc_writeRunError(edit_context.state.jsonRunner, {{"stage", "languageSupport"}, {"rc", rce}, {"message", "Failed loading language support"}}));
      if (rceReport != 0)
      {
        return rceReport;
      }
      const TYPE_rc rceUpdate = to_abs(_rc_updateLastRun(edit_context.state.pathFileRunner, edit_context.state.jsonRunnerRecorded, rce));
      if (rceUpdate != 0)
      {
        return rceUpdate;
      }
      return rce;
    }

    const linda0::gig::REQUEST_rc_openGigRuntime requestOpen =
    {
      edit_context.state.jsonRunner,
      interfaceAudio,
      interfaceLanguage
    };
    rce = to_abs(linda0::gig::rc_openGigRuntime(requestOpen, handle_gigRuntime));
    if (rce != 0)
    {
      const TYPE_rc rceFreeLanguage = to_abs(linda0::languageSupport::rc_free(interfaceLanguage));
      const TYPE_rc rceFreeAudio = to_abs(linda0::audioSupport::rc_free(interfaceAudio));
      vd_printErrorRc(rce, "Failed opening GIG runtime");
      const TYPE_rc rceReport = to_abs(_rc_writeRunError(edit_context.state.jsonRunner, {{"stage", "open"}, {"rc", rce}, {"message", "Failed opening GIG runtime"}}));
      if (rceReport != 0)
      {
        return rceReport;
      }
      const TYPE_rc rceUpdate = to_abs(_rc_updateLastRun(edit_context.state.pathFileRunner, edit_context.state.jsonRunnerRecorded, rce));
      if (rceUpdate != 0)
      {
        return rceUpdate;
      }
      if (rceFreeLanguage != 0)
      {
        return rceFreeLanguage;
      }
      if (rceFreeAudio != 0)
      {
        return rceFreeAudio;
      }
      return rce;
    }

    rci--;
    if (handle_gigRuntime == nullptr)
    {
      const TYPE_rc rceReport = to_abs(_rc_writeRunError(edit_context.state.jsonRunner, {{"stage", "access"}, {"rc", rci}, {"message", "GIG runtime handle was not returned"}}));
      const TYPE_rc rceFreeLanguage = to_abs(linda0::languageSupport::rc_free(interfaceLanguage));
      const TYPE_rc rceFreeAudio = to_abs(linda0::audioSupport::rc_free(interfaceAudio));
      vd_printErrorRc(rci, "GIG runtime handle was not returned");
      if (rceReport != 0)
      {
        return rceReport;
      }
      const TYPE_rc rceUpdate = to_abs(_rc_updateLastRun(edit_context.state.pathFileRunner, edit_context.state.jsonRunnerRecorded, rci));
      if (rceUpdate != 0)
      {
        return rceUpdate;
      }
      if (rceFreeLanguage != 0)
      {
        return rceFreeLanguage;
      }
      if (rceFreeAudio != 0)
      {
        return rceFreeAudio;
      }
      return rci;
    }

    rce = to_abs(linda0::gig::rc_startGigRuntime(handle_gigRuntime));
    if (rce != 0)
    {
      linda0::gig::vd_closeGigRuntime(handle_gigRuntime);
      const TYPE_rc rceFreeLanguage = to_abs(linda0::languageSupport::rc_free(interfaceLanguage));
      const TYPE_rc rceFreeAudio = to_abs(linda0::audioSupport::rc_free(interfaceAudio));
      vd_printErrorRc(rce, "Failed starting GIG runtime");
      const TYPE_rc rceReport = to_abs(_rc_writeRunError(edit_context.state.jsonRunner, {{"stage", "start"}, {"rc", rce}, {"message", "Failed starting GIG runtime"}}));
      if (rceReport != 0)
      {
        return rceReport;
      }
      const TYPE_rc rceUpdate = to_abs(_rc_updateLastRun(edit_context.state.pathFileRunner, edit_context.state.jsonRunnerRecorded, rce));
      if (rceUpdate != 0)
      {
        return rceUpdate;
      }
      if (rceFreeLanguage != 0)
      {
        return rceFreeLanguage;
      }
      if (rceFreeAudio != 0)
      {
        return rceFreeAudio;
      }
      return rce;
    }

    cout << "Running - press Enter to stop" << endl;
    while (handle_gigRuntime->rcProcessBuffer.load(memory_order_acquire) == 0)
    {
      bool isEnterPressed = false;
#if defined(_WIN32)
      if (_kbhit())
      {
        const int key = _getch();
        isEnterPressed = key == '\r' || key == '\n';
        if (key == 0 || key == 224)
        {
          static_cast<void>(_getch());
        }
      }
#else
      fd_set readSet;
      FD_ZERO(&readSet);
      FD_SET(STDIN_FILENO, &readSet);
      timeval timeout = {};
      if (select(STDIN_FILENO + 1, &readSet, nullptr, nullptr, &timeout) > 0)
      {
        isEnterPressed = cin.get() == '\n';
      }
#endif
      if (isEnterPressed)
      {
        break;
      }
      this_thread::sleep_for(chrono::milliseconds(10));
    }

    const TYPE_rc rceStop = to_abs(linda0::gig::rc_stopGigRuntime(handle_gigRuntime));
    const TYPE_rc rceProcess = to_abs(handle_gigRuntime->rcProcessBuffer.load(memory_order_acquire));
    edit_context.state.rceDiagnostic = 0;
    if (rceProcess != 0)
    {
      edit_context.state.rceRun = rceProcess;
      edit_context.state.rceDiagnostic = to_abs(_rc_printProcessError(edit_context.state.rceRun, edit_context.state.jsonRunner));
    }
    else if (rceStop != 0)
    {
      edit_context.state.rceRun = rceStop;
      vd_printErrorRc(edit_context.state.rceRun, "Failed stopping GIG runtime");
      edit_context.state.rceDiagnostic = to_abs(_rc_writeRunError(edit_context.state.jsonRunner, {{"stage", "stop"}, {"rc", edit_context.state.rceRun}, {"message", "Failed stopping GIG runtime"}}));
    }

    linda0::gig::vd_closeGigRuntime(handle_gigRuntime);
    const TYPE_rc rceFreeLanguage = to_abs(linda0::languageSupport::rc_free(interfaceLanguage));
    const TYPE_rc rceFreeAudio = to_abs(linda0::audioSupport::rc_free(interfaceAudio));
    if (edit_context.state.rceRun == 0 && rceFreeLanguage != 0)
    {
      edit_context.state.rceRun = rceFreeLanguage;
      edit_context.state.rceDiagnostic = to_abs(_rc_writeRunError(edit_context.state.jsonRunner, {{"stage", "languageSupportRelease"}, {"rc", rceFreeLanguage}, {"message", "Failed releasing language support"}}));
    }
    if (edit_context.state.rceRun == 0 && rceFreeAudio != 0)
    {
      edit_context.state.rceRun = rceFreeAudio;
      edit_context.state.rceDiagnostic = to_abs(_rc_writeRunError(edit_context.state.jsonRunner, {{"stage", "audioSupportRelease"}, {"rc", rceFreeAudio}, {"message", "Failed releasing audio support"}}));
    }

    // END
    return 0;
  }

  // [3]
  TYPE_rc narr_rc_recordRunResult(STRUCT_context& edit_context)
  {
    TYPE_rc rce = 0;
    const bool isRunSuccess = edit_context.state.rceRun == 0;
    rce = to_abs(_rc_updateLastRun(edit_context.state.pathFileRunner, edit_context.state.jsonRunnerRecorded, edit_context.state.rceRun));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed updating runner run state");
      const TYPE_rc rceReport = to_abs(_rc_writeRunError(edit_context.state.jsonRunner, {{"stage", "state"}, {"rc", rce}, {"message", "Failed updating runner run state"}}));
      if (rceReport != 0)
      {
        return rceReport;
      }
      return rce;
    }

    if (isRunSuccess && edit_context.state.isBackupEnabled)
    {
      filesystem::path pathFileBackup = edit_context.state.pathFileRunner;
      pathFileBackup += ".bak";
      rce = to_abs(rc_writeFileJsonAtomic(pathFileBackup, edit_context.state.jsonRunnerRecorded));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed writing runner backup");
        const TYPE_rc rceReport = to_abs(_rc_writeRunError(edit_context.state.jsonRunner, {{"stage", "backup"}, {"rc", rce}, {"message", "Failed writing runner backup"}}));
        if (rceReport != 0)
        {
          return rceReport;
        }
        return rce;
      }
    }

    if (isRunSuccess)
    {
      // Remove a stale error report after a completely successful run.
      error_code ecRemoveError;
      filesystem::remove(edit_context.state.jsonRunner["rigPackage"]["error"].get<string>(), ecRemoveError);
      if (ecRemoveError)
      {
        cerr << "WARNING: Failed removing stale run error report" << endl;
      }
    }

    if (edit_context.state.rceDiagnostic != 0)
    {
      return edit_context.state.rceDiagnostic;
    }


    // END
    return edit_context.state.rceRun;
  }
}

//====----====----====----====----====----====----====----====---- PRIVATE : FDEF

namespace
{
  string _str_runnerValidationError(const TYPE_json& runner)
  {
    // Validate the runner root and recorded paths.
    if (!runner.is_object())
    {
      return "root must be an object";
    }
    if (!runner.contains("paths") || !runner["paths"].is_object())
    {
      return "paths must be an object";
    }
    for (const string key : {"rig"})
    {
      if (!runner["paths"].contains(key) || !runner["paths"][key].is_string() || runner["paths"][key].get_ref<const string&>().empty())
      {
        return "paths." + key + " must be a non-empty string";
      }
    }
    if (true)
    {
      const bool isSourcePathInvalid =
      {
        runner["paths"].contains("source")
        && (
          !runner["paths"]["source"].is_string()
          || runner["paths"]["source"].get_ref<const string&>().empty()
        )
      };
      if (isSourcePathInvalid)
      {
        return "paths.source must be a non-empty string when present";
      }
    }

    // Validate the audio configuration and wire containers.
    if (!runner.contains("wire") || !runner["wire"].is_object())
    {
      return "wire must be an object";
    }
    if (!runner["wire"].contains("audio") || !runner["wire"]["audio"].is_object())
    {
      return "wire.audio must be an object";
    }
    const TYPE_json& audio = runner["wire"]["audio"];
    if (!audio.contains("name") || !audio["name"].is_string())
    {
      return "wire.audio.name must be a string";
    }
    if (!audio.contains("sps") || !is_jsonValueUint32(audio["sps"]))
    {
      return "wire.audio.sps must be an unsigned 32-bit integer";
    }
    if (!audio.contains("spb") || !is_jsonValueUint32(audio["spb"]))
    {
      return "wire.audio.spb must be an unsigned 32-bit integer";
    }
    if (!audio.contains("meta") || !audio["meta"].is_object())
    {
      return "wire.audio.meta must be an object";
    }
    if (!runner["wire"].contains("listLindaIn") || !runner["wire"]["listLindaIn"].is_array())
    {
      return "wire.listLindaIn must be an array";
    }
    if (!runner["wire"].contains("listLindaOut") || !runner["wire"]["listLindaOut"].is_array())
    {
      return "wire.listLindaOut must be an array";
    }

    // Validate the shape of every Linda-In and Linda-Out link.
    for (const TYPE_json& lindaIn : runner["wire"]["listLindaIn"])
    {
      if (!lindaIn.is_object())
      {
        return "wire.listLindaIn entries must be objects";
      }
      if (!lindaIn.contains("name") || !lindaIn["name"].is_string())
      {
        return "wire.listLindaIn.name must be a string";
      }
      if (!lindaIn.contains("listIdSrc") || !lindaIn["listIdSrc"].is_array())
      {
        return "wire.listLindaIn.listIdSrc must be an array";
      }
      if (!lindaIn.contains("listIdDest") || !lindaIn["listIdDest"].is_array())
      {
        return "wire.listLindaIn.listIdDest must be an array";
      }
      for (const TYPE_json& id : lindaIn["listIdSrc"])
      {
        if (!_is_positiveId(id))
        {
          return "wire.listLindaIn.listIdSrc must contain positive 32-bit IDs";
        }
      }
      for (const TYPE_json& id : lindaIn["listIdDest"])
      {
        if (!_is_positiveId(id))
        {
          return "wire.listLindaIn.listIdDest must contain positive 32-bit IDs";
        }
      }
    }
    for (const TYPE_json& lindaOut : runner["wire"]["listLindaOut"])
    {
      if (!lindaOut.is_object())
      {
        return "wire.listLindaOut entries must be objects";
      }
      if (!lindaOut.contains("name") || !lindaOut["name"].is_string())
      {
        return "wire.listLindaOut.name must be a string";
      }
      if (!lindaOut.contains("listIdSrc") || !lindaOut["listIdSrc"].is_array())
      {
        return "wire.listLindaOut.listIdSrc must be an array";
      }
      if (!lindaOut.contains("listIdDest") || !lindaOut["listIdDest"].is_array())
      {
        return "wire.listLindaOut.listIdDest must be an array";
      }
      for (const TYPE_json& id : lindaOut["listIdSrc"])
      {
        if (!_is_positiveId(id))
        {
          return "wire.listLindaOut.listIdSrc must contain positive 32-bit IDs";
        }
      }
      for (const TYPE_json& id : lindaOut["listIdDest"])
      {
        if (!_is_positiveId(id))
        {
          return "wire.listLindaOut.listIdDest must contain positive 32-bit IDs";
        }
      }
    }

    return {};
  }

  bool _is_positiveId(const TYPE_json& value)
  {
    if (!is_jsonValueUint32(value))
    {
      return false;
    }
    return value.is_number_unsigned() ? value.get<uint64_t>() > 0 : value.get<int64_t>() > 0;
  }

  string _str_semanticValidationError(const TYPE_json& semantic)
  {
    // Validate the semantic root and front container.
    if (!semantic.is_object())
    {
      return "root must be an object";
    }
    if (!semantic.contains("front") || !semantic["front"].is_object())
    {
      return "front must be an object";
    }
    const TYPE_json& front = semantic["front"];
    if (!front.contains("numIn") || !front["numIn"].is_number_unsigned())
    {
      return "numIn must be an unsigned integer";
    }
    if (!front.contains("numOut") || !front["numOut"].is_number_unsigned())
    {
      return "numOut must be an unsigned integer";
    }
    const uint64_t numIn = front["numIn"].get<uint64_t>();
    const uint64_t numOut = front["numOut"].get<uint64_t>();
    if (numIn > (numeric_limits<uint32_t>::max)())
    {
      return "numIn is out of range";
    }
    if (numOut > (numeric_limits<uint32_t>::max)())
    {
      return "numOut is out of range";
    }
    if (!front.contains("listIn") || !front["listIn"].is_array())
    {
      return "listIn must be an array";
    }
    if (!front.contains("listOut") || !front["listOut"].is_array())
    {
      return "listOut must be an array";
    }
    if (front["listIn"].size() != numIn)
    {
      return "numIn does not match listIn";
    }
    if (front["listOut"].size() != numOut)
    {
      return "numOut does not match listOut";
    }

    return {};
  }

  TYPE_rc _rc_loadRunner(const filesystem::path& pathFileRunner, TYPE_json& out_runner)
  {
    // NULLIFY : rc, out
    out_runner = nullptr;
    TYPE_json temp_out_runner;

    // Load only the runner explicitly requested by the user.
    TYPE_rc rce = to_abs(rc_readFileJson(pathFileRunner, temp_out_runner));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed loading runner file");
      return rce;
    }

    const string strValidationError = _str_runnerValidationError(temp_out_runner);
    // RCI : validate the runner structure
    TYPE_rc rci = 0;
    rci--;
    if (!strValidationError.empty())
    {
      vd_printErrorRc(rci, "Runner format is invalid: " + strValidationError);
      return rci;
    }

    out_runner = move(temp_out_runner);

    // END
    return 0;
  }

  TYPE_rc _rc_loadSemantic(const TYPE_json& runner, TYPE_json& out_semantic)
  {
    // NULLIFY : rc, out
    out_semantic = nullptr;
    TYPE_json temp_out_semantic;

    // Load the semantic contract stored beside the RIG artifact.
    TYPE_rc rce = to_abs(rc_readFileJson(runner["rigPackage"]["semantic"].get<string>(), temp_out_semantic));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed loading RIG semantic contract");
      return rce;
    }

    const string strValidationError = _str_semanticValidationError(temp_out_semantic);
    // RCI : validate the semantic contract
    TYPE_rc rci = 0;
    rci--;
    if (!strValidationError.empty())
    {
      vd_printErrorRc(rci, "RIG semantic contract is invalid: " + strValidationError);
      return rci;
    }

    out_semantic = move(temp_out_semantic);

    // END
    return 0;
  }

  TYPE_rc _rc_filterUnavailableSemanticLinks(TYPE_json& runner)
  {
    // NULLIFY : rc
    TYPE_rc rci = 0;
    const TYPE_json& front = runner["semantic"]["front"];
    const uint64_t numIn = front["numIn"].get<uint64_t>();
    const uint64_t numOut = front["numOut"].get<uint64_t>();

    // Keep Linda-In destinations that refer to existing semantic inputs.
    // RCI : validate Linda-In destination IDs
    rci--;
    for (TYPE_json& lindaIn : runner["wire"]["listLindaIn"])
    {
      TYPE_json listIdDest = TYPE_json::array();
      for (const TYPE_json& value : lindaIn["listIdDest"])
      {
        if (!value.is_number_unsigned())
        {
          vd_printErrorRc(rci, "Linda-In destination ID must be an unsigned integer");
          return rci;
        }
        const uint64_t id = value.get<uint64_t>();
        if ((id >= 1) && (id <= numIn))
        {
          listIdDest.push_back(id);
        }
        else
        {
          cerr << "WARNING: Ignored unavailable Linda-In destination ID " << id << endl;
        }
      }
      lindaIn["listIdDest"] = move(listIdDest);
    }

    // Keep Linda-Out sources that refer to existing semantic outputs.
    // RCI : validate Linda-Out source IDs
    rci--;
    for (TYPE_json& lindaOut : runner["wire"]["listLindaOut"])
    {
      TYPE_json listIdSrc = TYPE_json::array();
      for (const TYPE_json& value : lindaOut["listIdSrc"])
      {
        if (!value.is_number_unsigned())
        {
          vd_printErrorRc(rci, "Linda-Out source ID must be an unsigned integer");
          return rci;
        }
        const uint64_t id = value.get<uint64_t>();
        if ((id >= 1) && (id <= numOut))
        {
          listIdSrc.push_back(id);
        }
        else
        {
          cerr << "WARNING: Ignored unavailable Linda-Out source ID " << id << endl;
        }
      }
      lindaOut["listIdSrc"] = move(listIdSrc);
    }

    // END
    return 0;
  }

  TYPE_rc _rc_writeRunError(const TYPE_json& runner, const TYPE_json& jsonError)
  {
    // NULLIFY : rc
    // Store command-level failures in the same report consumed by Run.
    const TYPE_rc rce = to_abs(rc_writeFileJsonAtomic(runner["rigPackage"]["error"].get<string>(), jsonError));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed writing run error report");
      return rce;
    }

    // END
    return 0;
  }

  TYPE_rc _rc_updateLastRun(const filesystem::path& pathFileRunner, TYPE_json& runner, TYPE_rc rcRun)
  {
    // NULLIFY : rc
    // Record every run attempt after a structurally valid runner has been loaded.
    const string timestamp = str_now();
    // RCI : create the run timestamp
    TYPE_rc rci = 0;
    rci--;
    if (timestamp.empty())
    {
      vd_printErrorRc(rci, "Failed creating run timestamp");
      return rci;
    }

    const TYPE_json lastRun =
    {
      { "status", rcRun == 0 ? "ok" : "failed" },
      { "finishedAt", timestamp }
    };

    // Keep lastRun directly below paths while preserving all remaining fields.
    TYPE_json reordered = TYPE_json::object();
    reordered["paths"] = runner["paths"];
    reordered["lastRun"] = lastRun;
    for (const auto& [key, value] : runner.items())
    {
      if ((key != "paths") && (key != "lastRun"))
      {
        reordered[key] = value;
      }
    }
    runner = move(reordered);

    const TYPE_rc rce = to_abs(rc_writeFileJsonAtomic(pathFileRunner, runner));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed updating runner run state");
      return rce;
    }

    // END
    return 0;
  }

  TYPE_rc _rc_printProcessError(TYPE_rc rceProcess, const TYPE_json& runner)
  {
    // NULLIFY : rc
    // Load the language-specific error report written under the debug directory.
    const filesystem::path pathFileError = runner["rigPackage"]["error"].get<string>();
    TYPE_json jsonError;
    const TYPE_rc rceRead = to_abs(rc_readFileJson(pathFileError, jsonError));
    if (rceRead != 0 || !jsonError.is_object())
    {
      jsonError = {{"stage", "process"}, {"rc", rceProcess}, {"message", "GIG processing failed"}};
      const TYPE_rc rceReport = to_abs(_rc_writeRunError(runner, jsonError));
      vd_printErrorRc(rceProcess, "GIG processing failed");
      return rceReport;
    }

    // Always print the message, even when source-location data is unavailable.
    const string message = jsonError.contains("message") && jsonError["message"].is_string() ? jsonError["message"].get<string>() : "GIG processing failed";
    vd_printErrorRc(rceProcess, message);
    if (jsonError.contains("payload"))
    {
      _vd_printProcessLocation("payload", jsonError["payload"]);
    }
    if (jsonError.contains("source"))
    {
      _vd_printProcessLocation("source", jsonError["source"]);
    }

    // END
    return 0;
  }

  void _vd_printProcessLocation(const char* nameLocation, const TYPE_json& location)
  {
    if (true)
    {
      const bool isLocationValid =
      {
        location.is_object()
        && location.contains("path")
        && location["path"].is_string()
        && location.contains("point")
        && location["point"].is_object()
      };
      if (!isLocationValid)
      {
        return;
      }
    }

    const TYPE_json& point = location["point"];
    if (true)
    {
      const bool isPointValid =
      {
        point.contains("row")
        && point["row"].is_number_unsigned()
        && point.contains("column")
        && point["column"].is_number_unsigned()
      };
      if (!isPointValid)
      {
        return;
      }
    }

    // Convert stored zero-based coordinates to user-facing one-based coordinates.
    const uint64_t row = point["row"].get<uint64_t>() + 1;
    const uint64_t column = point["column"].get<uint64_t>() + 1;
    cerr << "    at " << nameLocation << " " << location["path"].get<string>() << ":" << row << ":" << column << endl;

    // END
    return;
  }
}
