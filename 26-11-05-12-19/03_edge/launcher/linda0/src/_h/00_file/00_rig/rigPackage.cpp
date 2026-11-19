#include "03_edge/launcher/linda0/src/_h/00_file/00_rig/_rigPackage.h"

#include "01_standard/phase01/ioFile.hpp"

using namespace std;

//====----====----====----====----====----====----====----====---- PUBLIC : FDEF

namespace linda0::rigPackage
{
  std::filesystem::path pathFile_manifest(const std::filesystem::path& pathDirPackage)
  {
    return pathDirPackage / const_nameFileManifest;
  }

  std::filesystem::path pathFile_semantic(const std::filesystem::path& pathDirPackage)
  {
    return pathDirPackage / const_nameFileSemantic;
  }

  std::filesystem::path pathDir_debug(const std::filesystem::path& pathDirPackage)
  {
    return pathDirPackage / const_nameDirDebug;
  }

  std::filesystem::path pathDir_payload(const std::filesystem::path& pathDirPackage)
  {
    return pathDirPackage / const_nameDirPayload;
  }

  std::filesystem::path pathFile_error(const std::filesystem::path& pathDirPackage)
  {
    return pathDirPackage / const_nameDirDebug / const_nameFileError;
  }

  TYPE_rc rc_writeManifest(const std::filesystem::path& pathDirPackage, const std::string& nameLanguageSupport)
  {
    // NULLIFY : rc, out
    TYPE_rc rce = 0;

    // RCE : write a file
    if (true)
    {
      TYPE_json jsonManifest =
      {
        {"type", const_typeManifest},
        {"schemaVersion", const_schemaVersionCurrent},
        {"languageSupport", nameLanguageSupport},
        {"semantic", const_nameFileSemantic},
        {"error", const_pathRelativeFileError}
      };

      rce = to_abs(rc_writeFileJson(pathDirPackage / const_nameFileManifest, jsonManifest));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed writing RIG package manifest");
        return rce;
      }
    }    

    // END
    return 0;
  }
}
