#include "03_edge/launcher/linda0/src/_h/02_shell/01_command/_build.h"

#include "03_edge/launcher/linda0/src/_h/02_shell/01_command/narrative/_narr.commandBuild.h"



//====----====----====----====----====----====----====----====---- PUBLIC : FDEF

namespace linda0::command::build
{
  TYPE_rc rc_commandBuild(const cli::STRUCT_lineParsed& line)
  {
    // NULLIFY : rc
    TYPE_rc rce = 0;

    // NARRATIVE [0]
    narr::commandBuild::STRUCT_context narrativeContext =
    {
      .input =
      {
        .line = line
      }
    };

    // NARRATIVE [1]
    if (true)
    {
      rce = to_abs(narr::commandBuild::narr_rc_prepareBuild(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed preparing build request");
        return rce;
      }
    }

    // NARRATIVE [2]
    if (true)
    {
      rce = to_abs(narr::commandBuild::narr_rc_buildRigByLanguageSupport(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed building RIG artifact");
        return rce;
      }
    }

    // NARRATIVE [3]
    if (true)
    {
      rce = to_abs(narr::commandBuild::narr_rc_stageRunner(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed writing RIG runner");
        return rce;
      }
    }

    // NARRATIVE [4]
    if (true)
    {
      rce = to_abs(narr::commandBuild::narr_rc_publishRigPackage(narrativeContext));
      if (rce != 0)
      {
        vd_printErrorRc(rce, "Failed publishing RIG package");
        return rce;
      }
    }

    // END
    return 0;
  }
}
