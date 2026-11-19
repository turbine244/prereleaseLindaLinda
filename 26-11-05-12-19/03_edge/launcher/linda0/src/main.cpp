#ifdef _WIN32
#include <Windows.h>
#endif

#include "03_edge/launcher/linda0/src/_h/02_shell/02_dispatch/_dispatch.h"
#include "01_standard/phase00/error.hpp"

//====----====----====----====----====----====----====----====---- PUBLIC : FDEF

int main(int argc, char** argv)
{
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif
  const TYPE_rc rc = linda0::dispatch::rc_dispatchCommand(argc, argv);
  return static_cast<int>(to_abs(rc));
}
