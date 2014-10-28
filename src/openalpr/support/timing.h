#ifndef TIMING_H
#define TIMING_H

#include <iostream>
#include <ctime>

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

  void getTime(timespec* time);
  double diffclock(timespec time1,timespec time2);

  long getEpochTime();

}

#endif
