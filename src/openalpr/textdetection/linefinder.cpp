/*
 * Copyright (c) 2014 New Designs Unlimited, LLC
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

LineFinder::LineFinder(PipelineData* pipeline_data) {
  this->pipeline_data = pipeline_data;
}

LineFinder::~LineFinder() {
}

vector<TextLine> LineFinder::findLines(Mat image, const TextContours contours)
{
  vector<TextLine> linesFound;
  
  
  cvtColor(image, image, CV_GRAY2BGR);
  vector<Rect> boxes = this->getBoundingBoxes(contours);
  
  vector<Point> tops = this->getCharTops(boxes);
  vector<Point> bottoms = this->getCharBottoms(boxes);
  
  for (uint i = 0; i < tops.size(); i++)
  {
    circle(image, tops[i], 1, Scalar(255, 0, 0), 2);
    circle(image, bottoms[i], 1, Scalar(0, 0, 255), 2);
  }
  
  drawAndWait(&image);
  
  vector<Point> bestLine = getBestLine(contours, tops, bottoms);
  
  if (pipeline_data->isMultiline)
  {
    // we have a two-line plate.  Find the next best line, removing the tops/bottoms from before.
  }
  
  for (uint i = 0; i < contours.goodIndices.size(); i++)
  {
    
  }
  
  
  return linesFound;
}


vector<Rect> LineFinder::getBoundingBoxes(const TextContours contours) {
  
  vector<Rect> boxes;
  for (uint i = 0; i < contours.goodIndices.size(); i++)
  {
    if (contours.goodIndices[i] == false)
      continue;
    
    Rect bRect = cv::boundingRect( Mat(contours.contours[i]) );
    
    boxes.push_back(bRect);
  }
  
  return boxes;
}


vector<Point> LineFinder::getCharTops(vector<Rect> boxes) {
  
  vector<Point> tops;
  for (uint i = 0; i < boxes.size(); i++)
  {
    int x = boxes[i].x + (boxes[i].width / 2);
    int y = boxes[i].y;
    
    tops.push_back(Point(x, y));
  }

  return tops;
}

vector<Point> LineFinder::getCharBottoms(vector<Rect> boxes) {
  
  vector<Point> bottoms;
  for (uint i = 0; i < boxes.size(); i++)
  {
    int x = boxes[i].x + (boxes[i].width / 2);
    int y = boxes[i].y + boxes[i].height;
    
    bottoms.push_back(Point(x, y));
  }

  return bottoms;
}





// Returns a polygon "stripe" across the width of the character region.  The lines are voted and the polygon starts at 0 and extends to image width
vector<Point> LineFinder::getBestLine(const TextContours contours, vector<Point> tops, vector<Point> bottoms)
{


  vector<Point> bestStripe;

  // Find the best fit line segment that is parallel with the most char segments
  if (tops.size() <= 1)
  {
    // Maybe do something about this later, for now let's just ignore
  }
  else
  {
    vector<LineSegment> topLines;
    vector<LineSegment> bottomLines;
    // Iterate through each possible char and find all possible lines for the top and bottom of each char segment
    for (uint i = 0; i < tops.size() - 1; i++)
    {
      for (uint k = i+1; k < tops.size(); k++)
      {
        
        Point topLeft, topRight;
        Point bottomLeft, bottomRight;
        if (tops[i].x < tops[k].x)
        {
          topLeft = tops[i];
          topRight = tops[k];
          bottomLeft = bottoms[i];
          bottomRight = bottoms[k];
        }
        else
        {
          topLeft = tops[k];
          topRight = tops[i];
          bottomLeft = bottoms[k];
          bottomRight = bottoms[i];
        }
        
        
        LineSegment top(topLeft, topRight);
        LineSegment bottom(bottomLeft, bottomRight);
        
        // Only allow lines that have a sane angle
        if (abs(top.angle) <= pipeline_data->config->maxPlateAngleDegrees &&
            abs(bottom.angle) <= pipeline_data->config->maxPlateAngleDegrees)
        {
          topLines.push_back(top);
          bottomLines.push_back(bottom);
        }
        

      }
    }

    int bestScoreIndex = 0;
    int bestScore = -1;
    int bestScoreDistance = -1; // Line segment distance is used as a tie breaker

    // Now, among all possible lines, find the one that is the best fit
    for (uint i = 0; i < topLines.size(); i++)
    {
      float SCORING_MIN_THRESHOLD = 0.97;
      float SCORING_MAX_THRESHOLD = 1.03;

      int curScore = 0;
      for (uint charidx = 0; charidx < tops.size(); charidx++)
      {
        float topYPos = topLines[i].getPointAt(tops[charidx].x);
        float botYPos = bottomLines[i].getPointAt(bottoms[charidx].x);

        float minTop = tops[charidx].y * SCORING_MIN_THRESHOLD;
        float maxTop = tops[charidx].y * SCORING_MAX_THRESHOLD;
        float minBot = (bottoms[charidx].y) * SCORING_MIN_THRESHOLD;
        float maxBot = (bottoms[charidx].y) * SCORING_MAX_THRESHOLD;
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

    if (true)
    {
      cout << "The winning score is: " << bestScore << endl;
      // Draw the winning line segment
      Mat tempImg = Mat::zeros(Size(contours.width, contours.height), CV_8U);
      cvtColor(tempImg, tempImg, CV_GRAY2BGR);

      cv::line(tempImg, topLines[bestScoreIndex].p1, topLines[bestScoreIndex].p2, Scalar(0, 0, 255), 2);
      cv::line(tempImg, bottomLines[bestScoreIndex].p1, bottomLines[bestScoreIndex].p2, Scalar(0, 0, 255), 2);

      drawAndWait(&tempImg);
    }

    Point topLeft 		= Point(0, topLines[bestScoreIndex].getPointAt(0) );
    Point topRight 		= Point(contours.width, topLines[bestScoreIndex].getPointAt(contours.width));
    Point bottomRight 	= Point(contours.width, bottomLines[bestScoreIndex].getPointAt(contours.width));
    Point bottomLeft 	= Point(0, bottomLines[bestScoreIndex].getPointAt(0));

    bestStripe.push_back(topLeft);
    bestStripe.push_back(topRight);
    bestStripe.push_back(bottomRight);
    bestStripe.push_back(bottomLeft);
  }

  return bestStripe;
}
