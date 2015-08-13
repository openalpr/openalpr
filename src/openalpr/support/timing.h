#ifndef OPENALPR_TIMING_H
#define OPENALPR_TIMING_H

#include <iostream>
#include <ctime>
#include <stdint.h>

#if WINDOWS
struct timespec
{
    time_t tv_sec;  // Seconds - >= 0
    long   tv_usec; // Nanoseconds - [0, 999999999]
};
#endif

#ifdef WINDOWS
    // Import windows only stuff
	#include <windows.h>

#else
    #include <sys/time.h>
#endif

// Support for OS X
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

namespace alpr
{

  void getTimeMonotonic(timespec* time);
  int64_t getTimeMonotonicMs();
  
  double diffclock(timespec time1,timespec time2);

  int64_t getEpochTimeMs();

}

#endif
