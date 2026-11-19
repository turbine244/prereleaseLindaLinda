#include "03_edge/launcher/linda0/src/_h/02_shell/01_command/narrative/_narr.commandBuild.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>

#include "01_standard/phase00/error.hpp"
#include "01_standard/phase01/ioFile.hpp"

#include "03_edge/launcher/linda0/src/_h/00_file/00_rig/_rigPackage.h"
#include "03_edge/launcher/linda0/src/_h/02_shell/01_command/_support.h"

using namespace std;


//====----====----====----====----====----====----====----====---- PRIVATE : TYPE

namespace
{
  struct _REQUEST_rc_writeBuildError
  {
    filesystem::path pathFile;
    string stage;
    TYPE_rc rc = 0;
    string message;
  };
}

//====----====----====----====----====----====----====----====---- PRIVATE : FDEC

namespace
{
  bool _is_validWire(const TYPE_json& wire);
  bool _is_positiveId(const TYPE_json& value);
  bool _is_pathWithin(const filesystem::path& path, const filesystem::path& pathDir);
  filesystem::path _pathRecorded(const filesystem::path& path, const filesystem::path& pathBase, bool isAbsolute);
  string _str_packageOwnershipError(const filesystem::path& pathDirPackage);
  TYPE_rc _rc_recoverInterruptedBuild(const filesystem::path& pathDirPackage);
  TYPE_rc _rc_removeBuildPath(const filesystem::path& path);
  TYPE_rc _rc_removeInterruptedBuildPaths(const filesystem::path& pathDirPackage);
  TYPE_rc _rc_writeBuildError(const _REQUEST_rc_writeBuildError& request);
  TYPE_rc _rc_publishBuildErrors(const filesystem::path& pathDirPackageStaging, const filesystem::path& pathDirPackageComplete, bool& out_published);
}

//====----====----====----====----====----====----====----====---- PUBLIC : FDEF

namespace linda0::command::build::narr::commandBuild
{
  // [1]
  TYPE_rc narr_rc_prepareBuild(STRUCT_context& edit_context)
  {
    // NULLIFY : rc
    TYPE_rc rci = 0;
    edit_context.state.jsonWire = {};
    edit_context.state.hasWire = false;
    const unordered_map<string, string>::const_iterator itSource = edit_context.input.line.mapOperand.find("source");
    rci--;
    if (itSource == edit_context.input.line.mapOperand.end())
    {
      vd_printErrorRc(rci, "Source file is missing");
      return rci;
    }
    edit_context.state.pathFileSource = itSource->second;
    edit_context.state.nameLanguageSupport = edit_context.input.line.mapOperand.at("support");

    edit_context.state.pathDirPackage.clear();
    // RESOLVE package directory
    if (true)
    {
      if (edit_context.input.line.mapOptionValue.contains("--package"))
      {
        edit_context.state.pathDirPackage = edit_context.input.line.mapOptionValue.at("--package");
      }
      else
      {
        filesystem::path pathDirParent = edit_context.state.pathFileSource.parent_path();
        if (pathDirParent.empty())
        {
          pathDirParent = ".";
        }
        edit_context.state.pathDirPackage = pathDirParent / "rig";
      }
    }
    
    edit_context.state.pathFileRunner.clear();
    // RESOLVE runner file
    if (true)
    {
      if (edit_context.input.line.mapOptionValue.contains("--runner"))
      {
        edit_context.state.pathFileRunner = edit_context.input.line.mapOptionValue.at("--runner");
        if (edit_context.state.pathFileRunner.is_relative())
        {
          edit_context.state.pathFileRunner = edit_context.state.pathDirPackage / edit_context.state.pathFileRunner;
        }
      }
      else
      {
        filesystem::path pathDirSource = edit_context.state.pathFileSource.parent_path();
        if (pathDirSource.empty())
        {
          pathDirSource = ".";
        }
        edit_context.state.pathFileRunner = pathDirSource / "runner";
      }
    }

    edit_context.state.pathDirPackageComplete = filesystem::absolute(edit_context.state.pathDirPackage).lexically_normal();
    edit_context.state.pathFileSourceAbsolute = filesystem::absolute(edit_context.state.pathFileSource).lexically_normal();
    edit_context.state.pathFileRunnerAbsolute = filesystem::absolute(edit_context.state.pathFileRunner).lexically_normal();
    const uint64_t nonce = static_cast<uint64_t>(chrono::steady_clock::now().time_since_epoch().count());
    const string namePackage = edit_context.state.pathDirPackageComplete.filename().string();
    edit_context.state.pathDirPackageStaging = edit_context.state.pathDirPackageComplete.parent_path() / ("." + namePackage + ".building." + to_string(nonce));
    edit_context.state.pathDirPackagePrevious = edit_context.state.pathDirPackageComplete.parent_path() / ("." + namePackage + ".previous." + to_string(nonce));
    edit_context.state.pathFileBuildError = rigPackage::pathFile_error(edit_context.state.pathDirPackageComplete);
    edit_context.state.isAbsolute = edit_context.input.line.setOptionFlag.contains("--absolute");

    // Transactional publication requires a dedicated package directory.
    // RCI : validate the package directory
    rci--;
    if (edit_context.state.pathDirPackageComplete == edit_context.state.pathDirPackageComplete.root_path() || _is_pathWithin(edit_context.state.pathFileSourceAbsolute, edit_context.state.pathDirPackageComplete))
    {
      vd_printErrorRc(rci, "RIG package directory must not contain the source file");
      return rci;
    }

    TYPE_rc rce = to_abs(_rc_recoverInterruptedBuild(edit_context.state.pathDirPackageComplete));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed recovering interrupted build");
      return rce;
    }

    const string ownershipError = _str_packageOwnershipError(edit_context.state.pathDirPackageComplete);
    // RCI : validate package ownership
    rci--;
    if (!ownershipError.empty())
    {
      vd_printErrorRc(rci, ownershipError);
      return rci;
    }

    rce = to_abs(_rc_removeInterruptedBuildPaths(edit_context.state.pathDirPackageComplete));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed cleaning interrupted build paths");
      return rce;
    }

    // Preserve the wire configuration from an existing runner.
    if (filesystem::exists(edit_context.state.pathFileRunner))
    {
      TYPE_json jsonRunnerExisting;
      rce = to_abs(rc_readFileJson(edit_context.state.pathFileRunner, jsonRunnerExisting));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed loading existing runner file");
        const TYPE_rc rceReport = to_abs(_rc_writeBuildError({edit_context.state.pathFileBuildError, "runner", rce, "Failed loading existing runner file"}));
        if (rceReport != 0)
        {
          return rceReport;
        }
        return rce;
      }
      if (jsonRunnerExisting.contains("wire") && _is_validWire(jsonRunnerExisting["wire"]))
      {
        edit_context.state.jsonWire = jsonRunnerExisting["wire"];
        edit_context.state.hasWire = true;
      }
    }


    // END
    return 0;
  }

  // [2]
  TYPE_rc narr_rc_buildRigByLanguageSupport(STRUCT_context& edit_context)
  {
    TYPE_rc rce = to_abs(linda0::command::_rc_loadLanguageSupport(edit_context.state.nameLanguageSupport, edit_context.state.supportLanguage));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed loading language support");
      const TYPE_rc rceReport = to_abs(_rc_writeBuildError({edit_context.state.pathFileBuildError, "support", rce, "Failed loading language support"}));
      if (rceReport != 0)
      {
        return rceReport;
      }
      return rce;
    }

    TYPE_rc rci = 0;
    rci--;
    if (edit_context.state.supportLanguage.rc_buildRig == nullptr)
    {
      const TYPE_rc rceFree = to_abs(linda0::languageSupport::rc_free(edit_context.state.supportLanguage));
      vd_printErrorRc(rci, "RIG building is not supported by: " + edit_context.state.nameLanguageSupport);
      if (rceFree != 0)
      {
        return rceFree;
      }
      const TYPE_rc rceReport = to_abs(_rc_writeBuildError({edit_context.state.pathFileBuildError, "build", rci, "RIG building is not supported by the selected language support"}));
      if (rceReport != 0)
      {
        return rceReport;
      }
      return rci;
    }

    const lsm::language::REQUEST_rc_buildRig request =
    {
      edit_context.state.pathFileSource,
      edit_context.state.pathDirPackageStaging,
      edit_context.state.isAbsolute
    };
    rce = to_abs(edit_context.state.supportLanguage.rc_buildRig(request));
    const TYPE_rc rceFree = to_abs(linda0::languageSupport::rc_free(edit_context.state.supportLanguage));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed building RIG artifact");
      bool isPublished = false;
      const TYPE_rc rcePublish = to_abs(_rc_publishBuildErrors(edit_context.state.pathDirPackageStaging, edit_context.state.pathDirPackageComplete, isPublished));
      if (rcePublish != 0)
      {
        return rcePublish;
      }
      if (!isPublished)
      {
        const TYPE_rc rceReport = to_abs(_rc_writeBuildError({edit_context.state.pathFileBuildError, "build", rce, "Failed building RIG artifact"}));
        if (rceReport != 0)
        {
          return rceReport;
        }
      }
      const TYPE_rc rceRemove = to_abs(_rc_removeBuildPath(edit_context.state.pathDirPackageStaging));
      if (rceRemove != 0)
      {
        return rceRemove;
      }
      return rce;
    }
    if (rceFree != 0)
    {
      const TYPE_rc rceReport = to_abs(_rc_writeBuildError({edit_context.state.pathFileBuildError, "support", rceFree, "Failed releasing language support"}));
      if (rceReport != 0)
      {
        return rceReport;
      }
      const TYPE_rc rceRemove = to_abs(_rc_removeBuildPath(edit_context.state.pathDirPackageStaging));
      if (rceRemove != 0)
      {
        return rceRemove;
      }
      return rceFree;
    }

    rce = to_abs(linda0::rigPackage::rc_writeManifest(edit_context.state.pathDirPackageStaging, edit_context.state.nameLanguageSupport));
    if (rce != 0)
    {
      const TYPE_rc rceReport = to_abs(_rc_writeBuildError({edit_context.state.pathFileBuildError, "manifest", rce, "Failed writing RIG package manifest"}));
      if (rceReport != 0)
      {
        return rceReport;
      }
      const TYPE_rc rceRemove = to_abs(_rc_removeBuildPath(edit_context.state.pathDirPackageStaging));
      if (rceRemove != 0)
      {
        return rceRemove;
      }
      return rce;
    }

    // END
    return 0;
  }

  // [3]
  TYPE_rc narr_rc_stageRunner(STRUCT_context& edit_context)
  {
    edit_context.state.jsonRunner = {};
    edit_context.state.pathFileRunnerStaging.clear();
    edit_context.state.runnerInsidePackage = false;
    edit_context.state.hadPreviousPackage = false;
    TYPE_rc rce = 0;
    TYPE_rc rci = 0;

    // WRITE runner paths
    if (true)
    {
      const filesystem::path pathAbsDirRunner = filesystem::absolute(edit_context.state.pathFileRunner).lexically_normal().parent_path();
      const filesystem::path pathRunnerSource = _pathRecorded(edit_context.state.pathFileSource, pathAbsDirRunner, edit_context.state.isAbsolute);
      const filesystem::path pathDirRigRecorded = _pathRecorded(edit_context.state.pathDirPackage, pathAbsDirRunner, edit_context.state.isAbsolute);

      // RCI : validate recorded runner paths
      rci--;
      if (
        pathRunnerSource.empty()
        || pathDirRigRecorded.empty()
      )
      {
        vd_printErrorRc(rci, "Failed resolving relative runner paths");
        const TYPE_rc rceReport = to_abs(_rc_writeBuildError({edit_context.state.pathFileBuildError, "runner", rci, "Failed resolving relative runner paths"}));
        if (rceReport != 0)
        {
          return rceReport;
        }
        const TYPE_rc rceRemove = to_abs(_rc_removeBuildPath(edit_context.state.pathDirPackageStaging));
        if (rceRemove != 0)
        {
          return rceRemove;
        }
        return rci;
      }

      edit_context.state.jsonRunner["paths"] =
      {
        { "source", pathRunnerSource.generic_string() },
        { "rig", pathDirRigRecorded.generic_string() }
      };
      edit_context.state.jsonRunner["lastRun"] =
      {
        { "status", "never" },
        { "finishedAt", nullptr }
      };
    }

    // write runner: wire
    const TYPE_json jsonWireInitial =
    {
      {
        "audio",
        {
          { "name", "" },
          { "sps", 0 },
          { "spb", 0 },
          { "meta", TYPE_json::object() }
        }
      },
      {
        "listLindaIn",
        TYPE_json::array(
          {
            {
              { "name", "" },
              { "listIdSrc", TYPE_json::array() },
              { "listIdDest", TYPE_json::array() }
            }
          }
        )
      },
      {
        "listLindaOut",
        TYPE_json::array(
          {
            {
              { "name", "" },
              { "listIdSrc", TYPE_json::array() },
              { "listIdDest", TYPE_json::array() }
            }
          }
        )
      }
    };
    edit_context.state.jsonRunner["wire"] = edit_context.state.hasWire ? edit_context.state.jsonWire : jsonWireInitial;

    edit_context.state.runnerInsidePackage = _is_pathWithin(edit_context.state.pathFileRunnerAbsolute, edit_context.state.pathDirPackageComplete);
    edit_context.state.pathFileRunnerStaging.clear();
    // RCI : preserve the runner backup
    rci--;
    if (edit_context.state.runnerInsidePackage)
    {
      edit_context.state.pathFileRunnerStaging = edit_context.state.pathDirPackageStaging / edit_context.state.pathFileRunnerAbsolute.lexically_relative(edit_context.state.pathDirPackageComplete);
      rce = to_abs(rc_writeFileJson(edit_context.state.pathFileRunnerStaging, edit_context.state.jsonRunner));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed staging runner file");
        const TYPE_rc rceReport = to_abs(_rc_writeBuildError({edit_context.state.pathFileBuildError, "runner", rce, "Failed staging runner file"}));
        if (rceReport != 0)
        {
          return rceReport;
        }
        const TYPE_rc rceRemove = to_abs(_rc_removeBuildPath(edit_context.state.pathDirPackageStaging));
        if (rceRemove != 0)
        {
          return rceRemove;
        }
        return rce;
      }

      filesystem::path pathFileBackup = edit_context.state.pathFileRunnerAbsolute;
      pathFileBackup += ".bak";
      if (filesystem::is_regular_file(pathFileBackup))
      {
        filesystem::path pathFileBackupStaging = edit_context.state.pathFileRunnerStaging;
        pathFileBackupStaging += ".bak";
        error_code ecBackup;
        filesystem::create_directories(pathFileBackupStaging.parent_path(), ecBackup);
        if (!ecBackup)
        {
          filesystem::copy_file(pathFileBackup, pathFileBackupStaging, filesystem::copy_options::overwrite_existing, ecBackup);
        }
        if (ecBackup)
        {
          vd_printErrorRc(rci, "Failed preserving runner backup");
          const TYPE_rc rceReport = to_abs(_rc_writeBuildError({edit_context.state.pathFileBuildError, "backup", rci, "Failed preserving runner backup"}));
          if (rceReport != 0)
          {
            return rceReport;
          }
          const TYPE_rc rceRemove = to_abs(_rc_removeBuildPath(edit_context.state.pathDirPackageStaging));
          if (rceRemove != 0)
          {
            return rceRemove;
          }
          return rci;
        }
      }
    }

    // END
    return 0;
  }

  // [4]
  TYPE_rc narr_rc_publishRigPackage(STRUCT_context& edit_context)
  {
    TYPE_rc rce = 0;
    TYPE_rc rci = 0;

    // Publish the completed package while retaining the previous package for rollback.
    edit_context.state.hadPreviousPackage = filesystem::exists(edit_context.state.pathDirPackageComplete);
    error_code ecPublish;
    // RCI : retain the previous package
    rci--;
    if (edit_context.state.hadPreviousPackage)
    {
      filesystem::rename(edit_context.state.pathDirPackageComplete, edit_context.state.pathDirPackagePrevious, ecPublish);
      if (ecPublish)
      {
        vd_printErrorRc(rci, "Failed retaining previous RIG package");
        const TYPE_rc rceReport = to_abs(_rc_writeBuildError({edit_context.state.pathFileBuildError, "publish", rci, "Failed retaining previous RIG package"}));
        if (rceReport != 0)
        {
          return rceReport;
        }
        const TYPE_rc rceRemove = to_abs(_rc_removeBuildPath(edit_context.state.pathDirPackageStaging));
        if (rceRemove != 0)
        {
          return rceRemove;
        }
        return rci;
      }
    }

    filesystem::rename(edit_context.state.pathDirPackageStaging, edit_context.state.pathDirPackageComplete, ecPublish);
    // RCI : publish the staged package
    rci--;
    if (ecPublish)
    {
      vd_printErrorRc(rci, "Failed publishing RIG package");
      if (edit_context.state.hadPreviousPackage)
      {
        error_code ecRollback;
        filesystem::rename(edit_context.state.pathDirPackagePrevious, edit_context.state.pathDirPackageComplete, ecRollback);
        if (ecRollback)
        {
          vd_printErrorVendor("filesystem", "Failed restoring previous RIG package");
        }
      }
      const TYPE_rc rceReport = to_abs(_rc_writeBuildError({edit_context.state.pathFileBuildError, "publish", rci, "Failed publishing RIG package"}));
      if (rceReport != 0)
      {
        return rceReport;
      }
      const TYPE_rc rceRemove = to_abs(_rc_removeBuildPath(edit_context.state.pathDirPackageStaging));
      if (rceRemove != 0)
      {
        return rceRemove;
      }
      return rci;
    }

    if (!edit_context.state.runnerInsidePackage)
    {
      rce = to_abs(rc_writeFileJsonAtomic(edit_context.state.pathFileRunnerAbsolute, edit_context.state.jsonRunner));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed publishing runner file");
        const TYPE_rc rceRemove = to_abs(_rc_removeBuildPath(edit_context.state.pathDirPackageComplete));
        if (rceRemove != 0)
        {
          return rceRemove;
        }
        if (edit_context.state.hadPreviousPackage)
        {
          error_code ecRollback;
          filesystem::rename(edit_context.state.pathDirPackagePrevious, edit_context.state.pathDirPackageComplete, ecRollback);
          if (ecRollback)
          {
            vd_printErrorVendor("filesystem", "Failed restoring previous RIG package");
          }
        }
        const TYPE_rc rceReport = to_abs(_rc_writeBuildError({edit_context.state.pathFileBuildError, "runner", rce, "Failed publishing runner file"}));
        if (rceReport != 0)
        {
          return rceReport;
        }
        return rce;
      }
    }

    rce = to_abs(_rc_removeBuildPath(edit_context.state.pathDirPackagePrevious));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed removing previous RIG package");
      return rce;
    }


    // END
    return 0;
  }
}

//====----====----====----====----====----====----====----====---- PRIVATE : FDEF

namespace
{
  bool _is_validWire(const TYPE_json& wire)
  {
    if (true)
    {
      const bool isWireStructureValid =
      {
        wire.is_object()
        && wire.contains("audio")
        && wire["audio"].is_object()
        && wire["audio"].contains("name")
        && wire["audio"]["name"].is_string()
        && wire["audio"].contains("sps")
        && is_jsonValueUint32(wire["audio"]["sps"])
        && wire["audio"].contains("spb")
        && is_jsonValueUint32(wire["audio"]["spb"])
        && wire["audio"].contains("meta")
        && wire["audio"]["meta"].is_object()
        && wire.contains("listLindaIn")
        && wire["listLindaIn"].is_array()
        && wire.contains("listLindaOut")
        && wire["listLindaOut"].is_array()
      };
      if (!isWireStructureValid)
      {
        return false;
      }
    }

    for (const TYPE_json* listWire : { &wire["listLindaIn"], &wire["listLindaOut"] })
    {
      for (const TYPE_json& item : *listWire)
      {
        if (true)
        {
          const bool isRouteFormatValid =
          {
            item.is_object()
            && item.contains("name")
            && item["name"].is_string()
            && item.contains("listIdSrc")
            && item["listIdSrc"].is_array()
            && item.contains("listIdDest")
            && item["listIdDest"].is_array()
          };
          if (!isRouteFormatValid)
          {
            return false;
          }
        }

        for (const TYPE_json* listId : { &item["listIdSrc"], &item["listIdDest"] })
        {
          for (const TYPE_json& id : *listId)
          {
            if (!_is_positiveId(id))
            {
              return false;
            }
          }
        }
      }
    }

    return true;
  }

  bool _is_positiveId(const TYPE_json& value)
  {
    if (!is_jsonValueUint32(value))
    {
      return false;
    }
    return value.is_number_unsigned() ? value.get<uint64_t>() > 0 : value.get<int64_t>() > 0;
  }

  bool _is_pathWithin(const filesystem::path& path, const filesystem::path& pathDir)
  {
    const filesystem::path relative = path.lexically_relative(pathDir);
    if (relative.empty() || relative == "." || relative.is_absolute())
    {
      return false;
    }
    return *relative.begin() != "..";
  }

  filesystem::path _pathRecorded(const filesystem::path& path, const filesystem::path& pathBase, bool isAbsolute)
  {
    const filesystem::path pathAbsolute = filesystem::absolute(path).lexically_normal();
    if (isAbsolute)
    {
      return pathAbsolute;
    }

    return pathAbsolute.lexically_relative(filesystem::absolute(pathBase).lexically_normal());
  }

  string _str_packageOwnershipError(const filesystem::path& pathDirPackage)
  {
    if (!filesystem::exists(pathDirPackage))
    {
      return {};
    }
    if (!filesystem::is_directory(pathDirPackage))
    {
      return "RIG package path is not a directory: " + pathDirPackage.generic_string();
    }

    const filesystem::path pathFileMarker = pathDirPackage / linda0::rigPackage::const_nameFileManifest;
    if (filesystem::exists(pathFileMarker))
    {
      TYPE_json jsonManifest;
      const TYPE_rc rceManifest = to_abs(rc_readFileJson(pathFileMarker, jsonManifest));
      if (true)
      {
        const bool isManifestValid =
        {
          rceManifest == 0
          && jsonManifest.is_object()
          && jsonManifest.contains("type")
          && jsonManifest["type"].is_string()
          && jsonManifest["type"] == linda0::rigPackage::const_typeManifest
          && jsonManifest.contains("schemaVersion")
          && is_jsonValueUint32(jsonManifest["schemaVersion"])
          && jsonManifest["schemaVersion"].get<uint32_t>() == linda0::rigPackage::const_schemaVersionCurrent
          && jsonManifest.contains("languageSupport")
          && jsonManifest["languageSupport"].is_string()
          && jsonManifest.contains("semantic")
          && jsonManifest["semantic"].is_string()
          && jsonManifest["semantic"] == linda0::rigPackage::const_nameFileSemantic
          && jsonManifest.contains("error")
          && jsonManifest["error"].is_string()
          && jsonManifest["error"] == linda0::rigPackage::const_pathRelativeFileError
        };
        if (!isManifestValid)
        {
          return "RIG package marker is invalid: " + pathFileMarker.generic_string();
        }
      }
      return {};
    }

    error_code ecEmpty;
    if (filesystem::is_empty(pathDirPackage, ecEmpty) && !ecEmpty)
    {
      return {};
    }

    return "Refusing to replace an unmarked non-empty package directory: " + pathDirPackage.generic_string();
  }

  TYPE_rc _rc_recoverInterruptedBuild(const filesystem::path& pathDirPackage)
  {
    // NULLIFY : rc
    TYPE_rc rci = 0;
    const filesystem::path pathDirParent = pathDirPackage.parent_path();
    if (!filesystem::is_directory(pathDirParent))
    {
      return 0;
    }

    const string prefixPrevious = "." + pathDirPackage.filename().string() + ".previous.";
    vector<filesystem::path> listPrevious;
    error_code ecInspect;
    for (filesystem::directory_iterator it(pathDirParent, ecInspect); !ecInspect && it != filesystem::directory_iterator(); it.increment(ecInspect))
    {
      if (it->path().filename().string().starts_with(prefixPrevious))
      {
        listPrevious.push_back(it->path());
      }
    }
    // RCI : inspect interrupted builds
    rci--;
    if (ecInspect)
    {
      vd_printErrorRc(rci, "Failed inspecting interrupted builds");
      return rci;
    }

    // RCI : reject ambiguous recovery state
    rci--;
    if (!filesystem::exists(pathDirPackage) && listPrevious.size() > 1)
    {
      vd_printErrorRc(rci, "Multiple previous RIG packages require manual recovery");
      return rci;
    }
    // RCI : restore the interrupted package
    rci--;
    if (!filesystem::exists(pathDirPackage) && listPrevious.size() == 1)
    {
      filesystem::rename(listPrevious.front(), pathDirPackage, ecInspect);
      if (ecInspect)
      {
        vd_printErrorRc(rci, "Failed restoring interrupted RIG package");
        return rci;
      }
    }

    // END
    return 0;
  }

  TYPE_rc _rc_removeBuildPath(const filesystem::path& path)
  {
    // NULLIFY : rc
    TYPE_rc rci = 0;
    if (path.empty())
    {
      return 0;
    }

    error_code ecRemove;
    filesystem::remove_all(path, ecRemove);
    // RCI : remove the temporary build path
    rci--;
    if (ecRemove)
    {
      vd_printErrorRc(rci, "Failed removing temporary build path: " + path.string());
      return rci;
    }

    // END
    return 0;
  }

  TYPE_rc _rc_removeInterruptedBuildPaths(const filesystem::path& pathDirPackage)
  {
    // NULLIFY : rc
    TYPE_rc rci = 0;
    const filesystem::path pathDirParent = pathDirPackage.parent_path();
    if (!filesystem::is_directory(pathDirParent))
    {
      return 0;
    }

    const string namePackage = pathDirPackage.filename().string();
    const string prefixBuilding = "." + namePackage + ".building.";
    const string prefixPrevious = "." + namePackage + ".previous.";
    error_code ecInspect;
    for (filesystem::directory_iterator it(pathDirParent, ecInspect); !ecInspect && it != filesystem::directory_iterator(); it.increment(ecInspect))
    {
      const string name = it->path().filename().string();
      if (name.starts_with(prefixBuilding) || name.starts_with(prefixPrevious))
      {
        const TYPE_rc rce = to_abs(_rc_removeBuildPath(it->path()));
        if (rce != 0)
        {
          return rce;
        }
      }
    }
    // RCI : inspect interrupted build paths
    rci--;
    if (ecInspect)
    {
      vd_printErrorRc(rci, "Failed cleaning interrupted build paths");
      return rci;
    }

    // END
    return 0;
  }

  TYPE_rc _rc_writeBuildError(const _REQUEST_rc_writeBuildError& request)
  {
    // NULLIFY : rc
    const TYPE_json jsonError = {{"stage", request.stage}, {"rc", request.rc}, {"message", request.message}};
    const TYPE_rc rce = to_abs(rc_writeFileJsonAtomic(request.pathFile, jsonError));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed writing build error report");
    }

    // END
    return rce;
  }

  TYPE_rc _rc_publishBuildErrors(const filesystem::path& pathDirPackageStaging, const filesystem::path& pathDirPackageComplete, bool& out_published)
  {
    // NULLIFY : rc, out
    out_published = false;
    TYPE_rc rci = 0;
    const filesystem::path pathDirDebugStaging = linda0::rigPackage::pathDir_debug(pathDirPackageStaging);
    if (!filesystem::is_directory(pathDirDebugStaging))
    {
      return 0;
    }

    // Keep structured diagnostics from a failed staged build without publishing partial artifacts.
    error_code ecInspect;
    for (filesystem::directory_iterator it(pathDirDebugStaging, ecInspect); !ecInspect && it != filesystem::directory_iterator(); it.increment(ecInspect))
    {
      if (!it->is_regular_file() || it->path().filename().string() != linda0::rigPackage::const_nameFileError)
      {
        continue;
      }

      TYPE_json jsonError;
      const TYPE_rc rceRead = to_abs(rc_readFileJson(it->path(), jsonError));
      if (rceRead != 0)
      {
        vd_printErrorRc(rceRead, "Failed reading build error report");
        return rceRead;
      }
      const filesystem::path pathFilePublished = linda0::rigPackage::pathFile_error(pathDirPackageComplete);
      const TYPE_rc rceWrite = to_abs(rc_writeFileJsonAtomic(pathFilePublished, jsonError));
      if (rceWrite != 0)
      {
        vd_printErrorRc(rceWrite, "Failed publishing build error report");
        return rceWrite;
      }
      out_published = true;
    }
    // RCI : inspect staged build error reports
    rci--;
    if (ecInspect)
    {
      vd_printErrorRc(rci, "Failed enumerating staged build error reports");
      return rci;
    }

    // END
    return 0;
  }
}
