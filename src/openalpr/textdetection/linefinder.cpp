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

#include <opencv2/core/core.hpp>

#include "linefinder.h"
#include "utility.h"
#include "pipeline_data.h"

using namespace std;
using namespace cv;

namespace alpr
{

  LineFinder::LineFinder(PipelineData* pipeline_data) {
    this->pipeline_data = pipeline_data;
  }

  LineFinder::~LineFinder() {
  }

  vector<vector<Point> > LineFinder::findLines(Mat image, const TextContours contours)
  {
    const float MIN_AREA_TO_IGNORE = 0.65;

    vector<vector<Point> > linesFound;

    cvtColor(image, image, CV_GRAY2BGR);

    vector<CharPointInfo> charPoints;

    for (unsigned int i = 0; i < contours.contours.size(); i++)
    {
      if (contours.goodIndices[i] == false)
        continue;

      charPoints.push_back( CharPointInfo(contours.contours[i], i) );
    }

    vector<Point> bestLine = getBestLine(contours, charPoints);

    if (bestLine.size() > 0)
      linesFound.push_back(bestLine);

    if (pipeline_data->isMultiline)
    {
      // we have a two-line plate.  Find the next best line, removing the tops/bottoms from before.
      // Create a mask from the bestLine area, and remove all contours with tops that fall inside of it.

      vector<CharPointInfo> remainingPoints;
      for (unsigned int i = 0; i < charPoints.size(); i++)
      {
        Mat mask = Mat::zeros(Size(contours.width, contours.height), CV_8U);
        fillConvexPoly(mask, bestLine.data(), bestLine.size(), Scalar(255,255,255));

        float percentInside = getContourAreaPercentInsideMask(mask, contours.contours, contours.hierarchy, charPoints[i].contourIndex);

        if (percentInside < MIN_AREA_TO_IGNORE)
        {
          remainingPoints.push_back(charPoints[i]);
        }
      }

      vector<Point> nextBestLine = getBestLine(contours, remainingPoints);

      if (nextBestLine.size() > 0)
        linesFound.push_back(nextBestLine);
    }


    return linesFound;
  }


  // Returns a polygon "stripe" across the width of the character region.  The lines are voted and the polygon starts at 0 and extends to image width
  vector<Point> LineFinder::getBestLine(const TextContours contours, vector<CharPointInfo> charPoints)
  {
    vector<Point> bestStripe;

    // Find the best fit line segment that is parallel with the most char segments
    if (charPoints.size() <= 1)
    {
      // Maybe do something about this later, for now let's just ignore
      return bestStripe;
    }


    vector<int> charheights;
    for (unsigned int i = 0; i < charPoints.size(); i++)
      charheights.push_back(charPoints[i].boundingBox.height);
    float medianCharHeight = median(charheights.data(), charheights.size());



    vector<LineSegment> topLines;
    vector<LineSegment> bottomLines;
    // Iterate through each possible char and find all possible lines for the top and bottom of each char segment
    for (unsigned int i = 0; i < charPoints.size() - 1; i++)
    {
      for (unsigned int k = i+1; k < charPoints.size(); k++)
      {

        int leftCPIndex, rightCPIndex;
        if (charPoints[i].top.x < charPoints[k].top.x)
        {
          leftCPIndex = i;
          rightCPIndex = k;
        }
        else
        {
          leftCPIndex = k;
          rightCPIndex = i;
        }


        LineSegment top(charPoints[leftCPIndex].top, charPoints[rightCPIndex].top);
        LineSegment bottom(charPoints[leftCPIndex].bottom, charPoints[rightCPIndex].bottom);


        // Only allow lines that have a sane angle
  //        if (abs(top.angle) <= pipeline_data->config->maxPlateAngleDegrees &&
  //            abs(bottom.angle) <= pipeline_data->config->maxPlateAngleDegrees)
  //        {
  //          topLines.push_back(top);
  //          bottomLines.push_back(bottom);
  //        }

        LineSegment parallelBot = top.getParallelLine(medianCharHeight * -1);
        LineSegment parallelTop = bottom.getParallelLine(medianCharHeight);

        // Only allow lines that have a sane angle
        if (abs(top.angle) <= pipeline_data->config->maxPlateAngleDegrees &&
            abs(parallelBot.angle) <= pipeline_data->config->maxPlateAngleDegrees)
        {
          topLines.push_back(top);
          bottomLines.push_back(parallelBot);
        }

        // Only allow lines that have a sane angle
        if (abs(parallelTop.angle) <= pipeline_data->config->maxPlateAngleDegrees &&
            abs(bottom.angle) <= pipeline_data->config->maxPlateAngleDegrees)
        {
          topLines.push_back(parallelTop);
          bottomLines.push_back(bottom);
        }
      }
    }

    int bestScoreIndex = 0;
    int bestScore = -1;
    int bestScoreDistance = -1; // Line segment distance is used as a tie breaker

    // Now, among all possible lines, find the one that is the best fit
    for (unsigned int i = 0; i < topLines.size(); i++)
    {
      float SCORING_MIN_THRESHOLD = 0.97;
      float SCORING_MAX_THRESHOLD = 1.03;

      int curScore = 0;
      for (unsigned int charidx = 0; charidx < charPoints.size(); charidx++)
      {
        float topYPos = topLines[i].getPointAt(charPoints[charidx].top.x);
        float botYPos = bottomLines[i].getPointAt(charPoints[charidx].bottom.x);

        float minTop = charPoints[charidx].top.y * SCORING_MIN_THRESHOLD;
        float maxTop = charPoints[charidx].top.y * SCORING_MAX_THRESHOLD;
        float minBot = (charPoints[charidx].bottom.y) * SCORING_MIN_THRESHOLD;
        float maxBot = (charPoints[charidx].bottom.y) * SCORING_MAX_THRESHOLD;
        if ( (topYPos >= minTop && topYPos <= maxTop) &&
             (botYPos >= minBot && botYPos <= maxBot))
        {
          curScore++;
        }

        //cout << "Slope: " << topslope << " yPos: " << topYPos << endl;
        //drawAndWait(&tempImg);
      }

      // Tie goes to the one with longer line segments
      if ((curScore > bestScore) ||
          (curScore == bestScore && topLines[i].length > bestScoreDistance))
      {
        bestScore = curScore;
        bestScoreIndex = i;
        // Just use x distance for now
        bestScoreDistance = topLines[i].length;
      }
    }

    if (bestScore < 0)
      return bestStripe;

    if (pipeline_data->config->debugCharAnalysis)
    {
      cout << "The winning score is: " << bestScore << endl;
      // Draw the winning line segment

      Mat tempImg = Mat::zeros(Size(contours.width, contours.height), CV_8U);
      cvtColor(tempImg, tempImg, CV_GRAY2BGR);

      cv::line(tempImg, topLines[bestScoreIndex].p1, topLines[bestScoreIndex].p2, Scalar(0, 0, 255), 2);
      cv::line(tempImg, bottomLines[bestScoreIndex].p1, bottomLines[bestScoreIndex].p2, Scalar(0, 0, 255), 2);

      displayImage(pipeline_data->config, "Winning lines", tempImg);
    }

    Point topLeft 		= Point(0, topLines[bestScoreIndex].getPointAt(0) );
    Point topRight 		= Point(contours.width, topLines[bestScoreIndex].getPointAt(contours.width));
    Point bottomRight 	= Point(contours.width, bottomLines[bestScoreIndex].getPointAt(contours.width));
    Point bottomLeft 	= Point(0, bottomLines[bestScoreIndex].getPointAt(0));

    bestStripe.push_back(topLeft);
    bestStripe.push_back(topRight);
    bestStripe.push_back(bottomRight);
    bestStripe.push_back(bottomLeft);


    return bestStripe;
  }

  CharPointInfo::CharPointInfo(vector<Point> contour, int index) {


    this->contourIndex = index;

    this->boundingBox = cv::boundingRect( Mat(contour) );


    int x = boundingBox.x + (boundingBox.width / 2);
    int y = boundingBox.y;

    this->top = Point(x, y);

    x = boundingBox.x + (boundingBox.width / 2);
    y = boundingBox.y + boundingBox.height;

    this->bottom = Point(x,y);

  }

}