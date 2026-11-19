#pragma once

#include <cstdint>
#include <string>

//====----====----====----====----====----====----====----====---- TYPE

namespace lsm
{
  enum class ENUM_lsmKind : uint32_t
  {
    unknown = 0,
    audio = 1,
    language = 2
  };

  struct STRUCT_lsmInfo
  {
    const char* name = "";
    uint32_t versionMajor = 0;
    uint32_t versionMinor = 0;
    uint32_t versionPatch = 0;

    ENUM_lsmKind kind = ENUM_lsmKind::unknown;
    std::string description = "";
  };

  using CALLBACK_vd_getInfo =
    void (*)(STRUCT_lsmInfo& out_info);
}
