#pragma once

/*
  UTC timestamp provider  
*/

#include <string>
#include <chrono>
#include <ctime>

#if defined(_WIN32)
  #include <windows.h>
#elif defined(__APPLE__)
  #include <sys/sysctl.h>
  #include <sys/time.h>
#elif defined(__linux__)
  #include <time.h>
#endif

//====----====----====----====----====----====----====----====---- FDEC

static inline std::string str_now();

//====----====----====----====----====----====----====----====---- FDEF

static inline std::string str_now()
{
  auto now = std::chrono::system_clock::now();
  auto t = std::chrono::system_clock::to_time_t(now);

  std::tm tm{};

  #if defined(_WIN32)
    if (gmtime_s(&tm, &t) != 0)
    {
      return {};
    }
  #else
    if (gmtime_r(&t, &tm) == nullptr)
    {
      return {};
    }
  #endif

  char temp[32]{};
  if (std::strftime(temp, sizeof(temp), "%Y-%m-%dT%H:%M:%SZ", &tm) == 0)
  {
    return {};
  }
  return temp;
}