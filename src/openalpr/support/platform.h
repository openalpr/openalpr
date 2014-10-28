#ifndef OPENALPR_PLATFORM_H
#define OPENALPR_PLATFORM_H

#include <string>
#include <sstream>

#ifdef WINDOWS
	#include <windows.h>
#else
	#include <unistd.h>
#endif

namespace alpr
{

  void sleep_ms(int sleepMs);

  std::string getExeDir();

}

#endif //OPENALPR_PLATFORM_H
