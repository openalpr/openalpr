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

#include <opencv2/imgproc/imgproc.hpp>

#include "characteranalysis.h"
#include "linefinder.h"

using namespace cv;
using namespace std;

namespace alpr
{

  bool sort_text_line(TextLine i, TextLine j) { return (i.topLine.p1.y < j.topLine.p1.y); }

  CharacterAnalysis::CharacterAnalysis(PipelineData* pipeline_data)
  {
    this->pipeline_data = pipeline_data;
    this->config = pipeline_data->config;

    if (this->config->debugCharAnalysis)
      cout << "Starting CharacterAnalysis identification" << endl;

    this->analyze();
  }

  CharacterAnalysis::~CharacterAnalysis()
  {

  }

  void CharacterAnalysis::analyze()
  {
    timespec startTime;
    getTimeMonotonic(&startTime);

    if (config->always_invert)
      bitwise_not(pipeline_data->crop_gray, pipeline_data->crop_gray);

    pipeline_data->clearThresholds();
    pipeline_data->thresholds = produceThresholds(pipeline_data->crop_gray, config);

    timespec contoursStartTime;
    getTimeMonotonic(&contoursStartTime);

    pipeline_data->textLines.clear();

    for (unsigned int i = 0; i < pipeline_data->thresholds.size(); i++)
    {
      TextContours tc(pipeline_data->thresholds[i]);

      allTextContours.push_back(tc);
    }

    if (config->debugTiming)
    {
      timespec contoursEndTime;
      getTimeMonotonic(&contoursEndTime);
      cout << "  -- Character Analysis Find Contours Time: " << diffclock(contoursStartTime, contoursEndTime) << "ms." << endl;
    }
    //Mat img_equalized = equalizeBrightness(img_gray);

    timespec filterStartTime;
    getTimeMonotonic(&filterStartTime);

    for (unsigned int i = 0; i < pipeline_data->thresholds.size(); i++)
    {
      this->filter(pipeline_data->thresholds[i], allTextContours[i]);

      if (config->debugCharAnalysis)
        cout << "Threshold " << i << " had " << allTextContours[i].getGoodIndicesCount() << " good indices." << endl;
    }

    if (config->debugTiming)
    {
      timespec filterEndTime;
      getTimeMonotonic(&filterEndTime);
      cout << "  -- Character Analysis Filter Time: " << diffclock(filterStartTime, filterEndTime) << "ms." << endl;
    }

    PlateMask plateMask(pipeline_data);
    plateMask.findOuterBoxMask(allTextContours);

    pipeline_data->hasPlateBorder = plateMask.hasPlateMask;
    pipeline_data->plateBorderMask = plateMask.getMask();

    if (plateMask.hasPlateMask)
    {
      // Filter out bad contours now that we have an outer box mask...
      for (unsigned int i = 0; i < pipeline_data->thresholds.size(); i++)
      {
        filterByOuterMask(allTextContours[i]);
      }
    }

    int bestFitScore = -1;
    int bestFitIndex = -1;
    for (unsigned int i = 0; i < pipeline_data->thresholds.size(); i++)
    {

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
    {
      pipeline_data->disqualified = true;
      pipeline_data->disqualify_reason = "Low best fit score in characteranalysis";
      return;
    }

    //getColorMask(img, allContours, allHierarchy, charSegments);

    if (this->config->debugCharAnalysis)
    {
      Mat img_contours = bestContours.drawDebugImage(bestThreshold);

      displayImage(config, "Matching Contours", img_contours);
    }

    if (config->auto_invert)
      pipeline_data->plate_inverted = isPlateInverted();
    else
      pipeline_data->plate_inverted = config->always_invert;

    if (config->debugGeneral)
      cout << "Plate inverted: " << pipeline_data->plate_inverted << endl;
    
    // Invert multiline plates and redo the thresholds before finding the second line
    if (config->multiline && config->auto_invert && pipeline_data->plate_inverted)
    {
      bitwise_not(pipeline_data->crop_gray, pipeline_data->crop_gray);
      pipeline_data->thresholds = produceThresholds(pipeline_data->crop_gray, pipeline_data->config);
    }
      
    
    LineFinder lf(pipeline_data);
    vector<vector<Point> > linePolygons = lf.findLines(pipeline_data->crop_gray, bestContours);

    vector<TextLine> tempTextLines;
    for (unsigned int i = 0; i < linePolygons.size(); i++)
    {
      vector<Point> linePolygon = linePolygons[i];

      LineSegment topLine = LineSegment(linePolygon[0].x, linePolygon[0].y, linePolygon[1].x, linePolygon[1].y);
      LineSegment bottomLine = LineSegment(linePolygon[3].x, linePolygon[3].y, linePolygon[2].x, linePolygon[2].y);

      vector<Point> textArea = getCharArea(topLine, bottomLine);

      TextLine textLine(textArea, linePolygon, pipeline_data->crop_gray.size());

      tempTextLines.push_back(textLine);
    }

    filterBetweenLines(bestThreshold, bestContours, tempTextLines);

    // Sort the lines from top to bottom.
    std::sort(tempTextLines.begin(), tempTextLines.end(), sort_text_line);

    // Now that we've filtered a few more contours, re-do the text area.
    for (unsigned int i = 0; i < tempTextLines.size(); i++)
    {
      vector<Point> updatedTextArea = getCharArea(tempTextLines[i].topLine, tempTextLines[i].bottomLine);
      vector<Point> linePolygon = tempTextLines[i].linePolygon;
      if (updatedTextArea.size() > 0 && linePolygon.size() > 0)
      {
        pipeline_data->textLines.push_back(TextLine(updatedTextArea, linePolygon, pipeline_data->crop_gray.size()));
      }

    }


    if (pipeline_data->textLines.size() > 0)
    {
      int confidenceDrainers = 0;
      int charSegmentCount = this->bestContours.getGoodIndicesCount();
      if (charSegmentCount == 1)
        confidenceDrainers += 91;
      else if (charSegmentCount < 5)
        confidenceDrainers += (5 - charSegmentCount) * 10;

      // Use the angle for the first line -- assume they'll always be parallel for multi-line plates
      int absangle = abs(pipeline_data->textLines[0].topLine.angle);
      if (absangle > config->maxPlateAngleDegrees)
        confidenceDrainers += 91;
      else if (absangle > 1)
        confidenceDrainers += (config->maxPlateAngleDegrees - absangle) ;

      // If a multiline plate has only one line, disqualify
      if (pipeline_data->isMultiline && pipeline_data->textLines.size() < 2)
      {
        if (config->debugCharAnalysis)
          std::cout << "Did not detect multiple lines on multi-line plate" << std::endl;
        confidenceDrainers += 95;
      }

      if (confidenceDrainers >= 90)
      {
        pipeline_data->disqualified = true;
        pipeline_data->disqualify_reason = "Low confidence in characteranalysis";
      }
      else
      {
        float confidence = 100 - confidenceDrainers;
        pipeline_data->confidence_weights.setScore("CHARACTER_ANALYSIS_SCORE", confidence, 1.0);
      }
    }
    else
    {
        pipeline_data->disqualified = true;
        pipeline_data->disqualify_reason = "No text lines found in characteranalysis";
    }

    if (config->debugTiming)
    {
      timespec endTime;
      getTimeMonotonic(&endTime);
      cout << "Character Analysis Time: " << diffclock(startTime, endTime) << "ms." << endl;
    }

    // Draw debug dashboard
    if (this->pipeline_data->config->debugCharAnalysis && pipeline_data->textLines.size() > 0)
    {
      vector<Mat> tempDash;
      for (unsigned int z = 0; z < pipeline_data->thresholds.size(); z++)
      {
        Mat tmp(pipeline_data->thresholds[z].size(), pipeline_data->thresholds[z].type());
        pipeline_data->thresholds[z].copyTo(tmp);
        cvtColor(tmp, tmp, CV_GRAY2BGR);

        tempDash.push_back(tmp);
      }

      Mat bestVal(this->bestThreshold.size(), this->bestThreshold.type());
      this->bestThreshold.copyTo(bestVal);
      cvtColor(bestVal, bestVal, CV_GRAY2BGR);

      for (unsigned int z = 0; z < this->bestContours.size(); z++)
      {
        Scalar dcolor(255,0,0);
        if (this->bestContours.goodIndices[z])
          dcolor = Scalar(0,255,0);
        drawContours(bestVal, this->bestContours.contours, z, dcolor, 1);
      }
      tempDash.push_back(bestVal);
      displayImage(config, "Character Region Step 1 Thresholds", drawImageDashboard(tempDash, bestVal.type(), 3));
    }
  }




  Mat CharacterAnalysis::getCharacterMask()
  {
    Mat charMask = Mat::zeros(bestThreshold.size(), CV_8U);

    for (unsigned int i = 0; i < bestContours.size(); i++)
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


  void CharacterAnalysis::filter(Mat img, TextContours& textContours)
  {
    int STARTING_MIN_HEIGHT = round (((float) img.rows) * config->charAnalysisMinPercent);
    int STARTING_MAX_HEIGHT = round (((float) img.rows) * (config->charAnalysisMinPercent + config->charAnalysisHeightRange));
    int HEIGHT_STEP = round (((float) img.rows) * config->charAnalysisHeightStepSize);
    int NUM_STEPS = config->charAnalysisNumSteps;

    int bestFitScore = -1;

    vector<bool> bestIndices;
     
    for (int i = 0; i < NUM_STEPS; i++)
    {

      //vector<bool> goodIndices(contours.size());
      for (unsigned int z = 0; z < textContours.size(); z++) textContours.goodIndices[z] = true;

      this->filterByBoxSize(textContours, STARTING_MIN_HEIGHT + (i * HEIGHT_STEP), STARTING_MAX_HEIGHT + (i * HEIGHT_STEP));

      int goodIndices = textContours.getGoodIndicesCount();
      if ( goodIndices == 0 || goodIndices <= bestFitScore)	// Don't bother doing more filtering if we already lost...
        continue;

      this->filterContourHoles(textContours);

      goodIndices = textContours.getGoodIndicesCount();
      if ( goodIndices == 0 || goodIndices <= bestFitScore)	// Don't bother doing more filtering if we already lost...
        continue;


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
    // For multiline plates, we want to target the biggest line for character analysis, since it should be easier to spot.
    float larger_char_height_mm = 0;
    float larger_char_width_mm = 0;
    for (unsigned int i = 0; i < config->charHeightMM.size(); i++)
    {
      if (config->charHeightMM[i] > larger_char_height_mm)
      {
        larger_char_height_mm = config->charHeightMM[i];
        larger_char_width_mm = config->charWidthMM[i];
      }
    }
    
    float idealAspect=larger_char_width_mm / larger_char_height_mm;
    float aspecttolerance=0.25;


    for (unsigned int i = 0; i < textContours.size(); i++)
    {
      if (textContours.goodIndices[i] == false)
        continue;

      textContours.goodIndices[i] = false;  // Set it to not included unless it proves valid

      //Create bounding rect of object
      Rect mr= boundingRect(textContours.contours[i]);

      float minWidth = mr.height * 0.2;
      //Crop image

      //cout << "Height: " << minHeightPx << " - " << mr.height << " - " << maxHeightPx << " ////// Width: " << mr.width << " - " << minWidth << endl;
      if(mr.height >= minHeightPx && mr.height <= maxHeightPx && mr.width > minWidth)
      {
        float charAspect= (float)mr.width/(float)mr.height;

        //cout << "  -- stage 2 aspect: " << abs(charAspect) << " - " << aspecttolerance << endl;
        if (abs(charAspect - idealAspect) < aspecttolerance)
          textContours.goodIndices[i] = true;
      }
    }

  }

  void CharacterAnalysis::filterContourHoles(TextContours& textContours)
  {

    for (unsigned int i = 0; i < textContours.size(); i++)
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

    for (unsigned int i = 0; i < textContours.size(); i++)
    {
      if (textContours.goodIndices[i] == false)
        continue;

      textContours.goodIndices[i] = false;  // Set it to not included unless it proves 

      int voteIndex = -1;
      int parentID = textContours.hierarchy[i][3];
      // check if parentID is already in the lsit
      for (unsigned int j = 0; j < parentIDs.size(); j++)
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
    for (unsigned int i = 0; i < parentIDs.size(); i++)
    {
      if (votes[i] > highestVotes)
      {
        winningParentId = parentIDs[i];
        highestVotes = votes[i];
      }
      totalVotes += votes[i];
    }

    // Now filter out all the contours with a different parent ID (assuming the totalVotes > 2)
    for (unsigned int i = 0; i < textContours.size(); i++)
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

  void CharacterAnalysis::filterBetweenLines(Mat img, TextContours& textContours, vector<TextLine> textLines )
  {
    static float MIN_AREA_PERCENT_WITHIN_LINES = 0.88;
    static float MAX_DISTANCE_PERCENT_FROM_LINES = 0.15;

    if (textLines.size() == 0)
      return;

    vector<Point> validPoints;


    // Create a white mask for the area inside the polygon
    Mat outerMask = Mat::zeros(img.size(), CV_8U);

    for (unsigned int i = 0; i < textLines.size(); i++)
      fillConvexPoly(outerMask, textLines[i].linePolygon.data(), textLines[i].linePolygon.size(), Scalar(255,255,255));

    // For each contour, determine if enough of it is between the lines to qualify
    for (unsigned int i = 0; i < textContours.size(); i++)
    {
      if (textContours.goodIndices[i] == false)
        continue;

      float percentInsideMask = getContourAreaPercentInsideMask(outerMask, 
              textContours.contours,
              textContours.hierarchy, 
              (int) i);



      if (percentInsideMask < MIN_AREA_PERCENT_WITHIN_LINES)
      {
        // Not enough area is inside the lines.
        if (config->debugCharAnalysis)
          cout << "Rejecting due to insufficient area" << endl;
        textContours.goodIndices[i] = false; 

        continue;
      }


      // now check to make sure that the top and bottom of the contour are near enough to the lines

      // First get the high and low point for the contour
      // Remember that origin is top-left, so the top Y values are actually closer to 0.
      Rect brect = boundingRect(textContours.contours[i]);
      int xmiddle = brect.x + (brect.width / 2);
      Point topMiddle = Point(xmiddle, brect.y);
      Point botMiddle = Point(xmiddle, brect.y+brect.height);

      // Get the absolute distance from the top and bottom lines

      for (unsigned int i = 0; i < textLines.size(); i++)
      {
        Point closestTopPoint = textLines[i].topLine.closestPointOnSegmentTo(topMiddle);
        Point closestBottomPoint = textLines[i].bottomLine.closestPointOnSegmentTo(botMiddle);

        float absTopDistance = distanceBetweenPoints(closestTopPoint, topMiddle);
        float absBottomDistance = distanceBetweenPoints(closestBottomPoint, botMiddle);

        float maxDistance = textLines[i].lineHeight * MAX_DISTANCE_PERCENT_FROM_LINES;

        if (absTopDistance < maxDistance && absBottomDistance < maxDistance)
        {
          // It's ok, leave it as-is.
        }
        else
        {

          textContours.goodIndices[i] = false; 
          if (config->debugCharAnalysis)
            cout << "Rejecting due to top/bottom points that are out of range" << endl;
        }
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
    for (unsigned int i = 0; i < textContours.size(); i++)
      originalindices.push_back(textContours.goodIndices[i]);

    for (unsigned int i=0; i < textContours.size(); i++)
    {
      if (textContours.goodIndices[i] == false)
        continue;

      totalChars++;

	  tempFullContour = Mat::zeros(plateMask.size(), CV_8U);
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


  vector<Point> CharacterAnalysis::getCharArea(LineSegment topLine, LineSegment bottomLine)
  {
    const int MAX = 100000;
    const int MIN= -1;

    int leftX = MAX;
    int rightX = MIN;

    for (unsigned int i = 0; i < bestContours.size(); i++)
    {
      if (bestContours.goodIndices[i] == false)
        continue;

      for (unsigned int z = 0; z < bestContours.contours[i].size(); z++)
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

}
