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

#include "characteranalysis.h"

using namespace cv;
using namespace std;

CharacterAnalysis::CharacterAnalysis(PipelineData* pipeline_data)
{
  this->pipeline_data = pipeline_data;
  this->config = pipeline_data->config;

  this->hasPlateMask = false;

  if (this->config->debugCharAnalysis)
    cout << "Starting CharacterAnalysis identification" << endl;

}

CharacterAnalysis::~CharacterAnalysis()
{

}

void CharacterAnalysis::analyze()
{
  pipeline_data->clearThresholds();
  pipeline_data->thresholds = produceThresholds(pipeline_data->crop_gray, config);



  timespec startTime;
  getTime(&startTime);

  for (int i = 0; i < pipeline_data->thresholds.size(); i++)
  {
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    Mat tempThreshold(pipeline_data->thresholds[i].size(), CV_8U);
    pipeline_data->thresholds[i].copyTo(tempThreshold);
    findContours(tempThreshold,
                 contours, // a vector of contours
                 hierarchy,
                 CV_RETR_TREE, // retrieve all contours
                 CV_CHAIN_APPROX_SIMPLE ); // all pixels of each contours

    allContours.push_back(contours);
    allHierarchy.push_back(hierarchy);
  }

  if (config->debugTiming)
  {
    timespec endTime;
    getTime(&endTime);
    cout << "  -- Character Analysis Find Contours Time: " << diffclock(startTime, endTime) << "ms." << endl;
  }
  //Mat img_equalized = equalizeBrightness(img_gray);

  getTime(&startTime);

  for (int i = 0; i < pipeline_data->thresholds.size(); i++)
  {
    vector<bool> goodIndices = this->filter(pipeline_data->thresholds[i], allContours[i], allHierarchy[i]);
    charSegments.push_back(goodIndices);

    if (config->debugCharAnalysis)
      cout << "Threshold " << i << " had " << getGoodIndicesCount(goodIndices) << " good indices." << endl;
  }

  if (config->debugTiming)
  {
    timespec endTime;
    getTime(&endTime);
    cout << "  -- Character Analysis Filter Time: " << diffclock(startTime, endTime) << "ms." << endl;
  }

  this->plateMask = findOuterBoxMask();

  if (hasPlateMask)
  {
    // Filter out bad contours now that we have an outer box mask...
    for (int i = 0; i < pipeline_data->thresholds.size(); i++)
    {
      charSegments[i] = filterByOuterMask(allContours[i], allHierarchy[i], charSegments[i]);
    }
  }

  int bestFitScore = -1;
  int bestFitIndex = -1;
  for (int i = 0; i < pipeline_data->thresholds.size(); i++)
  {
    //vector<bool> goodIndices = this->filter(thresholds[i], allContours[i], allHierarchy[i]);
    //charSegments.push_back(goodIndices);

    int segmentCount = getGoodIndicesCount(charSegments[i]);

    if (segmentCount > bestFitScore)
    {
      bestFitScore = segmentCount;
      bestFitIndex = i;
      bestCharSegments = charSegments[i];
      bestThreshold = pipeline_data->thresholds[i];
      bestContours = allContours[i];
      bestHierarchy = allHierarchy[i];
      bestCharSegmentsCount = segmentCount;
    }
  }

  if (this->config->debugCharAnalysis)
    cout << "Best fit score: " << bestFitScore << " Index: " << bestFitIndex << endl;

  if (bestFitScore <= 1)
    return;

  //getColorMask(img, allContours, allHierarchy, charSegments);

  if (this->config->debugCharAnalysis)
  {
    Mat img_contours(bestThreshold.size(), CV_8U);
    bestThreshold.copyTo(img_contours);
    cvtColor(img_contours, img_contours, CV_GRAY2RGB);

    vector<vector<Point> > allowedContours;
    for (int i = 0; i < bestContours.size(); i++)
    {
      if (bestCharSegments[i])
        allowedContours.push_back(bestContours[i]);
    }

    drawContours(img_contours, bestContours,
                 -1, // draw all contours
                 cv::Scalar(255,0,0), // in blue
                 1); // with a thickness of 1

    drawContours(img_contours, allowedContours,
                 -1, // draw all contours
                 cv::Scalar(0,255,0), // in green
                 1); // with a thickness of 1

    displayImage(config, "Matching Contours", img_contours);
  }

  //charsegments = this->getPossibleCharRegions(img_threshold, allContours, allHierarchy, STARTING_MIN_HEIGHT + (bestFitIndex * HEIGHT_STEP), STARTING_MAX_HEIGHT + (bestFitIndex * HEIGHT_STEP));

  this->linePolygon =  getBestVotedLines(pipeline_data->crop_gray, bestContours, bestCharSegments);

  if (this->linePolygon.size() > 0)
  {
    this->topLine = LineSegment(this->linePolygon[0].x, this->linePolygon[0].y, this->linePolygon[1].x, this->linePolygon[1].y);
    this->bottomLine = LineSegment(this->linePolygon[3].x, this->linePolygon[3].y, this->linePolygon[2].x, this->linePolygon[2].y);
    //this->charArea = getCharSegmentsBetweenLines(bestThreshold, bestContours, this->linePolygon);
    filterBetweenLines(bestThreshold, bestContours, bestHierarchy, linePolygon, bestCharSegments);

    this->charArea = getCharArea();

    if (this->charArea.size() > 0)
    {
      this->charBoxTop = LineSegment(this->charArea[0].x, this->charArea[0].y, this->charArea[1].x, this->charArea[1].y);
      this->charBoxBottom = LineSegment(this->charArea[3].x, this->charArea[3].y, this->charArea[2].x, this->charArea[2].y);
      this->charBoxLeft = LineSegment(this->charArea[3].x, this->charArea[3].y, this->charArea[0].x, this->charArea[0].y);
      this->charBoxRight = LineSegment(this->charArea[2].x, this->charArea[2].y, this->charArea[1].x, this->charArea[1].y);
    }
  }

  this->thresholdsInverted = isPlateInverted();
}

int CharacterAnalysis::getGoodIndicesCount(vector<bool> goodIndices)
{
  int count = 0;
  for (int i = 0; i < goodIndices.size(); i++)
  {
    if (goodIndices[i])
      count++;
  }

  return count;
}

Mat CharacterAnalysis::findOuterBoxMask()
{
  double min_parent_area = config->templateHeightPx * config->templateWidthPx * 0.10;	// Needs to be at least 10% of the plate area to be considered.

  int winningIndex = -1;
  int winningParentId = -1;
  int bestCharCount = 0;
  double lowestArea = 99999999999999;

  if (this->config->debugCharAnalysis)
    cout << "CharacterAnalysis::findOuterBoxMask" << endl;

  for (int imgIndex = 0; imgIndex < allContours.size(); imgIndex++)
  {
    //vector<bool> charContours = filter(thresholds[imgIndex], allContours[imgIndex], allHierarchy[imgIndex]);

    int charsRecognized = 0;
    int parentId = -1;
    bool hasParent = false;
    for (int i = 0; i < charSegments[imgIndex].size(); i++)
    {
      if (charSegments[imgIndex][i]) charsRecognized++;
      if (charSegments[imgIndex][i] && allHierarchy[imgIndex][i][3] != -1)
      {
        parentId = allHierarchy[imgIndex][i][3];
        hasParent = true;
      }
    }

    if (charsRecognized == 0)
      continue;

    if (hasParent)
    {
      double boxArea = contourArea(allContours[imgIndex][parentId]);
      if (boxArea < min_parent_area)
        continue;

      if ((charsRecognized > bestCharCount) ||
          (charsRecognized == bestCharCount && boxArea < lowestArea))
        //(boxArea < lowestArea)
      {
        bestCharCount = charsRecognized;
        winningIndex = imgIndex;
        winningParentId = parentId;
        lowestArea = boxArea;
      }
    }
  }

  if (this->config->debugCharAnalysis)
    cout << "Winning image index (findOuterBoxMask) is: " << winningIndex << endl;

  if (winningIndex != -1 && bestCharCount >= 3)
  {
    int longestChildIndex = -1;
    double longestChildLength = 0;
    // Find the child with the longest permiter/arc length ( just for kicks)
    for (int i = 0; i < allContours[winningIndex].size(); i++)
    {
      for (int j = 0; j < allContours[winningIndex].size(); j++)
      {
        if (allHierarchy[winningIndex][j][3] == winningParentId)
        {
          double arclength = arcLength(allContours[winningIndex][j], false);
          if (arclength > longestChildLength)
          {
            longestChildIndex = j;
            longestChildLength = arclength;
          }
        }
      }
    }

    Mat mask = Mat::zeros(pipeline_data->thresholds[winningIndex].size(), CV_8U);

    // get rid of the outline by drawing a 1 pixel width black line
    drawContours(mask, allContours[winningIndex],
                 winningParentId, // draw this contour
                 cv::Scalar(255,255,255), // in
                 CV_FILLED,
                 8,
                 allHierarchy[winningIndex],
                 0
                );

    // Morph Open the mask to get rid of any little connectors to non-plate portions
    int morph_elem  = 2;
    int morph_size = 3;
    Mat element = getStructuringElement( morph_elem, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );

    //morphologyEx( mask, mask, MORPH_CLOSE, element );
    morphologyEx( mask, mask, MORPH_OPEN, element );

    //morph_size = 1;
    //element = getStructuringElement( morph_elem, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );
    //dilate(mask, mask, element);

    // Drawing the edge black effectively erodes the image.  This may clip off some extra junk from the edges.
    // We'll want to do the contour again and find the larges one so that we remove the clipped portion.

    vector<vector<Point> > contoursSecondRound;

    findContours(mask, contoursSecondRound, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    int biggestContourIndex = -1;
    double largestArea = 0;
    for (int c = 0; c < contoursSecondRound.size(); c++)
    {
      double area = contourArea(contoursSecondRound[c]);
      if (area > largestArea)
      {
        biggestContourIndex = c;
        largestArea = area;
      }
    }

    if (biggestContourIndex != -1)
    {
      mask = Mat::zeros(pipeline_data->thresholds[winningIndex].size(), CV_8U);

      vector<Point> smoothedMaskPoints;
      approxPolyDP(contoursSecondRound[biggestContourIndex], smoothedMaskPoints, 2, true);

      vector<vector<Point> > tempvec;
      tempvec.push_back(smoothedMaskPoints);
      //fillPoly(mask, smoothedMaskPoints.data(), smoothedMaskPoints, Scalar(255,255,255));
      drawContours(mask, tempvec,
                   0, // draw this contour
                   cv::Scalar(255,255,255), // in
                   CV_FILLED,
                   8,
                   allHierarchy[winningIndex],
                   0
                  );
    }

    if (this->config->debugCharAnalysis)
    {
      vector<Mat> debugImgs;
      Mat debugImgMasked = Mat::zeros(pipeline_data->thresholds[winningIndex].size(), CV_8U);

      pipeline_data->thresholds[winningIndex].copyTo(debugImgMasked, mask);

      debugImgs.push_back(mask);
      debugImgs.push_back(pipeline_data->thresholds[winningIndex]);
      debugImgs.push_back(debugImgMasked);

      Mat dashboard = drawImageDashboard(debugImgs, CV_8U, 1);
      displayImage(config, "Winning outer box", dashboard);
    }

    hasPlateMask = true;
    return mask;
  }

  hasPlateMask = false;
  Mat fullMask = Mat::zeros(pipeline_data->thresholds[0].size(), CV_8U);
  bitwise_not(fullMask, fullMask);
  return fullMask;
}

Mat CharacterAnalysis::getCharacterMask()
{
  Mat charMask = Mat::zeros(bestThreshold.size(), CV_8U);

  for (int i = 0; i < bestContours.size(); i++)
  {
    if (bestCharSegments[i] == false)
      continue;

    drawContours(charMask, bestContours,
                 i, // draw this contour
                 cv::Scalar(255,255,255), // in
                 CV_FILLED,
                 8,
                 bestHierarchy,
                 1
                );

  }

  return charMask;
}

// Returns a polygon "stripe" across the width of the character region.  The lines are voted and the polygon starts at 0 and extends to image width
vector<Point> CharacterAnalysis::getBestVotedLines(Mat img, vector<vector<Point> > contours, vector<bool> goodIndices)
{
  //if (this->debug)
  //  cout << "CharacterAnalysis::getBestVotedLines" << endl;

  vector<Point> bestStripe;

  vector<Rect> charRegions;

  for (int i = 0; i < contours.size(); i++)
  {
    if (goodIndices[i])
      charRegions.push_back(boundingRect(contours[i]));
  }

  // Find the best fit line segment that is parallel with the most char segments
  if (charRegions.size() <= 1)
  {
    // Maybe do something about this later, for now let's just ignore
  }
  else
  {
    vector<LineSegment> topLines;
    vector<LineSegment> bottomLines;
    // Iterate through each possible char and find all possible lines for the top and bottom of each char segment
    for (int i = 0; i < charRegions.size() - 1; i++)
    {
      for (int k = i+1; k < charRegions.size(); k++)
      {
        //Mat tempImg;
        //result.copyTo(tempImg);

        Rect* leftRect;
        Rect* rightRect;
        if (charRegions[i].x < charRegions[k].x)
        {
          leftRect = &charRegions[i];
          rightRect = &charRegions[k];
        }
        else
        {
          leftRect = &charRegions[k];
          rightRect = &charRegions[i];
        }

        //rectangle(tempImg, *leftRect, Scalar(0, 255, 0), 2);
        //rectangle(tempImg, *rightRect, Scalar(255, 255, 255), 2);

        int x1, y1, x2, y2;

        if (leftRect->y > rightRect->y)	// Rising line, use the top left corner of the rect
        {
          x1 = leftRect->x;
          x2 = rightRect->x;
        }
        else					// falling line, use the top right corner of the rect
        {
          x1 = leftRect->x + leftRect->width;
          x2 = rightRect->x + rightRect->width;
        }
        y1 = leftRect->y;
        y2 = rightRect->y;

        //cv::line(tempImg, Point(x1, y1), Point(x2, y2), Scalar(0, 0, 255));
        topLines.push_back(LineSegment(x1, y1, x2, y2));

        if (leftRect->y > rightRect->y)	// Rising line, use the bottom right corner of the rect
        {
          x1 = leftRect->x + leftRect->width;
          x2 = rightRect->x + rightRect->width;
        }
        else					// falling line, use the bottom left corner of the rect
        {
          x1 = leftRect->x;
          x2 = rightRect->x;
        }
        y1 = leftRect->y + leftRect->height;
        y2 = rightRect->y + leftRect->height;

        //cv::line(tempImg, Point(x1, y1), Point(x2, y2), Scalar(0, 0, 255));
        bottomLines.push_back(LineSegment(x1, y1, x2, y2));

        //drawAndWait(&tempImg);
      }
    }

    int bestScoreIndex = 0;
    int bestScore = -1;
    int bestScoreDistance = -1; // Line segment distance is used as a tie breaker

    // Now, among all possible lines, find the one that is the best fit
    for (int i = 0; i < topLines.size(); i++)
    {
      float SCORING_MIN_THRESHOLD = 0.97;
      float SCORING_MAX_THRESHOLD = 1.03;

      int curScore = 0;
      for (int charidx = 0; charidx < charRegions.size(); charidx++)
      {
        float topYPos = topLines[i].getPointAt(charRegions[charidx].x);
        float botYPos = bottomLines[i].getPointAt(charRegions[charidx].x);

        float minTop = charRegions[charidx].y * SCORING_MIN_THRESHOLD;
        float maxTop = charRegions[charidx].y * SCORING_MAX_THRESHOLD;
        float minBot = (charRegions[charidx].y + charRegions[charidx].height) * SCORING_MIN_THRESHOLD;
        float maxBot = (charRegions[charidx].y + charRegions[charidx].height) * SCORING_MAX_THRESHOLD;
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

    if (this->config->debugCharAnalysis)
    {
      cout << "The winning score is: " << bestScore << endl;
      // Draw the winning line segment
      //Mat tempImg;
      //result.copyTo(tempImg);
      //cv::line(tempImg, topLines[bestScoreIndex].p1, topLines[bestScoreIndex].p2, Scalar(0, 0, 255), 2);
      //cv::line(tempImg, bottomLines[bestScoreIndex].p1, bottomLines[bestScoreIndex].p2, Scalar(0, 0, 255), 2);

      //displayImage(config, "Lines", tempImg);
    }

    //winningLines.push_back(topLines[bestScoreIndex]);
    //winningLines.push_back(bottomLines[bestScoreIndex]);

    Point topLeft 		= Point(0, topLines[bestScoreIndex].getPointAt(0) );
    Point topRight 		= Point(img.cols, topLines[bestScoreIndex].getPointAt(img.cols));
    Point bottomRight 	= Point(img.cols, bottomLines[bestScoreIndex].getPointAt(img.cols));
    Point bottomLeft 	= Point(0, bottomLines[bestScoreIndex].getPointAt(0));

    bestStripe.push_back(topLeft);
    bestStripe.push_back(topRight);
    bestStripe.push_back(bottomRight);
    bestStripe.push_back(bottomLeft);
  }

  return bestStripe;
}

vector<bool> CharacterAnalysis::filter(Mat img, vector<vector<Point> > contours, vector<Vec4i> hierarchy)
{
  static int STARTING_MIN_HEIGHT = round (((float) img.rows) * config->charAnalysisMinPercent);
  static int STARTING_MAX_HEIGHT = round (((float) img.rows) * (config->charAnalysisMinPercent + config->charAnalysisHeightRange));
  static int HEIGHT_STEP = round (((float) img.rows) * config->charAnalysisHeightStepSize);
  static int NUM_STEPS = config->charAnalysisNumSteps;

  vector<bool> charSegments;
  int bestFitScore = -1;
  for (int i = 0; i < NUM_STEPS; i++)
  {
    int goodIndicesCount;

    vector<bool> goodIndices(contours.size());
    for (int z = 0; z < goodIndices.size(); z++) goodIndices[z] = true;

    goodIndices = this->filterByBoxSize(contours, goodIndices, STARTING_MIN_HEIGHT + (i * HEIGHT_STEP), STARTING_MAX_HEIGHT + (i * HEIGHT_STEP));

    goodIndicesCount = getGoodIndicesCount(goodIndices);
    if ( goodIndicesCount == 0 || goodIndicesCount <= bestFitScore)	// Don't bother doing more filtering if we already lost...
      continue;
    goodIndices = this->filterContourHoles(contours, hierarchy, goodIndices);

    goodIndicesCount = getGoodIndicesCount(goodIndices);
    if ( goodIndicesCount == 0 || goodIndicesCount <= bestFitScore)	// Don't bother doing more filtering if we already lost...
      continue;
    //goodIndices = this->filterByParentContour( contours, hierarchy, goodIndices);
    vector<Point> lines = getBestVotedLines(img, contours, goodIndices);
    goodIndices = this->filterBetweenLines(img, contours, hierarchy, lines, goodIndices);

    int segmentCount = getGoodIndicesCount(goodIndices);

    if (segmentCount > bestFitScore)
    {
      bestFitScore = segmentCount;
      charSegments = goodIndices;
    }
  }

  return charSegments;
}

// Goes through the contours for the plate and picks out possible char segments based on min/max height
vector<bool> CharacterAnalysis::filterByBoxSize(vector< vector< Point> > contours, vector<bool> goodIndices, int minHeightPx, int maxHeightPx)
{
  float idealAspect=config->charWidthMM / config->charHeightMM;
  float aspecttolerance=0.25;

  vector<bool> includedIndices(contours.size());
  for (int j = 0; j < contours.size(); j++)
    includedIndices.push_back(false);

  for (int i = 0; i < contours.size(); i++)
  {
    if (goodIndices[i] == false)
      continue;

    //Create bounding rect of object
    Rect mr= boundingRect(contours[i]);

    float minWidth = mr.height * 0.2;
    //Crop image
    //Mat auxRoi(img, mr);
    if(mr.height >= minHeightPx && mr.height <= maxHeightPx && mr.width > minWidth)
    {
      float charAspect= (float)mr.width/(float)mr.height;

      if (abs(charAspect - idealAspect) < aspecttolerance)
        includedIndices[i] = true;
    }
  }

  return includedIndices;
}

vector< bool > CharacterAnalysis::filterContourHoles(vector< vector< Point > > contours, vector< Vec4i > hierarchy, vector< bool > goodIndices)
{
  vector<bool> includedIndices(contours.size());
  for (int j = 0; j < contours.size(); j++)
    includedIndices.push_back(false);

  for (int i = 0; i < contours.size(); i++)
  {
    if (goodIndices[i] == false)
      continue;

    int parentIndex = hierarchy[i][3];

    if (parentIndex >= 0 && goodIndices[parentIndex])
    {
      // this contour is a child of an already identified contour.  REMOVE it
      if (this->config->debugCharAnalysis)
      {
        cout << "filterContourHoles: contour index: " << i << endl;
      }
    }
    else
    {
      includedIndices[i] = true;
    }
  }

  return includedIndices;
}

// Goes through the contours for the plate and picks out possible char segments based on min/max height
// returns a vector of indices corresponding to valid contours
vector<bool> CharacterAnalysis::filterByParentContour( vector< vector< Point> > contours, vector<Vec4i> hierarchy, vector<bool> goodIndices)
{
  vector<bool> includedIndices(contours.size());
  for (int j = 0; j < contours.size(); j++)
    includedIndices[j] = false;

  vector<int> parentIDs;
  vector<int> votes;

  for (int i = 0; i < contours.size(); i++)
  {
    if (goodIndices[i] == false)
      continue;

    int voteIndex = -1;
    int parentID = hierarchy[i][3];
    // check if parentID is already in the lsit
    for (int j = 0; j < parentIDs.size(); j++)
    {
      if (parentIDs[j] == parentID)
      {
        voteIndex = j;
        break;
      }
    }
    if (voteIndex == -1)
    {
      parentIDs.push_back(parentID);
      votes.push_back(1);
    }
    else
    {
      votes[voteIndex] = votes[voteIndex] + 1;
    }
  }

  // Tally up the votes, pick the winner
  int totalVotes = 0;
  int winningParentId = 0;
  int highestVotes = 0;
  for (int i = 0; i < parentIDs.size(); i++)
  {
    if (votes[i] > highestVotes)
    {
      winningParentId = parentIDs[i];
      highestVotes = votes[i];
    }
    totalVotes += votes[i];
  }

  // Now filter out all the contours with a different parent ID (assuming the totalVotes > 2)
  for (int i = 0; i < contours.size(); i++)
  {
    if (goodIndices[i] == false)
      continue;

    if (totalVotes <= 2)
    {
      includedIndices[i] = true;
    }
    else if (hierarchy[i][3] == winningParentId)
    {
      includedIndices[i] = true;
    }
  }

  return includedIndices;
}

vector<bool> CharacterAnalysis::filterBetweenLines(Mat img, vector<vector<Point> > contours, vector<Vec4i> hierarchy, vector<Point> outerPolygon, vector<bool> goodIndices)
{
  static float MIN_AREA_PERCENT_WITHIN_LINES = 0.88;
  static float MAX_DISTANCE_PERCENT_FROM_LINES = 0.15;

  vector<bool> includedIndices(contours.size());
  for (int j = 0; j < contours.size(); j++)
    includedIndices[j] = false;

  if (outerPolygon.size() == 0)
    return includedIndices;

  vector<Point> validPoints;

  // Figure out the line height
  LineSegment topLine(outerPolygon[0].x, outerPolygon[0].y, outerPolygon[1].x, outerPolygon[1].y);
  LineSegment bottomLine(outerPolygon[3].x, outerPolygon[3].y, outerPolygon[2].x, outerPolygon[2].y);

  float x = ((float) img.cols) / 2;
  Point midpoint = Point(x, bottomLine.getPointAt(x));
  Point acrossFromMidpoint = topLine.closestPointOnSegmentTo(midpoint);
  float lineHeight = distanceBetweenPoints(midpoint, acrossFromMidpoint);

  // Create a white mask for the area inside the polygon
  Mat outerMask = Mat::zeros(img.size(), CV_8U);
  Mat innerArea(img.size(), CV_8U);
  fillConvexPoly(outerMask, outerPolygon.data(), outerPolygon.size(), Scalar(255,255,255));

  // For each contour, determine if enough of it is between the lines to qualify
  for (int i = 0; i < contours.size(); i++)
  {
    if (goodIndices[i] == false)
      continue;

    innerArea.setTo(Scalar(0,0,0));
    
    drawContours(innerArea, contours,
                 i, // draw this contour
                 cv::Scalar(255,255,255), // in
                 CV_FILLED,
                 8,
                 hierarchy,
                 0
                );

    bitwise_and(innerArea, outerMask, innerArea);

    vector<vector<Point> > tempContours;
    findContours(innerArea, tempContours,
                 CV_RETR_EXTERNAL, // retrieve the external contours
                 CV_CHAIN_APPROX_SIMPLE  ); // all pixels of each contours );

    double totalArea = contourArea(contours[i]);
    double areaBetweenLines = 0;

    for (int tempContourIdx = 0; tempContourIdx < tempContours.size(); tempContourIdx++)
    {
      areaBetweenLines += contourArea(tempContours[tempContourIdx]);
    }


    
    if (areaBetweenLines / totalArea < MIN_AREA_PERCENT_WITHIN_LINES)
    {
      // Not enough area is inside the lines.
      continue;
    }
    
    
    // now check to make sure that the top and bottom of the contour are near enough to the lines
    
    // First get the high and low point for the contour
    // Remember that origin is top-left, so the top Y values are actually closer to 0.
    int highPointIndex = 0;
    int highPointValue = 999999999;
    int lowPointIndex = 0;
    int lowPointValue = 0;
    for (int cidx = 0; cidx < contours[i].size(); cidx++)
    {
      if (contours[i][cidx].y < highPointValue)
      {
	highPointIndex = cidx;
	highPointValue = contours[i][cidx].y;
      }
      if (contours[i][cidx].y > lowPointValue)
      {
	lowPointIndex = cidx;
	lowPointValue = contours[i][cidx].y;
      }
    }
    
    // Get the absolute distance from the top and bottom lines
    Point closestTopPoint = topLine.closestPointOnSegmentTo(contours[i][highPointIndex]);
    Point closestBottomPoint = bottomLine.closestPointOnSegmentTo(contours[i][lowPointIndex]);
    
    float absTopDistance = distanceBetweenPoints(closestTopPoint, contours[i][highPointIndex]);
    float absBottomDistance = distanceBetweenPoints(closestBottomPoint, contours[i][lowPointIndex]);
    
    float maxDistance = lineHeight * MAX_DISTANCE_PERCENT_FROM_LINES;
     
    if (absTopDistance < maxDistance && absBottomDistance < maxDistance)
    {
      includedIndices[i] = true;
    }

  }

  return includedIndices;
}

std::vector< bool > CharacterAnalysis::filterByOuterMask(vector< vector< Point > > contours, vector< Vec4i > hierarchy, std::vector< bool > goodIndices)
{
  float MINIMUM_PERCENT_LEFT_AFTER_MASK = 0.1;
  float MINIMUM_PERCENT_OF_CHARS_INSIDE_PLATE_MASK = 0.6;

  if (hasPlateMask == false)
    return goodIndices;

  vector<bool> passingIndices;
  for (int i = 0; i < goodIndices.size(); i++)
    passingIndices.push_back(false);

  Mat tempMaskedContour = Mat::zeros(plateMask.size(), CV_8U);
  Mat tempFullContour = Mat::zeros(plateMask.size(), CV_8U);

  int charsInsideMask = 0;
  int totalChars = 0;

  for (int i=0; i < goodIndices.size(); i++)
  {
    if (goodIndices[i] == false)
      continue;

    totalChars++;

    drawContours(tempFullContour, contours, i, Scalar(255,255,255), CV_FILLED, 8, hierarchy);
    bitwise_and(tempFullContour, plateMask, tempMaskedContour);

    float beforeMaskWhiteness = mean(tempFullContour)[0];
    float afterMaskWhiteness = mean(tempMaskedContour)[0];

    if (afterMaskWhiteness / beforeMaskWhiteness > MINIMUM_PERCENT_LEFT_AFTER_MASK)
    {
      charsInsideMask++;
      passingIndices[i] = true;
    }
  }

  if (totalChars == 0)
    return goodIndices;

  // Check to make sure that this is a valid box.  If the box is too small (e.g., 1 char is inside, and 3 are outside)
  // then don't use this to filter.
  float percentCharsInsideMask = ((float) charsInsideMask) / ((float) totalChars);
  if (percentCharsInsideMask < MINIMUM_PERCENT_OF_CHARS_INSIDE_PLATE_MASK)
    return goodIndices;

  return passingIndices;
}

bool CharacterAnalysis::isPlateInverted()
{
  Mat charMask = getCharacterMask();

  Scalar meanVal = mean(bestThreshold, charMask)[0];

  if (this->config->debugCharAnalysis)
    cout << "CharacterAnalysis, plate inverted: MEAN: " << meanVal << " : " << bestThreshold.type() << endl;

  if (meanVal[0] < 100)		// Half would be 122.5.  Give it a little extra oomf before saying it needs inversion.  Most states aren't inverted.
    return true;

  return false;
}

bool CharacterAnalysis::verifySize(Mat r, float minHeightPx, float maxHeightPx)
{
  //Char sizes 45x90
  float aspect=config->charWidthMM / config->charHeightMM;
  float charAspect= (float)r.cols/(float)r.rows;
  float error=0.35;
  //float minHeight=TEMPLATE_PLATE_HEIGHT * .35;
  //float maxHeight=TEMPLATE_PLATE_HEIGHT * .65;
  //We have a different aspect ratio for number 1, and it can be ~0.2
  float minAspect=0.2;
  float maxAspect=aspect+aspect*error;
  //area of pixels
  float area=countNonZero(r);
  //bb area
  float bbArea=r.cols*r.rows;
  //% of pixel in area
  float percPixels=area/bbArea;

  //if(DEBUG)
  //cout << "Aspect: "<< aspect << " ["<< minAspect << "," << maxAspect << "] "  << "Area "<< percPixels <<" Char aspect " << charAspect  << " Height char "<< r.rows << "\n";
  if(percPixels < 0.8 && charAspect > minAspect && charAspect < maxAspect && r.rows >= minHeightPx && r.rows < maxHeightPx)
    return true;
  else
    return false;
}

vector<Point> CharacterAnalysis::getCharArea()
{
  const int MAX = 100000;
  const int MIN= -1;

  int leftX = MAX;
  int rightX = MIN;

  for (int i = 0; i < bestContours.size(); i++)
  {
    if (bestCharSegments[i] == false)
      continue;

    for (int z = 0; z < bestContours[i].size(); z++)
    {
      if (bestContours[i][z].x < leftX)
        leftX = bestContours[i][z].x;
      if (bestContours[i][z].x > rightX)
        rightX = bestContours[i][z].x;
    }
  }

  vector<Point> charArea;
  if (leftX != MAX && rightX != MIN)
  {
    Point tl(leftX, topLine.getPointAt(leftX));
    Point tr(rightX, topLine.getPointAt(rightX));
    Point br(rightX, bottomLine.getPointAt(rightX));
    Point bl(leftX, bottomLine.getPointAt(leftX));
    charArea.push_back(tl);
    charArea.push_back(tr);
    charArea.push_back(br);
    charArea.push_back(bl);
  }

  return charArea;
}
