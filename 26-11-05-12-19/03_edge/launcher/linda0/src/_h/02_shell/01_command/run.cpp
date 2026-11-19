#include "03_edge/launcher/linda0/src/_h/02_shell/01_command/_run.h"

#include "03_edge/launcher/linda0/src/_h/02_shell/01_command/narrative/_narr.commandRun.h"



//====----====----====----====----====----====----====----====---- PUBLIC : FDEF

namespace linda0::command::run
{
  TYPE_rc rc_commandRun(const cli::STRUCT_lineParsed& line)
  {
    // NULLIFY : rc
    TYPE_rc rce = 0;

    // NARRATIVE [0]
    narr::commandRun::STRUCT_context narrativeContext =
    {
      .input =
      {
        .line = line
      }
    };

    // NARRATIVE [1]
    if (true)
    {
      rce = to_abs(narr::commandRun::narr_rc_prepareRun(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed preparing runner");
        return rce;
      }
    }

    // NARRATIVE [2]
    if (true)
    {
      rce = to_abs(narr::commandRun::narr_rc_executeRun(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed executing GIG runtime");
        return rce;
      }
    }

    // NARRATIVE [3]
    if (true)
    {
      rce = to_abs(narr::commandRun::narr_rc_recordRunResult(narrativeContext));
      // END
      return rce;
    }
  }
}
