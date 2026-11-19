#include "03_edge/language/linda.js/src/_h/00_file/_lindaJsFile.h"

using namespace std;

//====----====----====----====----====----====----====----====---- FDEF

namespace lindaJs::file
{
  filesystem::path pathDir_payload(const filesystem::path& pathDirPackage)
  {
    return pathDirPackage / const_nameDirPayload;
  }

  filesystem::path pathDir_debug(const filesystem::path& pathDirPackage)
  {
    return pathDirPackage / const_nameDirDebug;
  }

  filesystem::path pathFile_manifest(const filesystem::path& pathDirPackage)
  {
    return pathDirPackage / const_nameFileManifest;
  }

  filesystem::path pathFile_semantic(const filesystem::path& pathDirPackage)
  {
    return pathDirPackage / const_nameFileSemantic;
  }

  filesystem::path pathFile_error(const filesystem::path& pathDirPackage)
  {
    return pathDir_debug(pathDirPackage) / const_nameFileError;
  }

  filesystem::path pathFile_payloadCore(const filesystem::path& pathDirPackage)
  {
    return pathDir_payload(pathDirPackage) / const_nameFilePayloadCore;
  }

  filesystem::path pathFile_sourceMap(const filesystem::path& pathDirPackage)
  {
    return pathDir_debug(pathDirPackage) / const_nameFileSourceMap;
  }
}
