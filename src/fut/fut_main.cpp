/*****************************************************************************
  FALCON - The Falcon Programming Language
  FILE: fut_main.cpp

  Default main() function for unit tests
  -------------------------------------------------------------------
  Author: Giancarlo Niccolai
  Begin : Sat, 13 Jan 2018 21:24:04 +0000
  Touch : Tue, 16 Jan 2018 22:29:26 +0000

  -------------------------------------------------------------------
  (C) Copyright 2018 The Falcon Programming Language
  Released under Apache 2.0 License.
******************************************************************************/

#include <falcon/fut/unittest.h>
#include <cassert>

extern "C" {
	int main(int argc, char* argv[]) {
		assert(argc > 0);
		return ::falcon::testing::UnitTest::singleton()->main(argc, argv);
	}
}

/* end of fut_main.cpp */

