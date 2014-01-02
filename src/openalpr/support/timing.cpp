#include "timing.h"

#ifdef WINDOWS


timespec diff(timespec start, timespec end);

void getTime(timespec* time)
{
    // Do nothing on Windows
}
double diffclock(timespec time1,timespec time2)
{
	// Mock this out for Windows
    return 0;
}

timespec diff(timespec start, timespec end)
{
	// Mock this out for Windows
	return 0;
}

#else


timespec diff(timespec start, timespec end);

void getTime(timespec* time)
{
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, time);
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
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}


#endif
