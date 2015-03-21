#ifndef TIMING_H
#define TIMING_H

#include <iostream>
#include <ctime>
#include <stdint.h>

#ifdef WINDOWS
    // Import windows only stuff
#else
    #include <sys/time.h>
#endif

// Support for OS X
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

// Support for Windows
#ifdef WINDOWS
#include <windows.h>

#define timespec timeval
#endif

namespace alpr
{

  void getTimeMonotonic(timespec* time);
  int64_t getTimeMonotonicMs();
  
  double diffclock(timespec time1,timespec time2);

  int64_t getEpochTimeMs();

}

#endif
