#pragma once

#include "01_standard/phase00/error.hpp"
#include "03_edge/launcher/linda0/src/_h/02_shell/00_cli/_cli.h"

//====----====----====----====----====----====----====----====---- PUBLIC : FDEC

namespace linda0::command::scan
{
  TYPE_rc rc_commandScan(const cli::STRUCT_lineParsed& line);
}

