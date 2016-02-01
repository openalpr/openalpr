#include "timing.h"

namespace alpr
{

  timespec diff(timespec start, timespec end);

  #ifdef WINDOWS

  // Windows timing code
  LARGE_INTEGER getFILETIMEoffset()
  {
    SYSTEMTIME s;
    FILETIME f;
    LARGE_INTEGER t;

    s.wYear = 1970;
    s.wMonth = 1;
    s.wDay = 1;
    s.wHour = 0;
    s.wMinute = 0;
    s.wSecond = 0;
    s.wMilliseconds = 0;
    SystemTimeToFileTime(&s, &f);
    t.QuadPart = f.dwHighDateTime;
    t.QuadPart <<= 32;
    t.QuadPart |= f.dwLowDateTime;
    return (t);
  }

  int clock_gettime(int X, timespec *tv)
  {
    LARGE_INTEGER           t;
    FILETIME            f;
    double                  microseconds;
    static LARGE_INTEGER    offset;
    static double           frequencyToMicroseconds;
    static int              initialized = 0;
    static BOOL             usePerformanceCounter = 0;

    if (!initialized)
    {
      LARGE_INTEGER performanceFrequency;
      initialized = 1;
      usePerformanceCounter = QueryPerformanceFrequency(&performanceFrequency);
      if (usePerformanceCounter)
      {
        QueryPerformanceCounter(&offset);
        frequencyToMicroseconds = (double)performanceFrequency.QuadPart / 1000000.;
      }
      else
      {
        offset = getFILETIMEoffset();
        frequencyToMicroseconds = 10.;
      }
    }
    if (usePerformanceCounter) QueryPerformanceCounter(&t);
    else
    {
      GetSystemTimeAsFileTime(&f);
      t.QuadPart = f.dwHighDateTime;
      t.QuadPart <<= 32;
      t.QuadPart |= f.dwLowDateTime;
    }

    t.QuadPart -= offset.QuadPart;
    microseconds = (double)t.QuadPart / frequencyToMicroseconds;
    t.QuadPart = microseconds;
    tv->tv_sec = t.QuadPart / 1000000;
    tv->tv_nsec = (t.QuadPart % 1000000)*1000;
    return (0);
  }

  void getTimeMonotonic(timespec* time)

  {
    clock_gettime(0, time);
  }
  
  int64_t getTimeMonotonicMs()
  {
    timespec time;
    getTimeMonotonic(&time);

    timespec time_start;
    time_start.tv_sec = 0;
    time_start.tv_nsec = 0;

    return diffclock(time_start, time);
  }
  
  
  double diffclock(timespec time1,timespec time2)
  {
    timespec delta = diff(time1,time2);
    double milliseconds = (delta.tv_sec * 1000) +  (((double) delta.tv_nsec) / 1000000.0);

    return milliseconds;
  }

  timespec diff(timespec start, timespec end)
  {
    timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0)
    {
      temp.tv_sec = end.tv_sec-start.tv_sec-1;
      temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    }
    else
    {
      temp.tv_sec = end.tv_sec-start.tv_sec;
      temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
  }


  int64_t getEpochTimeMs()
  {
    return std::time(0) * 1000;
  } 

  #else

  void _getTime(bool realtime, timespec* time)
  {
    #if defined(__APPLE__) && defined(__MACH__) // OS X does not have clock_gettime, use clock_get_time
      clock_serv_t cclock;
      mach_timespec_t mts;
      host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
      clock_get_time(cclock, &mts);
      mach_port_deallocate(mach_task_self(), cclock);
      time->tv_sec = mts.tv_sec;
      time->tv_nsec = mts.tv_nsec;
    #else
      if (realtime)
        clock_gettime(CLOCK_REALTIME, time);
      else
        clock_gettime(CLOCK_MONOTONIC, time);
    #endif
  }
  
  // Returns a monotonic clock time unaffected by time changes (e.g., NTP)
  // Useful for interval comparisons
  void getTimeMonotonic(timespec* time)
  {
    _getTime(false, time);
  }
  
  int64_t getTimeMonotonicMs()
  {
    timespec time;
    getTimeMonotonic(&time);

    timespec time_start;
    time_start.tv_sec = 0;
    time_start.tv_nsec = 0;

    return diffclock(time_start, time);
  }

  double diffclock(timespec time1,timespec time2)
  {
    timespec delta = diff(time1,time2);
    double milliseconds = (((double) delta.tv_sec) * 1000.0) +  (((double) delta.tv_nsec) / 1000000.0);

    return milliseconds;
  }

  timespec diff(timespec start, timespec end)
  {
    timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0)
    {
      temp.tv_sec = end.tv_sec-start.tv_sec-1;
      temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    }
    else
    {
      temp.tv_sec = end.tv_sec-start.tv_sec;
      temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
  }


  // Returns wall clock time since Unix epoch (Jan 1, 1970)
  int64_t getEpochTimeMs()
  {
    timespec time;
    _getTime(true, &time);

    timespec epoch_start;
    epoch_start.tv_sec = 0;
    epoch_start.tv_nsec = 0;

    return diffclock(epoch_start, time);

  } 

  #endif

}


