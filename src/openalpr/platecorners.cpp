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

#include "platecorners.h"

using namespace cv;
using namespace std;

PlateCorners::PlateCorners(Mat inputImage, PlateLines* plateLines, CharacterRegion* charRegion, Config* config)
{
  this->config = config;

  if (this->config->debugPlateCorners)
    cout << "PlateCorners constructor" << endl;

  this->inputImage = inputImage;
  this->plateLines = plateLines;
  this->charRegion = charRegion;

  this->bestHorizontalScore = 9999999999999;
  this->bestVerticalScore = 9999999999999;

  Point topPoint = charRegion->getTopLine().midpoint();
  Point bottomPoint = charRegion->getBottomLine().closestPointOnSegmentTo(topPoint);
  this->charHeight = distanceBetweenPoints(topPoint, bottomPoint);


  this->charAngle = angleBetweenPoints(charRegion->getCharArea()[0], charRegion->getCharArea()[1]);
}

PlateCorners::~PlateCorners()
{
}

vector<Point> PlateCorners::findPlateCorners()
{
  if (this->config->debugPlateCorners)
    cout << "PlateCorners::findPlateCorners" << endl;

  timespec startTime;
  getTime(&startTime);

  int horizontalLines = this->plateLines->horizontalLines.size();
  int verticalLines = this->plateLines->verticalLines.size();

  // layout horizontal lines
  for (int h1 = NO_LINE; h1 < horizontalLines; h1++)
  {
    for (int h2 = NO_LINE; h2 < horizontalLines; h2++)
    {
      if (h1 == h2 && h1 != NO_LINE) continue;

      this->scoreHorizontals(h1, h2);
    }
  }

  // layout vertical lines
  for (int v1 = NO_LINE; v1 < verticalLines; v1++)
  {
    for (int v2 = NO_LINE; v2 < verticalLines; v2++)
    {
      if (v1 == v2 && v1 != NO_LINE) continue;

      this->scoreVerticals(v1, v2);
    }
  }

  if (this->config->debugPlateCorners)
  {
    cout << "Drawing debug stuff..." << endl;

    Mat imgCorners = Mat(inputImage.size(), inputImage.type());
    inputImage.copyTo(imgCorners);
    for (int i = 0; i < 4; i++)
      circle(imgCorners, charRegion->getCharArea()[i], 2, Scalar(0, 0, 0));

    line(imgCorners, this->bestTop.p1, this->bestTop.p2, Scalar(255, 0, 0), 1, CV_AA);
    line(imgCorners, this->bestRight.p1, this->bestRight.p2, Scalar(0, 0, 255), 1, CV_AA);
    line(imgCorners, this->bestBottom.p1, this->bestBottom.p2, Scalar(0, 0, 255), 1, CV_AA);
    line(imgCorners, this->bestLeft.p1, this->bestLeft.p2, Scalar(255, 0, 0), 1, CV_AA);

    displayImage(config, "Winning top/bottom Boundaries", imgCorners);
  }

  // Check if a left/right edge has been established.
  if (bestLeft.p1.x == 0 && bestLeft.p1.y == 0 && bestLeft.p2.x == 0 && bestLeft.p2.y == 0)
    confidence = 0;
  else if (bestTop.p1.x == 0 && bestTop.p1.y == 0 && bestTop.p2.x == 0 && bestTop.p2.y == 0)
    confidence = 0;
  else
    confidence = 100;

  vector<Point> corners;
  corners.push_back(bestTop.intersection(bestLeft));
  corners.push_back(bestTop.intersection(bestRight));
  corners.push_back(bestBottom.intersection(bestRight));
  corners.push_back(bestBottom.intersection(bestLeft));

  if (config->debugTiming)
  {
    timespec endTime;
    getTime(&endTime);
    cout << "Plate Corners Time: " << diffclock(startTime, endTime) << "ms." << endl;
  }

  return corners;
}

void PlateCorners::scoreVerticals(int v1, int v2)
{
  float score = 0;	// Lower is better

  LineSegment left;
  LineSegment right;

  float charHeightToPlateWidthRatio = config->plateWidthMM / config->charHeightMM;
  float idealPixelWidth = this->charHeight *  (charHeightToPlateWidthRatio * 1.03);	// Add 3% so we don't clip any characters

  float confidenceDiff = 0;
  float missingSegmentPenalty = 0;
  
  if (v1 == NO_LINE && v2 == NO_LINE)
  {
    //return;
    Point centerTop = charRegion->getCharBoxTop().midpoint();
    Point centerBottom = charRegion->getCharBoxBottom().midpoint();
    LineSegment centerLine = LineSegment(centerBottom.x, centerBottom.y, centerTop.x, centerTop.y);

    left = centerLine.getParallelLine(idealPixelWidth / 2);
    right = centerLine.getParallelLine(-1 * idealPixelWidth / 2 );

    missingSegmentPenalty += SCORING_MISSING_SEGMENT_PENALTY_VERTICAL * 2;
    confidenceDiff += 2;
  }
  else if (v1 != NO_LINE && v2 != NO_LINE)
  {
    left = this->plateLines->verticalLines[v1].line;
    right = this->plateLines->verticalLines[v2].line;
    confidenceDiff += (1.0 - this->plateLines->verticalLines[v1].confidence);
    confidenceDiff += (1.0 - this->plateLines->verticalLines[v2].confidence);
  }
  else if (v1 == NO_LINE && v2 != NO_LINE)
  {
    right = this->plateLines->verticalLines[v2].line;
    left = right.getParallelLine(idealPixelWidth);
    missingSegmentPenalty += SCORING_MISSING_SEGMENT_PENALTY_VERTICAL;
    confidenceDiff += (1.0 - this->plateLines->verticalLines[v2].confidence);
  }
  else if (v1 != NO_LINE && v2 == NO_LINE)
  {
    left = this->plateLines->verticalLines[v1].line;
    right = left.getParallelLine(-1 * idealPixelWidth);
    missingSegmentPenalty += SCORING_MISSING_SEGMENT_PENALTY_VERTICAL;
    confidenceDiff += (1.0 - this->plateLines->verticalLines[v1].confidence);
  }
  
  score += confidenceDiff * SCORING_LINE_CONFIDENCE_WEIGHT;
  score += missingSegmentPenalty;

  // Make sure this line is to the left of our license plate letters
  if (left.isPointBelowLine(charRegion->getCharBoxLeft().midpoint()) == false)
    return;

  // Make sure this line is to the right of our license plate letters
  if (right.isPointBelowLine(charRegion->getCharBoxRight().midpoint()))
    return;

  /////////////////////////////////////////////////////////////////////////
  // Score "Distance from the edge...
  /////////////////////////////////////////////////////////////////////////

  float leftDistanceFromEdge =  abs((float) (left.p1.x + left.p2.x) / 2);
  float rightDistanceFromEdge = abs(this->inputImage.cols - ((float) (right.p1.x + right.p2.x) / 2));

  float distanceFromEdge = leftDistanceFromEdge + rightDistanceFromEdge;
  score += distanceFromEdge * SCORING_VERTICALDISTANCE_FROMEDGE_WEIGHT;

  /////////////////////////////////////////////////////////////////////////
  // Score "Boxiness" of the 4 lines.  How close is it to a parallelogram?
  /////////////////////////////////////////////////////////////////////////

  float verticalAngleDiff = abs(left.angle - right.angle);

  score += (verticalAngleDiff) * SCORING_BOXINESS_WEIGHT;

  //////////////////////////////////////////////////////////////////////////
  // SCORE the shape wrt character position and height relative to position
  //////////////////////////////////////////////////////////////////////////

  Point leftMidLinePoint = left.closestPointOnSegmentTo(charRegion->getCharBoxLeft().midpoint());
  Point rightMidLinePoint = right.closestPointOnSegmentTo(charRegion->getCharBoxRight().midpoint());

  float plateDistance = abs(idealPixelWidth - distanceBetweenPoints(leftMidLinePoint, rightMidLinePoint));

  score += plateDistance * SCORING_DISTANCE_WEIGHT_VERTICAL;

  if (score < this->bestVerticalScore)
  {
    float scorecomponent;

    if (this->config->debugPlateCorners)
    {
      cout << "xx xx Score: charHeight " << this->charHeight << endl;
      cout << "xx xx Score: idealwidth " << idealPixelWidth << endl;
      cout << "xx xx Score: v1,v2= " << v1 << "," << v2 << endl;
      cout << "xx xx Score: Left= " << left.str() << endl;
      cout << "xx xx Score: Right= " << right.str() << endl;


      cout << "Vertical breakdown Score:" << endl;
      
      cout << " -- Missing Segment Score: " << missingSegmentPenalty << "  -- Weight (1.0)" << endl;
      scorecomponent = missingSegmentPenalty ;
      cout << " -- -- Score:       " << scorecomponent << " = " << scorecomponent / score * 100 << "% of score" << endl;
      
      cout << " -- Boxiness Score: " << verticalAngleDiff << "  -- Weight (" << SCORING_BOXINESS_WEIGHT << ")" << endl;
      scorecomponent = verticalAngleDiff * SCORING_BOXINESS_WEIGHT;
      cout << " -- -- Score:       " << scorecomponent << " = " << scorecomponent / score * 100 << "% of score" << endl;

      cout << " -- Distance From Edge Score: " << distanceFromEdge << " -- Weight (" << SCORING_VERTICALDISTANCE_FROMEDGE_WEIGHT << ")" << endl;
      scorecomponent = distanceFromEdge * SCORING_VERTICALDISTANCE_FROMEDGE_WEIGHT;
      cout << " -- -- Score:       " << scorecomponent << " = " << scorecomponent / score * 100 << "% of score" << endl;

      cout << " -- Distance Score: " << plateDistance << "  -- Weight (" << SCORING_DISTANCE_WEIGHT_VERTICAL << ")" << endl;
      scorecomponent = plateDistance * SCORING_DISTANCE_WEIGHT_VERTICAL;
      cout << " -- -- Score:       " << scorecomponent << " = " << scorecomponent / score * 100 << "% of score" << endl;
      
      cout << " -- Plate line confidence Score: " << confidenceDiff << "  -- Weight (" << SCORING_LINE_CONFIDENCE_WEIGHT << ")" << endl;
      scorecomponent = confidenceDiff * SCORING_LINE_CONFIDENCE_WEIGHT;
      cout << " -- -- Score:         " << scorecomponent << " = " << scorecomponent / score * 100 << "% of score" << endl;
      
      cout << " -- Score: " << score << endl;
    }

    this->bestVerticalScore = score;
    bestLeft = LineSegment(left.p1.x, left.p1.y, left.p2.x, left.p2.y);
    bestRight = LineSegment(right.p1.x, right.p1.y, right.p2.x, right.p2.y);
  }
}
// Score a collection of lines as a possible license plate region.
// If any segments are missing, extrapolate the missing pieces
void PlateCorners::scoreHorizontals(int h1, int h2)
{
  //if (this->debug)
  //    cout << "PlateCorners::scorePlate" << endl;

  float score = 0;	// Lower is better

  LineSegment top;
  LineSegment bottom;

  float charHeightToPlateHeightRatio = config->plateHeightMM / config->charHeightMM;
  float idealPixelHeight = this->charHeight *  charHeightToPlateHeightRatio;

  float confidenceDiff = 0;
  float missingSegmentPenalty = 0;
  
  if (h1 == NO_LINE && h2 == NO_LINE)
  {
//    return;
    Point centerLeft = charRegion->getCharBoxLeft().midpoint();
    Point centerRight = charRegion->getCharBoxRight().midpoint();
    LineSegment centerLine = LineSegment(centerLeft.x, centerLeft.y, centerRight.x, centerRight.y);

    top = centerLine.getParallelLine(idealPixelHeight / 2);
    bottom = centerLine.getParallelLine(-1 * idealPixelHeight / 2 );

    missingSegmentPenalty += SCORING_MISSING_SEGMENT_PENALTY_HORIZONTAL * 2;
    confidenceDiff += 2;
  }
  else if (h1 != NO_LINE && h2 != NO_LINE)
  {
    top = this->plateLines->horizontalLines[h1].line;
    bottom = this->plateLines->horizontalLines[h2].line;
    confidenceDiff += (1.0 - this->plateLines->horizontalLines[h1].confidence);
    confidenceDiff += (1.0 - this->plateLines->horizontalLines[h2].confidence);
  }
  else if (h1 == NO_LINE && h2 != NO_LINE)
  {
    bottom = this->plateLines->horizontalLines[h2].line;
    top = bottom.getParallelLine(idealPixelHeight);
    missingSegmentPenalty += SCORING_MISSING_SEGMENT_PENALTY_HORIZONTAL;
    confidenceDiff += (1.0 - this->plateLines->horizontalLines[h2].confidence);
  }
  else if (h1 != NO_LINE && h2 == NO_LINE)
  {
    top = this->plateLines->horizontalLines[h1].line;
    bottom = top.getParallelLine(-1 * idealPixelHeight);
    missingSegmentPenalty += SCORING_MISSING_SEGMENT_PENALTY_HORIZONTAL;
    confidenceDiff += (1.0 - this->plateLines->horizontalLines[h1].confidence);
  }
  
  score += confidenceDiff * SCORING_LINE_CONFIDENCE_WEIGHT;
  score += missingSegmentPenalty;

  // Make sure this line is above our license plate letters
  if (top.isPointBelowLine(charRegion->getCharBoxTop().midpoint()) == false)
    return;

  // Make sure this line is below our license plate letters
  if (bottom.isPointBelowLine(charRegion->getCharBoxBottom().midpoint()))
    return;

  // We now have 4 possible lines.  Let's put them to the test and score them...

  /////////////////////////////////////////////////////////////////////////
  // Score "Boxiness" of the 4 lines.  How close is it to a parallelogram?
  /////////////////////////////////////////////////////////////////////////

  float horizontalAngleDiff = abs(top.angle - bottom.angle);

  score += (horizontalAngleDiff) * SCORING_BOXINESS_WEIGHT;
//  if (this->debug)
//    cout << "PlateCorners boxiness score: " << (horizontalAngleDiff + verticalAngleDiff) * SCORING_BOXINESS_WEIGHT << endl;

  //////////////////////////////////////////////////////////////////////////
  // SCORE the shape wrt character position and height relative to position
  //////////////////////////////////////////////////////////////////////////

  Point topPoint = top.midpoint();
  Point botPoint = bottom.closestPointOnSegmentTo(topPoint);
  float plateHeightPx = distanceBetweenPoints(topPoint, botPoint);

  // Get the height difference

  float heightRatio = charHeight / plateHeightPx;
  float idealHeightRatio = (config->charHeightMM / config->plateHeightMM);
  //if (leftRatio < MIN_CHAR_HEIGHT_RATIO || leftRatio > MAX_CHAR_HEIGHT_RATIO || rightRatio < MIN_CHAR_HEIGHT_RATIO || rightRatio > MAX_CHAR_HEIGHT_RATIO)
  float heightRatioDiff = abs(heightRatio - idealHeightRatio);
  // Ideal ratio == ~.45

  // Get the distance from the top and the distance from the bottom
  // Take the average distances from the corners of the character region to the top/bottom lines
//  float topDistance  = distanceBetweenPoints(topMidLinePoint, charRegion->getCharBoxTop().midpoint());
//  float bottomDistance = distanceBetweenPoints(bottomMidLinePoint, charRegion->getCharBoxBottom().midpoint());

//  float idealTopDistance = charHeight * (TOP_WHITESPACE_HEIGHT_MM / CHARACTER_HEIGHT_MM);
//  float idealBottomDistance = charHeight * (BOTTOM_WHITESPACE_HEIGHT_MM / CHARACTER_HEIGHT_MM);
//  float distScore = abs(topDistance - idealTopDistance) + abs(bottomDistance - idealBottomDistance);

  score += heightRatioDiff * SCORING_PLATEHEIGHT_WEIGHT;

  //////////////////////////////////////////////////////////////////////////
  // SCORE the middliness of the stuff.  We want our top and bottom line to have the characters right towards the middle
  //////////////////////////////////////////////////////////////////////////

  Point charAreaMidPoint = charRegion->getCharBoxLeft().midpoint();
  Point topLineSpot = top.closestPointOnSegmentTo(charAreaMidPoint);
  Point botLineSpot = bottom.closestPointOnSegmentTo(charAreaMidPoint);

  float topDistanceFromMiddle = distanceBetweenPoints(topLineSpot, charAreaMidPoint);
  float bottomDistanceFromMiddle = distanceBetweenPoints(topLineSpot, charAreaMidPoint);

  float idealDistanceFromMiddle = idealPixelHeight / 2;

  float middleScore = abs(topDistanceFromMiddle - idealDistanceFromMiddle) + abs(bottomDistanceFromMiddle - idealDistanceFromMiddle);

  score += middleScore * SCORING_TOP_BOTTOM_SPACE_VS_CHARHEIGHT_WEIGHT;

//  if (this->debug)
//  {
//    cout << "PlateCorners boxiness score: " << avgRatio * SCORING_TOP_BOTTOM_SPACE_VS_CHARHEIGHT_WEIGHT << endl;
//    cout << "PlateCorners boxiness score: " << distScore * SCORING_PLATEHEIGHT_WEIGHT << endl;
//  }
  //////////////////////////////////////////////////////////////
  // SCORE: the shape for angles matching the character region
  //////////////////////////////////////////////////////////////

  float charanglediff = abs(charAngle - top.angle) + abs(charAngle - bottom.angle);

  score += charanglediff * SCORING_ANGLE_MATCHES_LPCHARS_WEIGHT;

//  if (this->debug)
//    cout << "PlateCorners boxiness score: " << charanglediff * SCORING_ANGLE_MATCHES_LPCHARS_WEIGHT << endl;

  if (score < this->bestHorizontalScore)
  {
    float scorecomponent;

    if (this->config->debugPlateCorners)
    {
      cout << "xx xx Score: charHeight " << this->charHeight << endl;
      cout << "xx xx Score: idealHeight " << idealPixelHeight << endl;
      cout << "xx xx Score: h1,h2= " << h1 << "," << h2 << endl;
      cout << "xx xx Score: Top= " << top.str() << endl;
      cout << "xx xx Score: Bottom= " << bottom.str() << endl;

      
      cout << "Horizontal breakdown Score:" << endl;
      
      cout << " -- Missing Segment Score: " << missingSegmentPenalty << "  -- Weight (1.0)" << endl;
      scorecomponent = missingSegmentPenalty ;
      cout << " -- -- Score:       " << scorecomponent << " = " << scorecomponent / score * 100 << "% of score" << endl;
      
      cout << " -- Boxiness Score: " << horizontalAngleDiff << "  -- Weight (" << SCORING_BOXINESS_WEIGHT << ")" << endl;
      scorecomponent = horizontalAngleDiff * SCORING_BOXINESS_WEIGHT;
      cout << " -- -- Score:       " << scorecomponent << " = " << scorecomponent / score * 100 << "% of score" << endl;

      cout << " -- Height Ratio Diff Score: " << heightRatioDiff << "  -- Weight (" << SCORING_PLATEHEIGHT_WEIGHT << ")" << endl;
      scorecomponent = heightRatioDiff * SCORING_PLATEHEIGHT_WEIGHT;
      cout << " -- -- " << scorecomponent << " = " << scorecomponent / score * 100 << "% of score" << endl;

      cout << " -- Distance Score: " << middleScore << "  -- Weight (" << SCORING_TOP_BOTTOM_SPACE_VS_CHARHEIGHT_WEIGHT << ")" << endl;
      scorecomponent = middleScore * SCORING_TOP_BOTTOM_SPACE_VS_CHARHEIGHT_WEIGHT;
      cout << " -- -- Score:       " << scorecomponent << " = " << scorecomponent / score * 100 << "% of score" << endl;

      cout << " -- Char angle Score: " << charanglediff << "  -- Weight (" << SCORING_ANGLE_MATCHES_LPCHARS_WEIGHT << ")" << endl;
      scorecomponent = charanglediff * SCORING_ANGLE_MATCHES_LPCHARS_WEIGHT;
      cout << " -- -- Score:         " << scorecomponent << " = " << scorecomponent / score * 100 << "% of score" << endl;

      cout << " -- Plate line confidence Score: " << confidenceDiff << "  -- Weight (" << SCORING_LINE_CONFIDENCE_WEIGHT << ")" << endl;
      scorecomponent = confidenceDiff * SCORING_LINE_CONFIDENCE_WEIGHT;
      cout << " -- -- Score:         " << scorecomponent << " = " << scorecomponent / score * 100 << "% of score" << endl;
      
      cout << " -- Score: " << score << endl;
    }
    this->bestHorizontalScore = score;
    bestTop = LineSegment(top.p1.x, top.p1.y, top.p2.x, top.p2.y);
    bestBottom = LineSegment(bottom.p1.x, bottom.p1.y, bottom.p2.x, bottom.p2.y);
  }
}
