#include "03_edge/launcher/linda0/src/_h/02_shell/02_dispatch/_dispatch.h"

#include "03_edge/launcher/linda0/src/__env.h"
#include "01_standard/phase00/error.hpp"
#include "03_edge/launcher/linda0/src/_h/02_shell/00_cli/_cli.h"
#include "03_edge/launcher/linda0/src/_h/02_shell/01_command/_list.h"
#include "03_edge/launcher/linda0/src/_h/02_shell/01_command/_scan.h"
#include "03_edge/launcher/linda0/src/_h/02_shell/01_command/_build.h"
#include "03_edge/launcher/linda0/src/_h/02_shell/01_command/_run.h"

using namespace linda0;
using namespace linda0::cli;


//====----====----====----====----====----====----====----====---- PRIVATE : FDEC

namespace
{
  TYPE_rc _rc_dispatchCommand(const STRUCT_lineParsed& line);
}

//====----====----====----====----====----====----====----====---- PUBLIC : FDEF

namespace linda0::dispatch
{
  TYPE_rc rc_dispatchCommand(int argc, char** argv)
  {
    // NULLIFY : rc
    STRUCT_lineParsed lineParsed;

    TYPE_rc rce = to_abs(rc_parseLine(argc, argv, lineParsed));
    if (rce != 0)
    {
      vd_printErrorRc(rce, "Failed parsing command-line arguments");
      return rce;
    }

    if (lineParsed.command.empty())
    {
      return 0;
    }

    rce = to_abs(_rc_dispatchCommand(lineParsed));

    // END
    return rce;
  }
}

//====----====----====----====----====----====----====----====---- PRIVATE : FDEF

namespace
{
  TYPE_rc _rc_dispatchCommand(const STRUCT_lineParsed& line)
  {
    // NULLIFY : rc
    TYPE_rc rci = 0;

    if (line.command == "list")
    {
      return to_abs(linda0::command::list::rc_commandList(line));
    }
    else if (line.command == "scan")
    {
      return to_abs(linda0::command::scan::rc_commandScan(line));
    }
    else if (line.command == "build")
    {
      return to_abs(linda0::command::build::rc_commandBuild(line));
    }
    else if (line.command == "run")
    {
      return to_abs(linda0::command::run::rc_commandRun(line));
    }

    rci--;
    vd_printErrorRc(rci, "Unknown command '" + line.command + "'. Use '" + ENV_WHOAMI + " --help' for available commands.");
    return rci;
  }
}
