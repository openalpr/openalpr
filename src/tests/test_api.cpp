/* 
 * File:   runtests.cpp
 * Author: mhill
 *
 * Created on October 22, 2014, 11:11 PM
 */

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <cstdlib>
#include "catch.hpp"
#include "alpr.h"
#include "support/timing.h"


using namespace std;
using namespace alpr;


TEST_CASE( "JSON Serialization/Deserialization", "[json]" ) {
  
  AlprResults origResults;
  origResults.epoch_time = getEpochTime();
  origResults.img_width = 640;
  origResults.img_height = 480;
  origResults.total_processing_time_ms = 100;
  origResults.regionsOfInterest.push_back(AlprRegionOfInterest(0,0,100,200));
  origResults.regionsOfInterest.push_back(AlprRegionOfInterest(259,260,50,150));
  
  AlprPlateResult apr;
  for (int i = 0; i < 3; i++)
  {
    AlprPlate ap;
    ap.characters = "abc";
    ap.matches_template = i%2;
    ap.overall_confidence = i * 10;
    apr.topNPlates.push_back(ap);
  }
  apr.bestPlate = apr.topNPlates[0];
  for (int i = 0; i < 4; i++)
  {
    AlprCoordinate ac;
    ac.x = i;
    ac.y = i;
    apr.plate_points[i] = ac;
  }
  
  apr.processing_time_ms = 30;
  apr.requested_topn = 10;
  apr.region = "mo";
  apr.regionConfidence = 80;
          
  origResults.plates.push_back(apr);
  
  
  std::string resultsJson = Alpr::toJson(origResults);
  AlprResults roundTrip = Alpr::fromJson(resultsJson);
  
  REQUIRE( roundTrip.epoch_time == origResults.epoch_time );
  REQUIRE( roundTrip.img_width == origResults.img_width );
  REQUIRE( roundTrip.img_height == origResults.img_height );
  REQUIRE( roundTrip.total_processing_time_ms == origResults.total_processing_time_ms );
  
  REQUIRE( roundTrip.regionsOfInterest.size() == origResults.regionsOfInterest.size() );
  for (int i = 0; i < roundTrip.regionsOfInterest.size(); i++)
  {
    REQUIRE( roundTrip.regionsOfInterest[i].x == origResults.regionsOfInterest[i].x);
    REQUIRE( roundTrip.regionsOfInterest[i].y == origResults.regionsOfInterest[i].y);
    REQUIRE( roundTrip.regionsOfInterest[i].width == origResults.regionsOfInterest[i].width);
    REQUIRE( roundTrip.regionsOfInterest[i].height == origResults.regionsOfInterest[i].height);
  }
  
  REQUIRE( roundTrip.plates.size() == origResults.plates.size() );
  for (int i = 0; i < roundTrip.plates.size(); i++)
  {
    REQUIRE( roundTrip.plates[i].processing_time_ms == origResults.plates[i].processing_time_ms);
    REQUIRE( roundTrip.plates[i].region == origResults.plates[i].region);
    REQUIRE( roundTrip.plates[i].regionConfidence == origResults.plates[i].regionConfidence);
    REQUIRE( roundTrip.plates[i].requested_topn == origResults.plates[i].requested_topn);
    
    REQUIRE( roundTrip.plates[i].bestPlate.characters == origResults.plates[i].bestPlate.characters);
    REQUIRE( roundTrip.plates[i].bestPlate.matches_template == origResults.plates[i].bestPlate.matches_template);
    REQUIRE( roundTrip.plates[i].bestPlate.overall_confidence == origResults.plates[i].bestPlate.overall_confidence);
    
    REQUIRE( roundTrip.plates[i].topNPlates.size() == origResults.plates[i].topNPlates.size());
    for (int j = 0; j < roundTrip.plates[i].topNPlates.size(); j++)
    {
      REQUIRE( roundTrip.plates[i].topNPlates[j].characters == origResults.plates[i].topNPlates[j].characters);
      REQUIRE( roundTrip.plates[i].topNPlates[j].matches_template == origResults.plates[i].topNPlates[j].matches_template);
      REQUIRE( roundTrip.plates[i].topNPlates[j].overall_confidence == origResults.plates[i].topNPlates[j].overall_confidence);
    }
            
  }
  
}
