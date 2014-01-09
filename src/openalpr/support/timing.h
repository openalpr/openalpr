#ifndef TIMING_H
#define TIMING_H

#include <iostream>


#ifdef WINDOWS
	// Mock this out for Windows
	#define timespec int
#endif

  void getTime(timespec* time);
  double diffclock(timespec time1,timespec time2);
  
  
#endif