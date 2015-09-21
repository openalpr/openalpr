/*
 * Copyright (c) 2015 OpenALPR Technology, Inc.
 * Open source Automated License Plate Recognition [http://www.openalpr.com]
 *
 * This file is part of OpenALPR.
 *
 * OpenALPR is free software: you can redistribute it and/or modify
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

namespace alpr
{

  PlateCorners::PlateCorners(Mat inputImage, PlateLines* plateLines, PipelineData* pipelineData, vector<TextLine> textLines) :
      tlc(textLines)
  {
    this->pipelineData = pipelineData;

    if (pipelineData->config->debugPlateCorners)
      cout << "PlateCorners constructor" << endl;

    this->inputImage = inputImage;
    this->plateLines = plateLines;
    this->textLines = textLines;

    this->bestHorizontalScore = 9999999999999;
    this->bestVerticalScore = 9999999999999;


  }

  PlateCorners::~PlateCorners()
  {
  }

  vector<Point> PlateCorners::findPlateCorners()
  {
    if (pipelineData->config->debugPlateCorners)
      cout << "PlateCorners::findPlateCorners" << endl;

    timespec startTime;
    getTimeMonotonic(&startTime);

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

    if (pipelineData->config->debugPlateCorners)
    {
      cout << "Drawing debug stuff..." << endl;

      Mat imgCorners = Mat(inputImage.size(), inputImage.type());
      inputImage.copyTo(imgCorners);

      for (unsigned int linenum = 0; linenum < textLines.size(); linenum++)
      {
        for (int i = 0; i < 4; i++)
          circle(imgCorners, textLines[linenum].textArea[i], 2, Scalar(0, 0, 0));
      }

      line(imgCorners, this->bestTop.p1, this->bestTop.p2, Scalar(255, 0, 0), 1, CV_AA);
      line(imgCorners, this->bestRight.p1, this->bestRight.p2, Scalar(0, 0, 255), 1, CV_AA);
      line(imgCorners, this->bestBottom.p1, this->bestBottom.p2, Scalar(0, 0, 255), 1, CV_AA);
      line(imgCorners, this->bestLeft.p1, this->bestLeft.p2, Scalar(255, 0, 0), 1, CV_AA);

      displayImage(pipelineData->config, "Winning top/bottom Boundaries", imgCorners);
    }

    // Check if a left/right edge has been established.
    if (bestLeft.p1.x == 0 && bestLeft.p1.y == 0 && bestLeft.p2.x == 0 && bestLeft.p2.y == 0)
    {
      pipelineData->disqualified = true;
      pipelineData->disqualify_reason = "platecorners did not find a left/right edge";
    }
    else if (bestTop.p1.x == 0 && bestTop.p1.y == 0 && bestTop.p2.x == 0 && bestTop.p2.y == 0)
    {
      pipelineData->disqualified = true;
      pipelineData->disqualify_reason = "platecorners did not find a top/bottom edge";
    }


    vector<Point> corners;
    corners.push_back(bestTop.intersection(bestLeft));
    corners.push_back(bestTop.intersection(bestRight));
    corners.push_back(bestBottom.intersection(bestRight));
    corners.push_back(bestBottom.intersection(bestLeft));

    if (pipelineData->config->debugTiming)
    {
      timespec endTime;
      getTimeMonotonic(&endTime);
      cout << "Plate Corners Time: " << diffclock(startTime, endTime) << "ms." << endl;
    }

    return corners;
  }

  void PlateCorners::scoreVerticals(int v1, int v2)
  {
    ScoreKeeper scoreKeeper;

    LineSegment left;
    LineSegment right;


    float charHeightToPlateWidthRatio = pipelineData->config->plateWidthMM / pipelineData->config->avgCharHeightMM;
    float idealPixelWidth = tlc.charHeight *  (charHeightToPlateWidthRatio * 1.03);	// Add 3% so we don't clip any characters

    float confidenceDiff = 0;
    float missingSegmentPenalty = 0;

    if (v1 == NO_LINE && v2 == NO_LINE)
    {
      //return;

      left = tlc.centerVerticalLine.getParallelLine(-1 * idealPixelWidth / 2);
      right = tlc.centerVerticalLine.getParallelLine(idealPixelWidth / 2 );

      missingSegmentPenalty = 2;
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
      missingSegmentPenalty++;
      confidenceDiff += (1.0 - this->plateLines->verticalLines[v2].confidence);
    }
    else if (v1 != NO_LINE && v2 == NO_LINE)
    {
      left = this->plateLines->verticalLines[v1].line;
      right = left.getParallelLine(-1 * idealPixelWidth);
      missingSegmentPenalty++;
      confidenceDiff += (1.0 - this->plateLines->verticalLines[v1].confidence);
    }

    scoreKeeper.setScore("SCORING_LINE_CONFIDENCE_WEIGHT", confidenceDiff, SCORING_LINE_CONFIDENCE_WEIGHT);
    scoreKeeper.setScore("SCORING_MISSING_SEGMENT_PENALTY_VERTICAL", missingSegmentPenalty, SCORING_MISSING_SEGMENT_PENALTY_VERTICAL);

    // Make sure that the left and right lines are to the left and right of our text 
    // area
    if (tlc.isLeftOfText(left) < 1 || tlc.isLeftOfText(right) > -1)
      return;


    /////////////////////////////////////////////////////////////////////////
    // Score angle difference from detected character box
    /////////////////////////////////////////////////////////////////////////

    float perpendicularCharAngle = tlc.charAngle - 90;
    float charanglediff = abs(perpendicularCharAngle - left.angle) + abs(perpendicularCharAngle - right.angle);

    scoreKeeper.setScore("SCORING_ANGLE_MATCHES_LPCHARS_WEIGHT", charanglediff, SCORING_ANGLE_MATCHES_LPCHARS_WEIGHT);

    //////////////////////////////////////////////////////////////////////////
    // SCORE the shape wrt character position and height relative to position
    //////////////////////////////////////////////////////////////////////////

    Point leftMidLinePoint = left.closestPointOnSegmentTo(tlc.centerVerticalLine.midpoint());
    Point rightMidLinePoint = right.closestPointOnSegmentTo(tlc.centerVerticalLine.midpoint());

    float actual_width = distanceBetweenPoints(leftMidLinePoint, rightMidLinePoint);
    
    // Disqualify the pairing if it's less than one quarter of the ideal width
    if (actual_width < (idealPixelWidth / 4))
      return;
      
    float plateDistance = abs(idealPixelWidth - actual_width);

    // normalize for image width
    plateDistance = plateDistance / ((float)inputImage.cols);
    
    scoreKeeper.setScore("SCORING_DISTANCE_WEIGHT_VERTICAL", plateDistance, SCORING_DISTANCE_WEIGHT_VERTICAL);

    float score = scoreKeeper.getTotal();

    if (score < this->bestVerticalScore)
    {

      if (pipelineData->config->debugPlateCorners)
      {

        cout << "Vertical breakdown Score:" << endl;

        scoreKeeper.printDebugScores();
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

    ScoreKeeper scoreKeeper;

    LineSegment top;
    LineSegment bottom;

    float charHeightToPlateHeightRatio = pipelineData->config->plateHeightMM / pipelineData->config->avgCharHeightMM;
    float idealPixelHeight = tlc.charHeight *  charHeightToPlateHeightRatio;

    float confidenceDiff = 0;
    float missingSegmentPenalty = 0;

    if (h1 == NO_LINE && h2 == NO_LINE)
    {
  //    return;


      top = tlc.centerHorizontalLine.getParallelLine(idealPixelHeight / 2);
      bottom = tlc.centerHorizontalLine.getParallelLine(-1 * idealPixelHeight / 2 );

      missingSegmentPenalty = 2;
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
      missingSegmentPenalty++;
      confidenceDiff += (1.0 - this->plateLines->horizontalLines[h2].confidence);
    }
    else if (h1 != NO_LINE && h2 == NO_LINE)
    {
      top = this->plateLines->horizontalLines[h1].line;
      bottom = top.getParallelLine(-1 * idealPixelHeight);
      missingSegmentPenalty++;
      confidenceDiff += (1.0 - this->plateLines->horizontalLines[h1].confidence);
    }

    scoreKeeper.setScore("SCORING_MISSING_SEGMENT_PENALTY_HORIZONTAL", missingSegmentPenalty, SCORING_MISSING_SEGMENT_PENALTY_HORIZONTAL);
    //scoreKeeper.setScore("SCORING_LINE_CONFIDENCE_WEIGHT", confidenceDiff, SCORING_LINE_CONFIDENCE_WEIGHT);


    // Make sure that the top and bottom lines are above and below
    // the text area
    if (tlc.isAboveText(top) < 1 || tlc.isAboveText(bottom) > -1)
      return;

    // We now have 4 possible lines.  Let's put them to the test and score them...


    //////////////////////////////////////////////////////////////////////////
    // SCORE the shape wrt character position and height relative to position
    //////////////////////////////////////////////////////////////////////////

    Point topPoint = top.midpoint();
    Point botPoint = bottom.closestPointOnSegmentTo(topPoint);
    float plateHeightPx = distanceBetweenPoints(topPoint, botPoint);

    // Get the height difference

    float heightRatio = tlc.charHeight / plateHeightPx;
    float idealHeightRatio = (pipelineData->config->avgCharHeightMM / pipelineData->config->plateHeightMM);
    float heightRatioDiff = abs(heightRatio - idealHeightRatio);

    scoreKeeper.setScore("SCORING_PLATEHEIGHT_WEIGHT", heightRatioDiff, SCORING_PLATEHEIGHT_WEIGHT);

    //////////////////////////////////////////////////////////////////////////
    // SCORE the middliness of the stuff.  We want our top and bottom line to have the characters right towards the middle
    //////////////////////////////////////////////////////////////////////////

    Point charAreaMidPoint = tlc.centerVerticalLine.midpoint();
    Point topLineSpot = top.closestPointOnSegmentTo(charAreaMidPoint);
    Point botLineSpot = bottom.closestPointOnSegmentTo(charAreaMidPoint);

    float topDistanceFromMiddle = distanceBetweenPoints(topLineSpot, charAreaMidPoint);
    float bottomDistanceFromMiddle = distanceBetweenPoints(botLineSpot, charAreaMidPoint);

    float idealDistanceFromMiddle = idealPixelHeight / 2;

    float middleScore = abs(topDistanceFromMiddle - idealDistanceFromMiddle) / idealDistanceFromMiddle;
    middleScore +=      abs(bottomDistanceFromMiddle - idealDistanceFromMiddle) / idealDistanceFromMiddle;

    scoreKeeper.setScore("SCORING_TOP_BOTTOM_SPACE_VS_CHARHEIGHT_WEIGHT", middleScore, SCORING_TOP_BOTTOM_SPACE_VS_CHARHEIGHT_WEIGHT);


    //////////////////////////////////////////////////////////////
    // SCORE: the shape for angles matching the character region
    //////////////////////////////////////////////////////////////

    float charanglediff = abs(tlc.charAngle - top.angle) + abs(tlc.charAngle - bottom.angle);

    scoreKeeper.setScore("SCORING_ANGLE_MATCHES_LPCHARS_WEIGHT", charanglediff, SCORING_ANGLE_MATCHES_LPCHARS_WEIGHT);

    if (pipelineData->config->debugPlateCorners)
    {
      scoreKeeper.printDebugScores();
      Mat debugImg(this->inputImage.size(), this->inputImage.type());
      this->inputImage.copyTo(debugImg);
      cvtColor(debugImg, debugImg, CV_GRAY2BGR);
      line(debugImg, top.p1, top.p2, Scalar(0,0,255), 2);
      line(debugImg, bottom.p1, bottom.p2, Scalar(0,0,255), 2);
      //drawAndWait(&debugImg);

    }

    float score = scoreKeeper.getTotal();
    if (score < this->bestHorizontalScore)
    {
      float scorecomponent;

      if (pipelineData->config->debugPlateCorners)
      {
        cout << "Horizontal breakdown Score:" << endl;
        scoreKeeper.printDebugScores();
      }
      this->bestHorizontalScore = score;
      bestTop = LineSegment(top.p1.x, top.p1.y, top.p2.x, top.p2.y);
      bestBottom = LineSegment(bottom.p1.x, bottom.p1.y, bottom.p2.x, bottom.p2.y);
    }
  }

}


