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

#include "characterregion.h"
//#include <apr-1.0/apr_poll.h>
#include <math.h>

CharacterRegion::CharacterRegion(Mat img, Config* config)
{
  this->config = config;
  this->debug = config->debugCharRegions;

  this->confidence = 0;



  if (this->debug)
    cout << "Starting CharacterRegion identification" << endl;

  timespec startTime;
  getTime(&startTime);


  charAnalysis = new CharacterAnalysis(img, config);
  charAnalysis->analyze();


  if (this->debug)
  {
    vector<Mat> tempDash;
    for (int z = 0; z < charAnalysis->thresholds.size(); z++)
    {
      Mat tmp(charAnalysis->thresholds[z].size(), charAnalysis->thresholds[z].type());
      charAnalysis->thresholds[z].copyTo(tmp);
      cvtColor(tmp, tmp, CV_GRAY2BGR);

      tempDash.push_back(tmp);
    }


    Mat bestVal(charAnalysis->bestThreshold.size(), charAnalysis->bestThreshold.type());
    charAnalysis->bestThreshold.copyTo(bestVal);
    cvtColor(bestVal, bestVal, CV_GRAY2BGR);

    for (int z = 0; z < charAnalysis->bestContours.size(); z++)
    {
      Scalar dcolor(255,0,0);
      if (charAnalysis->bestCharSegments[z])
	dcolor = Scalar(0,255,0);
      drawContours(bestVal, charAnalysis->bestContours, z, dcolor, 1);
    }
    tempDash.push_back(bestVal);
    displayImage(config, "Character Region Step 1 Thresholds", drawImageDashboard(tempDash, bestVal.type(), 3));
  }



  if (this->debug)
  {
      /*
    Mat img_contours(img_threshold.size(), CV_8U);
    img_threshold.copyTo(img_contours);
    cvtColor(img_contours, img_contours, CV_GRAY2RGB);

    vector<vector<Point> > allowedContours;
    for (int i = 0; i < contours.size(); i++)
    {
	if (charSegments[i])
	  allowedContours.push_back(contours[i]);
    }

    drawContours(img_contours, contours,
	    -1, // draw all contours
	    cv::Scalar(255,0,0), // in blue
	    1); // with a thickness of 1

    drawContours(img_contours, allowedContours,
	    -1, // draw all contours
	    cv::Scalar(0,255,0), // in green
	    1); // with a thickness of 1


    displayImage(config, "Matching Contours", img_contours);
    */
  }


  //charsegments = this->getPossibleCharRegions(img_threshold, allContours, allHierarchy, STARTING_MIN_HEIGHT + (bestFitIndex * HEIGHT_STEP), STARTING_MAX_HEIGHT + (bestFitIndex * HEIGHT_STEP));


  if (charAnalysis->linePolygon.size() > 0)
  {

    int confidenceDrainers = 0;
    int charSegmentCount = charAnalysis->bestCharSegmentsCount;
    if (charSegmentCount == 1)
      confidenceDrainers += 91;
    else if (charSegmentCount < 5)
      confidenceDrainers += (5 - charSegmentCount) * 10;

    int absangle = abs(charAnalysis->topLine.angle);
    if (absangle > 10)
      confidenceDrainers += 91;
    else if (absangle > 1)
      confidenceDrainers += (10 - absangle) * 5;


    if (confidenceDrainers >= 100)
      this->confidence=1;
    else
      this->confidence = 100 - confidenceDrainers;

  }



  if (config->debugTiming)
  {
    timespec endTime;
    getTime(&endTime);
    cout << "Character Region Time: " << diffclock(startTime, endTime) << "ms." << endl;
  }

}

CharacterRegion::~CharacterRegion()
{
  delete(charAnalysis);
}



Mat CharacterRegion::getPlateMask()
{
    return charAnalysis->plateMask;
}

LineSegment CharacterRegion::getTopLine()
{
    return charAnalysis->topLine;
}

LineSegment CharacterRegion::getBottomLine()
{
    return charAnalysis->bottomLine;
}

vector<Point> CharacterRegion::getCharArea()
{
    return charAnalysis->charArea;
}

LineSegment CharacterRegion::getCharBoxTop()
{
    return charAnalysis->charBoxTop;
}

LineSegment CharacterRegion::getCharBoxBottom()
{
    return charAnalysis->charBoxBottom;
}

LineSegment CharacterRegion::getCharBoxLeft()
{
    return charAnalysis->charBoxLeft;
}

LineSegment CharacterRegion::getCharBoxRight()
{
    return charAnalysis->charBoxRight;
}

bool CharacterRegion::thresholdsInverted()
{
    return charAnalysis->thresholdsInverted;
}
