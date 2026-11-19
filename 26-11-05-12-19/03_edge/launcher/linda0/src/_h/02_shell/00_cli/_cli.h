#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>
#include "01_standard/phase00/error.hpp"

//====----====----====----====----====----====----====----====---- PUBLIC : TYPE

namespace linda0::cli
{
  struct STRUCT_lineParsed
  {
    std::string command;
    std::string subCommand;

    std::unordered_map<std::string, std::string> mapOperand;
    std::unordered_map<std::string, std::string> mapOptionValue;
    std::unordered_set<std::string> setOptionFlag;
  };
}

//====----====----====----====----====----====----====----====---- PUBLIC : FDEC

namespace linda0::cli
{
  TYPE_rc rc_parseLine(int argc, char** argv, STRUCT_lineParsed& out_lineParsed);
}
