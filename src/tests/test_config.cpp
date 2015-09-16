/* 
 * File:   utility_tests.cpp
 * Author: mhill
 *
 * Created on October 23, 2014, 10:16 PM
 */

#include <cstdlib>
#include "catch.hpp"
#include "config.h"

using namespace std;
using namespace alpr;

Config get_config(std::string countries)
{
  Config config(countries, "", "");
  return config;
}

TEST_CASE( "Loading Countries", "[Config]" ) {

  REQUIRE( get_config("us,eu").loaded_countries.size() == 2 );

  REQUIRE( get_config("us").loaded_countries.size() == 1 );

  REQUIRE( get_config("eu").loaded_countries.size() == 1 );

  REQUIRE( get_config("us, eu").loaded_countries.size() == 2 );

  REQUIRE( get_config("us, eu, au, kr").loaded_countries.size() == 4 );

  REQUIRE( get_config("us , eu").loaded_countries.size() == 2 );

  REQUIRE( get_config("us,eu,").loaded_countries.size() == 2 );

  REQUIRE( get_config(",,us,eu,").loaded_countries.size() == 2 );

}


//TEST_CASE( "Modifying Countries", "[Config]" )
//{
//  Config config("us,eu", "/etc/openalpr/openalpr.conf", "/usr/share/openalpr/runtime_data/");
//
//  REQUIRE(config.ocrLanguage == "lus");
//
//  config.setCountry("eu");
//  REQUIRE(config.ocrLanguage == "leu");
//
//  config.setCountry("us");
//  REQUIRE(config.ocrLanguage == "lus");
//}