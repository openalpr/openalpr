#ifndef TIMING_H
#define TIMING_H

#include <iostream>
#include <ctime>

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

void getTime(timespec* time);
double diffclock(timespec time1,timespec time2);

int getEpochTime();

#endif
