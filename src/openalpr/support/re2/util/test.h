// Copyright 2009 The RE2 Authors.  All Rights Reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef RE2_UTIL_TEST_H__
#define RE2_UTIL_TEST_H__

#include "util/util.h"
#include "util/flags.h"

#define TEST(x, y) \
	void x##y(void); \
	TestRegisterer r##x##y(x##y, # x "." # y); \
	void x##y(void)

void RegisterTest(void (*)(void), const char*);

class TestRegisterer {
 public:
  TestRegisterer(void (*fn)(void), const char *s) {
    RegisterTest(fn, s);
  }
};

// TODO(rsc): Do a better job.
#define EXPECT_EQ CHECK_EQ
#define EXPECT_TRUE CHECK
#define EXPECT_LT CHECK_LT
#define EXPECT_GT CHECK_GT
#define EXPECT_LE CHECK_LE
#define EXPECT_GE CHECK_GE
#define EXPECT_FALSE(x) CHECK(!(x))

const bool UsingMallocCounter = false;
namespace testing {
class MallocCounter {
 public:
  MallocCounter(int x) { } 
  static const int THIS_THREAD_ONLY = 0;
  long long HeapGrowth() { return 0; }
  long long PeakHeapGrowth() { return 0; }
  void Reset() { }
};
}  // namespace testing

namespace re2 {
int64 VirtualProcessSize();
} // namespace re2

#endif  // RE2_UTIL_TEST_H__
