#pragma once

#if defined(_WIN32)
  #if defined(LINDA_SUPPORT_BUILD)
    #define LINDA_SUPPORT_EXPORT __declspec(dllexport)
  #else
    #define LINDA_SUPPORT_EXPORT
  #endif
  #define LINDA_SUPPORT_CALL __cdecl
#else
  #define LINDA_SUPPORT_EXPORT
  #define LINDA_SUPPORT_CALL
#endif
