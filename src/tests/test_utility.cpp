/* 
 * File:   utility_tests.cpp
 * Author: mhill
 *
 * Created on October 23, 2014, 10:16 PM
 */

#include <cstdlib>
#include "utility.h"
#include "catch.hpp"

using namespace std;
using namespace cv;
using namespace alpr;



TEST_CASE( "LineSegment Test", "[2d primitives]" ) {

  // Test a flat horizontal line
  LineSegment flat_horizontal(1,1, 21,1);
  REQUIRE( flat_horizontal.angle == 0 );
  REQUIRE( flat_horizontal.slope == 0 );
  REQUIRE( flat_horizontal.length == 20 );
  REQUIRE( flat_horizontal.midpoint().y == 1 );
  REQUIRE( flat_horizontal.midpoint().x == 11 );
  REQUIRE( flat_horizontal.getPointAt(11) == 1 );
  
  // Test distance between points calculation
  REQUIRE( distanceBetweenPoints(Point(10,10), Point(20,20)) == Approx(14.1421) );
  REQUIRE( distanceBetweenPoints(Point(-5,10), Point(20,-12)) == Approx(33.3017) );
  
  int testarray1[] = {1, 2, 3, 3, 4, 5 };
  int testarray2[] = {0, 2, -3, 3, -4, 5 };
  int testarray3[] = { };
  REQUIRE( median(testarray1, 6) == 3 );
  REQUIRE( median(testarray2, 6) == 1 );
  REQUIRE( median(testarray3, 0) == 0 );
}