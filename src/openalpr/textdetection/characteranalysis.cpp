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

#include "characteranalysis.h"

using namespace cv;
using namespace std;

CharacterAnalysis::CharacterAnalysis(PipelineData* pipeline_data)
{
  this->pipeline_data = pipeline_data;
  this->config = pipeline_data->config;

  this->isTwoLine = true;

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

  for (uint i = 0; i < pipeline_data->thresholds.size(); i++)
  {
    TextContours tc(pipeline_data->thresholds[i]);

    allTextContours.push_back(tc);
  }

  if (config->debugTiming)
  {
    timespec endTime;
    getTime(&endTime);
    cout << "  -- Character Analysis Find Contours Time: " << diffclock(startTime, endTime) << "ms." << endl;
  }
  //Mat img_equalized = equalizeBrightness(img_gray);

  getTime(&startTime);

  for (uint i = 0; i < pipeline_data->thresholds.size(); i++)
  {
    this->filter(pipeline_data->thresholds[i], allTextContours[i]);

    if (config->debugCharAnalysis)
      cout << "Threshold " << i << " had " << allTextContours[i].getGoodIndicesCount() << " good indices." << endl;
  }

  if (config->debugTiming)
  {
    timespec endTime;
    getTime(&endTime);
    cout << "  -- Character Analysis Filter Time: " << diffclock(startTime, endTime) << "ms." << endl;
  }

  PlateMask plateMask(pipeline_data);
  plateMask.findOuterBoxMask(allTextContours);
  
  pipeline_data->hasPlateBorder = plateMask.hasPlateMask;
  pipeline_data->plateBorderMask = plateMask.getMask();

  if (plateMask.hasPlateMask)
  {
    // Filter out bad contours now that we have an outer box mask...
    for (uint i = 0; i < pipeline_data->thresholds.size(); i++)
    {
      filterByOuterMask(allTextContours[i]);
    }
  }

  int bestFitScore = -1;
  int bestFitIndex = -1;
  for (uint i = 0; i < pipeline_data->thresholds.size(); i++)
  {
    //vector<bool> goodIndices = this->filter(thresholds[i], allContours[i], allHierarchy[i]);
    //charSegments.push_back(goodIndices);

    int segmentCount = allTextContours[i].getGoodIndicesCount();

    if (segmentCount > bestFitScore)
    {
      bestFitScore = segmentCount;
      bestFitIndex = i;
      bestThreshold = pipeline_data->thresholds[i];
      bestContours = allTextContours[i];
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
    for (uint i = 0; i < bestContours.size(); i++)
    {
      if (bestContours.goodIndices[i])
        allowedContours.push_back(bestContours.contours[i]);
    }

    drawContours(img_contours, bestContours.contours,
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

  this->linePolygon =  getBestVotedLines(pipeline_data->crop_gray, bestContours);

  if (this->linePolygon.size() > 0)
  {
    this->topLine = LineSegment(this->linePolygon[0].x, this->linePolygon[0].y, this->linePolygon[1].x, this->linePolygon[1].y);
    this->bottomLine = LineSegment(this->linePolygon[3].x, this->linePolygon[3].y, this->linePolygon[2].x, this->linePolygon[2].y);
    //this->charArea = getCharSegmentsBetweenLines(bestThreshold, bestContours, this->linePolygon);
    filterBetweenLines(bestThreshold, bestContours, linePolygon);

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




Mat CharacterAnalysis::getCharacterMask()
{
  Mat charMask = Mat::zeros(bestThreshold.size(), CV_8U);

  for (uint i = 0; i < bestContours.size(); i++)
  {
    if (bestContours.goodIndices[i] == false)
      continue;

    drawContours(charMask, bestContours.contours,
                 i, // draw this contour
                 cv::Scalar(255,255,255), // in
                 CV_FILLED,
                 8,
                 bestContours.hierarchy,
                 1
                );

  }

  return charMask;
}

// Returns a polygon "stripe" across the width of the character region.  The lines are voted and the polygon starts at 0 and extends to image width
vector<Point> CharacterAnalysis::getBestVotedLines(Mat img, TextContours textContours)
{
  //if (this->debug)
  //  cout << "CharacterAnalysis::getBestVotedLines" << endl;

  vector<Point> bestStripe;

  vector<Rect> charRegions;

  for (uint i = 0; i < textContours.size(); i++)
  {
    if (textContours.goodIndices[i])
      charRegions.push_back(boundingRect(textContours.contours[i]));
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
    for (uint i = 0; i < charRegions.size() - 1; i++)
    {
      for (uint k = i+1; k < charRegions.size(); k++)
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
      for (uint charidx = 0; charidx < charRegions.size(); charidx++)
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

void CharacterAnalysis::filter(Mat img, TextContours& textContours)
{
  static int STARTING_MIN_HEIGHT = round (((float) img.rows) * config->charAnalysisMinPercent);
  static int STARTING_MAX_HEIGHT = round (((float) img.rows) * (config->charAnalysisMinPercent + config->charAnalysisHeightRange));
  static int HEIGHT_STEP = round (((float) img.rows) * config->charAnalysisHeightStepSize);
  static int NUM_STEPS = config->charAnalysisNumSteps;

  int bestFitScore = -1;
  
  vector<bool> bestIndices;
  
  for (int i = 0; i < NUM_STEPS; i++)
  {
    
    //vector<bool> goodIndices(contours.size());
    for (uint z = 0; z < textContours.size(); z++) textContours.goodIndices[z] = true;

    this->filterByBoxSize(textContours, STARTING_MIN_HEIGHT + (i * HEIGHT_STEP), STARTING_MAX_HEIGHT + (i * HEIGHT_STEP));

    int goodIndices = textContours.getGoodIndicesCount();
    if ( goodIndices == 0 || goodIndices <= bestFitScore)	// Don't bother doing more filtering if we already lost...
      continue;
    
    this->filterContourHoles(textContours);

    goodIndices = textContours.getGoodIndicesCount();
    if ( goodIndices == 0 || goodIndices <= bestFitScore)	// Don't bother doing more filtering if we already lost...
      continue;
    
    vector<Point> lines = getBestVotedLines(img, textContours);
    this->filterBetweenLines(img, textContours, lines);

    int segmentCount = textContours.getGoodIndicesCount();

    if (segmentCount > bestFitScore)
    {
      bestFitScore = segmentCount;
      bestIndices = textContours.getIndicesCopy();
    }
  }

  textContours.setIndices(bestIndices);
}

// Goes through the contours for the plate and picks out possible char segments based on min/max height
void CharacterAnalysis::filterByBoxSize(TextContours& textContours, int minHeightPx, int maxHeightPx)
{
  float idealAspect=config->charWidthMM / config->charHeightMM;
  float aspecttolerance=0.25;


  for (uint i = 0; i < textContours.size(); i++)
  {
    if (textContours.goodIndices[i] == false)
      continue;

    textContours.goodIndices[i] = false;  // Set it to not included unless it proves valid
    
    //Create bounding rect of object
    Rect mr= boundingRect(textContours.contours[i]);

    float minWidth = mr.height * 0.2;
    //Crop image
    //Mat auxRoi(img, mr);
    if(mr.height >= minHeightPx && mr.height <= maxHeightPx && mr.width > minWidth)
    {
      float charAspect= (float)mr.width/(float)mr.height;

      if (abs(charAspect - idealAspect) < aspecttolerance)
        textContours.goodIndices[i] = true;
    }
  }

}

void CharacterAnalysis::filterContourHoles(TextContours& textContours)
{

  for (uint i = 0; i < textContours.size(); i++)
  {
    if (textContours.goodIndices[i] == false)
      continue;

    textContours.goodIndices[i] = false;  // Set it to not included unless it proves valid
    
    int parentIndex = textContours.hierarchy[i][3];

    if (parentIndex >= 0 && textContours.goodIndices[parentIndex])
    {
      // this contour is a child of an already identified contour.  REMOVE it
      if (this->config->debugCharAnalysis)
      {
        cout << "filterContourHoles: contour index: " << i << endl;
      }
    }
    else
    {
      textContours.goodIndices[i] = true;
    }
  }

}

// Goes through the contours for the plate and picks out possible char segments based on min/max height
// returns a vector of indices corresponding to valid contours
void CharacterAnalysis::filterByParentContour( TextContours& textContours)
{

  vector<int> parentIDs;
  vector<int> votes;

  for (uint i = 0; i < textContours.size(); i++)
  {
    if (textContours.goodIndices[i] == false)
      continue;

    textContours.goodIndices[i] = false;  // Set it to not included unless it proves 
    
    int voteIndex = -1;
    int parentID = textContours.hierarchy[i][3];
    // check if parentID is already in the lsit
    for (uint j = 0; j < parentIDs.size(); j++)
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
  for (uint i = 0; i < parentIDs.size(); i++)
  {
    if (votes[i] > highestVotes)
    {
      winningParentId = parentIDs[i];
      highestVotes = votes[i];
    }
    totalVotes += votes[i];
  }

  // Now filter out all the contours with a different parent ID (assuming the totalVotes > 2)
  for (uint i = 0; i < textContours.size(); i++)
  {
    if (textContours.goodIndices[i] == false)
      continue;

    if (totalVotes <= 2)
    {
      textContours.goodIndices[i] = true;
    }
    else if (textContours.hierarchy[i][3] == winningParentId)
    {
      textContours.goodIndices[i] = true;
    }
  }

}

void CharacterAnalysis::filterBetweenLines(Mat img, TextContours& textContours, vector<Point> outerPolygon )
{
  static float MIN_AREA_PERCENT_WITHIN_LINES = 0.88;
  static float MAX_DISTANCE_PERCENT_FROM_LINES = 0.15;

  if (outerPolygon.size() == 0)
    return;

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
  for (uint i = 0; i < textContours.size(); i++)
  {
    if (textContours.goodIndices[i] == false)
      continue;
    
    textContours.goodIndices[i] = false;  // Set it to not included unless it proves 

    innerArea.setTo(Scalar(0,0,0));
    
    drawContours(innerArea, textContours.contours,
                 i, // draw this contour
                 cv::Scalar(255,255,255), // in
                 CV_FILLED,
                 8,
                 textContours.hierarchy,
                 0
                );

    bitwise_and(innerArea, outerMask, innerArea);

    vector<vector<Point> > tempContours;
    findContours(innerArea, tempContours,
                 CV_RETR_EXTERNAL, // retrieve the external contours
                 CV_CHAIN_APPROX_SIMPLE  ); // all pixels of each contours );

    double totalArea = contourArea(textContours.contours[i]);
    double areaBetweenLines = 0;

    for (uint tempContourIdx = 0; tempContourIdx < tempContours.size(); tempContourIdx++)
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
    for (uint cidx = 0; cidx < textContours.contours[i].size(); cidx++)
    {
      if (textContours.contours[i][cidx].y < highPointValue)
      {
	highPointIndex = cidx;
	highPointValue = textContours.contours[i][cidx].y;
      }
      if (textContours.contours[i][cidx].y > lowPointValue)
      {
	lowPointIndex = cidx;
	lowPointValue = textContours.contours[i][cidx].y;
      }
    }
    
    // Get the absolute distance from the top and bottom lines
    Point closestTopPoint = topLine.closestPointOnSegmentTo(textContours.contours[i][highPointIndex]);
    Point closestBottomPoint = bottomLine.closestPointOnSegmentTo(textContours.contours[i][lowPointIndex]);
    
    float absTopDistance = distanceBetweenPoints(closestTopPoint, textContours.contours[i][highPointIndex]);
    float absBottomDistance = distanceBetweenPoints(closestBottomPoint, textContours.contours[i][lowPointIndex]);
    
    float maxDistance = lineHeight * MAX_DISTANCE_PERCENT_FROM_LINES;
     
    if (absTopDistance < maxDistance && absBottomDistance < maxDistance)
    {
      textContours.goodIndices[i] = true;
    }

  }

}

void CharacterAnalysis::filterByOuterMask(TextContours& textContours)
{
  float MINIMUM_PERCENT_LEFT_AFTER_MASK = 0.1;
  float MINIMUM_PERCENT_OF_CHARS_INSIDE_PLATE_MASK = 0.6;

  if (this->pipeline_data->hasPlateBorder == false)
    return;


  cv::Mat plateMask = pipeline_data->plateBorderMask;
  
  Mat tempMaskedContour = Mat::zeros(plateMask.size(), CV_8U);
  Mat tempFullContour = Mat::zeros(plateMask.size(), CV_8U);

  int charsInsideMask = 0;
  int totalChars = 0;

  vector<bool> originalindices;
  for (uint i = 0; i < textContours.size(); i++)
    originalindices.push_back(textContours.goodIndices[i]);
  
  for (uint i=0; i < textContours.size(); i++)
  {
    if (textContours.goodIndices[i] == false)
      continue;

    totalChars++;

    drawContours(tempFullContour, textContours.contours, i, Scalar(255,255,255), CV_FILLED, 8, textContours.hierarchy);
    bitwise_and(tempFullContour, plateMask, tempMaskedContour);

    float beforeMaskWhiteness = mean(tempFullContour)[0];
    float afterMaskWhiteness = mean(tempMaskedContour)[0];

    if (afterMaskWhiteness / beforeMaskWhiteness > MINIMUM_PERCENT_LEFT_AFTER_MASK)
    {
      charsInsideMask++;
      textContours.goodIndices[i] = true;
    }
  }

  if (totalChars == 0)
  {
    textContours.goodIndices = originalindices;
    return;
  }

  // Check to make sure that this is a valid box.  If the box is too small (e.g., 1 char is inside, and 3 are outside)
  // then don't use this to filter.
  float percentCharsInsideMask = ((float) charsInsideMask) / ((float) totalChars);
  if (percentCharsInsideMask < MINIMUM_PERCENT_OF_CHARS_INSIDE_PLATE_MASK)
  {
    textContours.goodIndices = originalindices;
    return;
  }

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

  for (uint i = 0; i < bestContours.size(); i++)
  {
    if (bestContours.goodIndices[i] == false)
      continue;

    for (uint z = 0; z < bestContours.contours[i].size(); z++)
    {
      if (bestContours.contours[i][z].x < leftX)
        leftX = bestContours.contours[i][z].x;
      if (bestContours.contours[i][z].x > rightX)
        rightX = bestContours.contours[i][z].x;
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
