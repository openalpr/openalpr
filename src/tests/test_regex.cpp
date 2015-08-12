#include <cstdlib>
#include "utility.h"
#include "catch.hpp"
#include "postprocess/regexrule.h"

using namespace std;
using namespace cv;
using namespace alpr;



TEST_CASE( "ASCII tests", "[Regex]" ) {

  RegexRule rule1("us", "@@@####", "[A-Za-z]", "[0-9]");
  
  REQUIRE( rule1.match("123ABCD") == false);
  REQUIRE( rule1.match("123ABC") == false);
  REQUIRE( rule1.match("23ABCD") == false);
  REQUIRE( rule1.match("ABC123") == false);
  REQUIRE( rule1.match("BC1234") == false);
  REQUIRE( rule1.match("ABC12345") == false);
  REQUIRE( rule1.match("AABC1234") == false);
  REQUIRE( rule1.match("ABCD234") == false);
  REQUIRE( rule1.match("AB11234") == false);
  REQUIRE( rule1.match("ABC-234") == false);
  
  REQUIRE( rule1.match("ABC1234") == true);
  REQUIRE( rule1.match("AAA1111") == true);
  REQUIRE( rule1.match("zzz1111") == true);
  
  RegexRule rule2("us", "[ABC]@@####", "[A-Z]", "[0-9]");
  
  REQUIRE( rule2.match("ZBC1234") == false);
  REQUIRE( rule2.match("DBC1234") == false);
  REQUIRE( rule2.match("1BC1234") == false);
  
  REQUIRE( rule2.match("ABC1234") == true);
  REQUIRE( rule2.match("BAA1111") == true);
  REQUIRE( rule2.match("CAA1111") == true);
  
  RegexRule rule3("us", "[A]@@###[12]", "[A-Z]", "[0-9]");
  
  REQUIRE( rule3.match("ZBC1234") == false);
  REQUIRE( rule3.match("ZBC1231") == false);
  REQUIRE( rule3.match("ABC1234") == false);
  
  REQUIRE( rule3.match("ABC1231") == true);
  REQUIRE( rule3.match("ABC1232") == true);
  
  
  RegexRule rule4("us", "[A-C][E-G]1111", "[A-Z]", "[0-9]");
  
  REQUIRE( rule4.match("DG1111") == false);
  REQUIRE( rule4.match("AD1111") == false);
  REQUIRE( rule4.match("AF1112") == false);
  
  REQUIRE( rule4.match("AF1111") == true);
  REQUIRE( rule4.match("BF1111") == true);
  REQUIRE( rule4.match("CF1111") == true);
  REQUIRE( rule4.match("BE1111") == true);
  REQUIRE( rule4.match("BF1111") == true);
  REQUIRE( rule4.match("BG1111") == true);
  
  RegexRule rule5("us", "\\d\\d\\D\\D", "[A-Z]", "[0-9]");
  REQUIRE( rule5.match("AA11") == false);
  REQUIRE( rule5.match("11AA") == true);
  
}

TEST_CASE( "Unicode tests", "[Regex]" ) {

  RegexRule rule1("us", "@@@####", "\\pL", "\\pN");
  
  REQUIRE( rule1.match("123与与与下") == false);
  REQUIRE( rule1.match("与万12345") == false);
  REQUIRE( rule1.match("与万口口234") == false);
  REQUIRE( rule1.match("与万口abcd") == false);
  
  REQUIRE( rule1.match("与万口1234") == true);
  
  RegexRule rule2("us", "[십팔]@@####", "\\pL", "\\pN");
  
  REQUIRE( rule2.match("123与与与下") == false);
  REQUIRE( rule2.match("与万12345") == false);
  REQUIRE( rule2.match("与万口口234") == false);
  REQUIRE( rule2.match("与万口abcd") == false);
  REQUIRE( rule2.match("与万口1234") == false);
  REQUIRE( rule2.match("与팔십1234") == false);
  
  REQUIRE( rule2.match("십万口1234") == true);
  REQUIRE( rule2.match("팔万口1234") == true);
}


TEST_CASE( "Invalid tests", "[Regex]" ) {
  RegexRule rule1("us", "[A@@####", "\\pL", "\\pN");
  
  REQUIRE( rule1.match("123ABCD") == false);
  REQUIRE( rule1.match("123ABC") == false);
  REQUIRE( rule1.match("23ABCD") == false);
  
  REQUIRE( rule1.match("ABC1234") == false);
  REQUIRE( rule1.match("AAA1111") == false);
  REQUIRE( rule1.match("zzz1111") == false);
  
  RegexRule rule2("us", "A####]", "\\pL", "\\pN");
  REQUIRE( rule2.match("A1234") == false);
}