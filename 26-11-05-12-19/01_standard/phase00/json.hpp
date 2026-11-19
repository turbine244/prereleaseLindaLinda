#pragma once

/*
  Project-wide JSON type & utilities

  vendored backend:
    https://github.com/nlohmann/json
*/

#include <cstdint>
#include <limits>

#include "00_vendored/nlohmann/json.hpp"

//====----====----====----====----====----====----====----====---- TYPE

using TYPE_json = nlohmann::ordered_json;

//====----====----====----====----====----====----====----====---- FDEC

static inline bool is_jsonValueUint32(const TYPE_json& value);

//====----====----====----====----====----====----====----====---- FDEF

static inline bool is_jsonValueUint32(const TYPE_json& value)
{
  if (value.is_number_unsigned())
  {
    const uint64_t raw = value.get<uint64_t>();
    if (raw <= (std::numeric_limits<uint32_t>::max)())
    {
      return true;
    }
    return false;
  }
  if (value.is_number_integer())
  {
    const int64_t raw = value.get<int64_t>();
    if (raw >= 0 && static_cast<uint64_t>(raw) <= (std::numeric_limits<uint32_t>::max)())
    {
      return true;
    }
  }
  return false;
}