/*
 * Copyright (c) 2013 New Designs Unlimited, LLC
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

int StateIdentifier::recognize(Mat img, Rect frame, char* stateCode)
{
  Mat croppedImage = Mat(img, frame);

  return this->recognize(croppedImage, stateCode);
}
// Attempts to recognize the plate.  Returns a confidence level.  Updates teh "stateCode" variable
// with the value of the country/state
int StateIdentifier::recognize(Mat img, char* stateCode)
{
  timespec startTime;
  getTime(&startTime);

  cvtColor(img, img, CV_BGR2GRAY);

  resize(img, img, getSizeMaintainingAspect(img, config->stateIdImageWidthPx, config->stateIdimageHeightPx));

  Mat plateImg(img.size(), img.type());
  //plateImg = equalizeBrightness(img);
  img.copyTo(plateImg);

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

  strcpy(stateCode, result.winner.c_str());

  return result.confidence;
}
