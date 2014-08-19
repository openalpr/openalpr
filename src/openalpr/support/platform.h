#ifndef OPENALPR_PLATFORM_H
#define OPENALPR_PLATFORM_H

#ifdef WINDOWS
	#include <windows.h>
#else
	#include <unistd.h>
#endif


void sleep_ms(int sleepMs);


#endif //OPENALPR_PLATFORM_H
