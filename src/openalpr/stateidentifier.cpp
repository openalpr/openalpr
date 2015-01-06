/*
 * Copyright (c) 2015 New Designs Unlimited, LLC
 * Opensource Automated License Plate Recognition [http://www.openalpr.com]
 *
 * This file is part of OpenAlpr.
 *
 * OpenAlpr is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License
 * version 3 as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stateidentifier.h"


using namespace cv;
using namespace std;

namespace alpr
{

  StateIdentifier::StateIdentifier(Config* config)
  {
    this->config = config;

    featureMatcher = new FeatureMatcher(config);

    if (featureMatcher->isLoaded() == false)
    {
      cout << "Can not create detector or descriptor extractor or descriptor matcher of given types" << endl;
      return;
    }

    featureMatcher->loadRecognitionSet(config->country);
  }

  StateIdentifier::~StateIdentifier()
  {
    delete featureMatcher;
  }


  // Attempts to recognize the plate.  Returns a confidence level.  Updates the region code and confidence
  // If region is found, returns true.
  bool StateIdentifier::recognize(PipelineData* pipeline_data)
  {
    timespec startTime;
    getTime(&startTime);

    Mat plateImg = Mat(pipeline_data->grayImg, pipeline_data->regionOfInterest);

    resize(plateImg, plateImg, getSizeMaintainingAspect(plateImg, config->stateIdImageWidthPx, config->stateIdimageHeightPx));


    Mat debugImg(plateImg.size(), plateImg.type());
    plateImg.copyTo(debugImg);
    vector<int> matchesArray(featureMatcher->numTrainingElements());

    RecognitionResult result = featureMatcher->recognize(plateImg, true, &debugImg, true, matchesArray );

    if (this->config->debugStateId)
    {
      displayImage(config, "State Identifier1", plateImg);
      displayImage(config, "State Identifier", debugImg);
      cout << result.haswinner << " : " << result.confidence << " : " << result.winner << endl;
    }

    if (config->debugTiming)
    {
      timespec endTime;
      getTime(&endTime);
      cout << "State Identification Time: " << diffclock(startTime, endTime) << "ms." << endl;
    }

    if (result.haswinner == false)
      return 0;

    pipeline_data->region_code = result.winner;
    pipeline_data->region_confidence = result.confidence;

    if (result.confidence >= 10)
      return true;

    return false;
  }

}