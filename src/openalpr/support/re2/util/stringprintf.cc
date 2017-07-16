// Copyright 2002 The RE2 Authors.  All Rights Reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "re2/util/util.h"

#include <stdarg.h>

using std::string;

namespace re2 { 

static void StringAppendV(string* dst, const char* format, va_list ap) {
  // First try with a small fixed size buffer
  char space[1024];

  // It's possible for methods that use a va_list to invalidate
  // the data in it upon use.  The fix is to make a copy
  // of the structure before using it and use that copy instead.
  va_list backup_ap;

  #if defined(WIN32)
  backup_ap = ap;
  #else
  va_copy(backup_ap, ap);
  #endif
  int result = vsnprintf(space, sizeof(space), format, backup_ap);
  va_end(backup_ap);

  if ((result >= 0) && (static_cast<unsigned long>(result) < sizeof(space))) {
    // It fit
    dst->append(space, result);
    return;
  }

  // Repeatedly increase buffer size until it fits
  int length = sizeof(space);
  while (true) {
    if (result < 0) {
      // Older behavior: just try doubling the buffer size
      length *= 2;
    } else {
      // We need exactly "result+1" characters
      length = result+1;
    }
    char* buf = new char[length];

    // Restore the va_list before we use it again
	#if defined(WIN32)
	va_list backup_ap = ap;
	#else
    va_copy(backup_ap, ap);
	#endif

	#ifdef WIN32
    result = vsnprintf_s(buf, length, length, format, backup_ap);
	#else
	result = vsnprintf(buf, length, format, backup_ap);
	#endif
    va_end(backup_ap);

    if ((result >= 0) && (result < length)) {
      // It fit
      dst->append(buf, result);
      delete[] buf;
      return;
    }
    delete[] buf;
  }
}

string StringPrintf(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  string result;
  StringAppendV(&result, format, ap);
  va_end(ap);
  return result;
}

void SStringPrintf(string* dst, const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  dst->clear();
  StringAppendV(dst, format, ap);
  va_end(ap);
}

void StringAppendF(string* dst, const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  StringAppendV(dst, format, ap);
  va_end(ap);
}

}  // namespace re2
