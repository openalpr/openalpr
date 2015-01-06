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

#include "charactersegmenter.h"

using namespace cv;
using namespace std;

namespace alpr
{

  CharacterSegmenter::CharacterSegmenter(PipelineData* pipeline_data)
  {
    this->pipeline_data = pipeline_data;
    this->config = pipeline_data->config;

    this->confidence = 0;

    if (this->config->debugCharSegmenter)
      cout << "Starting CharacterSegmenter" << endl;

    //CharacterRegion charRegion(img, debug);

    timespec startTime;
    getTime(&startTime);

    if (pipeline_data->plate_inverted)
      bitwise_not(pipeline_data->crop_gray, pipeline_data->crop_gray);
    pipeline_data->clearThresholds();
    pipeline_data->thresholds = produceThresholds(pipeline_data->crop_gray, config);

    // TODO: Perhaps a bilateral filter would be better here.
    medianBlur(pipeline_data->crop_gray, pipeline_data->crop_gray, 3);

    if (this->config->debugCharSegmenter)
      cout << "Segmenter: inverted: " << pipeline_data->plate_inverted << endl;

    if (pipeline_data->plate_inverted)
      bitwise_not(pipeline_data->crop_gray, pipeline_data->crop_gray);


    if (this->config->debugCharSegmenter)
    {
      displayImage(config, "CharacterSegmenter  Thresholds", drawImageDashboard(pipeline_data->thresholds, CV_8U, 3));
    }

  //  if (this->config->debugCharSegmenter && pipeline_data->textLines.size() > 0)
  //  {
  //    Mat img_contours(charAnalysis->bestThreshold.size(), CV_8U);
  //    charAnalysis->bestThreshold.copyTo(img_contours);
  //    cvtColor(img_contours, img_contours, CV_GRAY2RGB);
  //
  //    vector<vector<Point> > allowedContours;
  //    for (unsigned int i = 0; i < charAnalysis->bestContours.size(); i++)
  //    {
  //      if (charAnalysis->bestContours.goodIndices[i])
  //        allowedContours.push_back(charAnalysis->bestContours.contours[i]);
  //    }
  //
  //    drawContours(img_contours, charAnalysis->bestContours.contours,
  //                 -1, // draw all contours
  //                 cv::Scalar(255,0,0), // in blue
  //                 1); // with a thickness of 1
  //
  //    drawContours(img_contours, allowedContours,
  //                 -1, // draw all contours
  //                 cv::Scalar(0,255,0), // in green
  //                 1); // with a thickness of 1
  //
  //
  //    line(img_contours, pipeline_data->textLines[0].linePolygon[0], pipeline_data->textLines[0].linePolygon[1], Scalar(255, 0, 255), 1);
  //    line(img_contours, pipeline_data->textLines[0].linePolygon[3], pipeline_data->textLines[0].linePolygon[2], Scalar(255, 0, 255), 1);
  //
  //
  //    Mat bordered = addLabel(img_contours, "Best Contours");
  //    imgDbgGeneral.push_back(bordered);
  //  }



    for (unsigned int lineidx = 0; lineidx < pipeline_data->textLines.size(); lineidx++)
    {
      this->top = pipeline_data->textLines[lineidx].topLine;
      this->bottom = pipeline_data->textLines[lineidx].bottomLine;

      float avgCharHeight = pipeline_data->textLines[lineidx].lineHeight;
      float height_to_width_ratio = pipeline_data->config->charHeightMM / pipeline_data->config->charWidthMM;
      float avgCharWidth = avgCharHeight / height_to_width_ratio;

      removeSmallContours(pipeline_data->thresholds, avgCharHeight, pipeline_data->textLines[lineidx]);

      // Do the histogram analysis to figure out char regions

      timespec startTime;
      getTime(&startTime);

      vector<Mat> allHistograms;

      vector<Rect> lineBoxes;
      for (unsigned int i = 0; i < pipeline_data->thresholds.size(); i++)
      {
        Mat histogramMask = Mat::zeros(pipeline_data->thresholds[i].size(), CV_8U);

        fillConvexPoly(histogramMask, pipeline_data->textLines[lineidx].linePolygon.data(), pipeline_data->textLines[lineidx].linePolygon.size(), Scalar(255,255,255));

        VerticalHistogram vertHistogram(pipeline_data->thresholds[i], histogramMask);

        if (this->config->debugCharSegmenter)
        {
          Mat histoCopy(vertHistogram.histoImg.size(), vertHistogram.histoImg.type());
          //vertHistogram.copyTo(histoCopy);
          cvtColor(vertHistogram.histoImg, histoCopy, CV_GRAY2RGB);

          string label = "threshold: " + toString(i);
          allHistograms.push_back(addLabel(histoCopy, label));
        }

  //
        float score = 0;
        vector<Rect> charBoxes = getHistogramBoxes(vertHistogram, avgCharWidth, avgCharHeight, &score);

        if (this->config->debugCharSegmenter)
        {
          for (unsigned int cboxIdx = 0; cboxIdx < charBoxes.size(); cboxIdx++)
          {
            rectangle(allHistograms[i], charBoxes[cboxIdx], Scalar(0, 255, 0));
          }

          Mat histDashboard = drawImageDashboard(allHistograms, allHistograms[0].type(), 1);
          displayImage(config, "Char seg histograms", histDashboard);
        }

        for (unsigned int z = 0; z < charBoxes.size(); z++)
          lineBoxes.push_back(charBoxes[z]);
        //drawAndWait(&histogramMask);
      }

      float medianCharWidth = avgCharWidth;
      vector<int> widthValues;
      // Compute largest char width
      for (unsigned int i = 0; i < lineBoxes.size(); i++)
      {
        widthValues.push_back(lineBoxes[i].width);
      }

      medianCharWidth = median(widthValues.data(), widthValues.size());

      if (config->debugTiming)
      {
        timespec endTime;
        getTime(&endTime);
        cout << "  -- Character Segmentation Create and Score Histograms Time: " << diffclock(startTime, endTime) << "ms." << endl;
      }

      vector<Rect> candidateBoxes = getBestCharBoxes(pipeline_data->thresholds[0], lineBoxes, medianCharWidth);

      if (this->config->debugCharSegmenter)
      {
        // Setup the dashboard images to show the cleaning filters
        for (unsigned int i = 0; i < pipeline_data->thresholds.size(); i++)
        {
          Mat cleanImg = Mat::zeros(pipeline_data->thresholds[i].size(), pipeline_data->thresholds[i].type());
          Mat boxMask = getCharBoxMask(pipeline_data->thresholds[i], candidateBoxes);
          pipeline_data->thresholds[i].copyTo(cleanImg);
          bitwise_and(cleanImg, boxMask, cleanImg);
          cvtColor(cleanImg, cleanImg, CV_GRAY2BGR);

          for (unsigned int c = 0; c < candidateBoxes.size(); c++)
            rectangle(cleanImg, candidateBoxes[c], Scalar(0, 255, 0), 1);
          imgDbgCleanStages.push_back(cleanImg);
        }
      }

      getTime(&startTime);

      filterEdgeBoxes(pipeline_data->thresholds, candidateBoxes, medianCharWidth, avgCharHeight);
      candidateBoxes = filterMostlyEmptyBoxes(pipeline_data->thresholds, candidateBoxes);
      candidateBoxes = combineCloseBoxes(candidateBoxes, medianCharWidth);
      cleanMostlyFullBoxes(pipeline_data->thresholds, candidateBoxes);

      candidateBoxes = filterMostlyEmptyBoxes(pipeline_data->thresholds, candidateBoxes);

      for (unsigned int cbox = 0; cbox < candidateBoxes.size(); cbox++)
        pipeline_data->charRegions.push_back(candidateBoxes[cbox]);

      if (config->debugTiming)
      {
        timespec endTime;
        getTime(&endTime);
        cout << "  -- Character Segmentation Box cleaning/filtering Time: " << diffclock(startTime, endTime) << "ms." << endl;
      }

      if (this->config->debugCharSegmenter)
      {
        Mat imgDash = drawImageDashboard(pipeline_data->thresholds, CV_8U, 3);
        displayImage(config, "Segmentation after cleaning", imgDash);

        Mat generalDash = drawImageDashboard(this->imgDbgGeneral, this->imgDbgGeneral[0].type(), 2);
        displayImage(config, "Segmentation General", generalDash);

        Mat cleanImgDash = drawImageDashboard(this->imgDbgCleanStages, this->imgDbgCleanStages[0].type(), 3);
        displayImage(config, "Segmentation Clean Filters", cleanImgDash);
      }
    }

    cleanCharRegions(pipeline_data->thresholds, pipeline_data->charRegions);

    if (config->debugTiming)
    {
      timespec endTime;
      getTime(&endTime);
      cout << "Character Segmenter Time: " << diffclock(startTime, endTime) << "ms." << endl;
    }
  }

  CharacterSegmenter::~CharacterSegmenter()
  {

  }

  // Given a histogram and the horizontal line boundaries, respond with an array of boxes where the characters are
  // Scores the histogram quality as well based on num chars, char volume, and even separation
  vector<Rect> CharacterSegmenter::getHistogramBoxes(VerticalHistogram histogram, float avgCharWidth, float avgCharHeight, float* score)
  {
    float MIN_HISTOGRAM_HEIGHT = avgCharHeight * config->segmentationMinCharHeightPercent;

    float MAX_SEGMENT_WIDTH = avgCharWidth * config->segmentationMaxCharWidthvsAverage;

    //float MIN_BOX_AREA = (avgCharWidth * avgCharHeight) * 0.25;

    int pxLeniency = 2;

    vector<Rect> charBoxes;
    vector<Rect> allBoxes = get1DHits(histogram.histoImg, pxLeniency);

    for (unsigned int i = 0; i < allBoxes.size(); i++)
    {
      if (allBoxes[i].width >= config->segmentationMinBoxWidthPx && allBoxes[i].width <= MAX_SEGMENT_WIDTH &&
          allBoxes[i].height > MIN_HISTOGRAM_HEIGHT )
      {
        charBoxes.push_back(allBoxes[i]);
      }
      else if (allBoxes[i].width > avgCharWidth * 2 && allBoxes[i].width < MAX_SEGMENT_WIDTH * 2 && allBoxes[i].height > MIN_HISTOGRAM_HEIGHT)
      {
        //    rectangle(histogram.histoImg, allBoxes[i], Scalar(255, 0, 0) );
        //    drawAndWait(&histogram.histoImg);
        // Try to split up doubles into two good char regions, check for a break between 40% and 60%
        int leftEdge = allBoxes[i].x + (int) (((float) allBoxes[i].width) * 0.4f);
        int rightEdge = allBoxes[i].x + (int) (((float) allBoxes[i].width) * 0.6f);

        int minX = histogram.getLocalMinimum(leftEdge, rightEdge);
        int maxXChar1 = histogram.getLocalMaximum(allBoxes[i].x, minX);
        int maxXChar2 = histogram.getLocalMaximum(minX, allBoxes[i].x + allBoxes[i].width);
        int minHeight = histogram.getHeightAt(minX);

        int maxHeightChar1 = histogram.getHeightAt(maxXChar1);
        int maxHeightChar2 = histogram.getHeightAt(maxXChar2);

        if (maxHeightChar1 > MIN_HISTOGRAM_HEIGHT && minHeight < (0.25 * ((float) maxHeightChar1)))
        {
          // Add a box for Char1
          Point botRight = Point(minX - 1, allBoxes[i].y + allBoxes[i].height);
          charBoxes.push_back(Rect(allBoxes[i].tl(), botRight) );
        }
        if (maxHeightChar2 > MIN_HISTOGRAM_HEIGHT && minHeight < (0.25 * ((float) maxHeightChar2)))
        {
          // Add a box for Char2
          Point topLeft = Point(minX + 1, allBoxes[i].y);
          charBoxes.push_back(Rect(topLeft, allBoxes[i].br()) );
        }
      }
    }

    return charBoxes;
  }

  vector<Rect> CharacterSegmenter::getBestCharBoxes(Mat img, vector<Rect> charBoxes, float avgCharWidth)
  {
    float MAX_SEGMENT_WIDTH = avgCharWidth * 1.65;

    // This histogram is based on how many char boxes (from ALL of the many thresholded images) are covering each column
    // Makes a sort of histogram from all the previous char boxes.  Figures out the best fit from that.

    Mat histoImg = Mat::zeros(Size(img.cols, img.rows), CV_8U);

    int columnCount;

    for (int col = 0; col < img.cols; col++)
    {
      columnCount = 0;

      for (unsigned int i = 0; i < charBoxes.size(); i++)
      {
        if (col >= charBoxes[i].x && col < (charBoxes[i].x + charBoxes[i].width))
          columnCount++;
      }

      // Fill the line of the histogram
      for (; columnCount > 0; columnCount--)
        histoImg.at<uchar>(histoImg.rows -  columnCount, col) = 255;
    }

    VerticalHistogram histogram(histoImg, Mat::ones(histoImg.size(), CV_8U));

    // Go through each row in the histoImg and score it.  Try to find the single line that gives me the most right-sized character regions (based on avgCharWidth)

    int bestRowIndex = 0;
    float bestRowScore = 0;
    vector<Rect> bestBoxes;

    for (int row = 0; row < histoImg.rows; row++)
    {
      vector<Rect> validBoxes;
      vector<Rect> allBoxes = get1DHits(histoImg, row);

      if (this->config->debugCharSegmenter)
        cout << "All Boxes size " << allBoxes.size() << endl;

      if (allBoxes.size() == 0)
        break;

      float rowScore = 0;

      for (unsigned int boxidx = 0; boxidx < allBoxes.size(); boxidx++)
      {
        int w = allBoxes[boxidx].width;
        if (w >= config->segmentationMinBoxWidthPx && w <= MAX_SEGMENT_WIDTH)
        {
          float widthDiffPixels = abs(w - avgCharWidth);
          float widthDiffPercent = widthDiffPixels / avgCharWidth;
          rowScore += 10 * (1 - widthDiffPercent);

          if (widthDiffPercent < 0.25)	// Bonus points when it's close to the average character width
            rowScore += 8;

          validBoxes.push_back(allBoxes[boxidx]);
        }
        else if (w > avgCharWidth * 2  && w <= MAX_SEGMENT_WIDTH * 2 )
        {
          // Try to split up doubles into two good char regions, check for a break between 40% and 60%
          int leftEdge = allBoxes[boxidx].x + (int) (((float) allBoxes[boxidx].width) * 0.4f);
          int rightEdge = allBoxes[boxidx].x + (int) (((float) allBoxes[boxidx].width) * 0.6f);

          int minX = histogram.getLocalMinimum(leftEdge, rightEdge);
          int maxXChar1 = histogram.getLocalMaximum(allBoxes[boxidx].x, minX);
          int maxXChar2 = histogram.getLocalMaximum(minX, allBoxes[boxidx].x + allBoxes[boxidx].width);
          int minHeight = histogram.getHeightAt(minX);

          int maxHeightChar1 = histogram.getHeightAt(maxXChar1);
          int maxHeightChar2 = histogram.getHeightAt(maxXChar2);

          if (  minHeight < (0.25 * ((float) maxHeightChar1)))
          {
            // Add a box for Char1
            Point botRight = Point(minX - 1, allBoxes[boxidx].y + allBoxes[boxidx].height);
            validBoxes.push_back(Rect(allBoxes[boxidx].tl(), botRight) );
          }
          if (  minHeight < (0.25 * ((float) maxHeightChar2)))
          {
            // Add a box for Char2
            Point topLeft = Point(minX + 1, allBoxes[boxidx].y);
            validBoxes.push_back(Rect(topLeft, allBoxes[boxidx].br()) );
          }
        }
      }

      if (rowScore > bestRowScore)
      {
        bestRowScore = rowScore;
        bestRowIndex = row;
        bestBoxes = validBoxes;
      }
    }

    if (this->config->debugCharSegmenter)
    {
      cvtColor(histoImg, histoImg, CV_GRAY2BGR);
      line(histoImg, Point(0, histoImg.rows - 1 - bestRowIndex), Point(histoImg.cols, histoImg.rows - 1 - bestRowIndex), Scalar(0, 255, 0));

      Mat imgBestBoxes(img.size(), img.type());
      img.copyTo(imgBestBoxes);
      cvtColor(imgBestBoxes, imgBestBoxes, CV_GRAY2BGR);
      for (unsigned int i = 0; i < bestBoxes.size(); i++)
        rectangle(imgBestBoxes, bestBoxes[i], Scalar(0, 255, 0));

      this->imgDbgGeneral.push_back(addLabel(histoImg, "All Histograms"));
      this->imgDbgGeneral.push_back(addLabel(imgBestBoxes, "Best Boxes"));
    }

    return bestBoxes;
  }

  vector<Rect> CharacterSegmenter::get1DHits(Mat img, int yOffset)
  {
    vector<Rect> hits;

    bool onSegment = false;
    int curSegmentLength = 0;
    for (int col = 0; col < img.cols; col++)
    {
      bool isOn = img.at<uchar>(img.rows - 1 - yOffset, col);
      if (isOn)
      {
        // We're on a segment.  Increment the length
        onSegment = true;
        curSegmentLength++;
      }

      if (onSegment && (isOn == false || (col == img.cols - 1)))
      {
        // A segment just ended or we're at the very end of the row and we're on a segment
        Point topLeft = Point(col - curSegmentLength, top.getPointAt(col - curSegmentLength) - 1);
        Point botRight = Point(col, bottom.getPointAt(col) + 1);
        hits.push_back(Rect(topLeft, botRight));

        onSegment = false;
        curSegmentLength = 0;
      }
    }

    return hits;
  }

  void CharacterSegmenter::removeSmallContours(vector<Mat> thresholds, float avgCharHeight,  TextLine textLine)
  {
    //const float MIN_CHAR_AREA = 0.02 * avgCharWidth * avgCharHeight;	// To clear out the tiny specks
    const float MIN_CONTOUR_HEIGHT = 0.3 * avgCharHeight;

    Mat textLineMask = Mat::zeros(thresholds[0].size(), CV_8U);
    fillConvexPoly(textLineMask, textLine.linePolygon.data(), textLine.linePolygon.size(), Scalar(255,255,255));

    for (unsigned int i = 0; i < thresholds.size(); i++)
    {
      vector<vector<Point> > contours;
      vector<Vec4i> hierarchy;
      Mat thresholdsCopy = Mat::zeros(thresholds[i].size(), thresholds[i].type());

      thresholds[i].copyTo(thresholdsCopy, textLineMask);
      findContours(thresholdsCopy, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

      for (unsigned int c = 0; c < contours.size(); c++)
      {
        if (contours[c].size() == 0)
          continue;

        Rect mr = boundingRect(contours[c]);
        if (mr.height < MIN_CONTOUR_HEIGHT)
        {
          // Erase it
          drawContours(thresholds[i], contours, c, Scalar(0, 0, 0), -1);
          continue;
        }
      }
    }
  }

  vector<Rect> CharacterSegmenter::combineCloseBoxes( vector<Rect> charBoxes, float biggestCharWidth)
  {
    vector<Rect> newCharBoxes;

    for (unsigned int i = 0; i < charBoxes.size(); i++)
    {
      if (i == charBoxes.size() - 1)
      {
        newCharBoxes.push_back(charBoxes[i]);
        break;
      }
      float bigWidth = (charBoxes[i + 1].x + charBoxes[i + 1].width - charBoxes[i].x);

      float w1Diff = abs(charBoxes[i].width - biggestCharWidth);
      float w2Diff = abs(charBoxes[i + 1].width - biggestCharWidth);
      float bigDiff = abs(bigWidth - biggestCharWidth);
      bigDiff *= 1.3;	// Make it a little harder to merge boxes.

      if (bigDiff < w1Diff && bigDiff < w2Diff)
      {
        Rect bigRect(charBoxes[i].x, charBoxes[i].y, bigWidth, charBoxes[i].height);
        newCharBoxes.push_back(bigRect);
        if (this->config->debugCharSegmenter)
        {
          for (unsigned int z = 0; z < pipeline_data->thresholds.size(); z++)
          {
            Point center(bigRect.x + bigRect.width / 2, bigRect.y + bigRect.height / 2);
            RotatedRect rrect(center, Size2f(bigRect.width, bigRect.height + (bigRect.height / 2)), 0);
            ellipse(imgDbgCleanStages[z], rrect, Scalar(0,255,0), 1);
          }
          cout << "Merging 2 boxes -- " << i << " and " << i + 1 << endl;
        }

        i++;
      }
      else
      {
        newCharBoxes.push_back(charBoxes[i]);
      }
    }

    return newCharBoxes;
  }

  void CharacterSegmenter::cleanCharRegions(vector<Mat> thresholds, vector<Rect> charRegions)
  {
    const float MIN_SPECKLE_HEIGHT_PERCENT = 0.13;
    const float MIN_SPECKLE_WIDTH_PX = 3;
    const float MIN_CONTOUR_AREA_PERCENT = 0.1;
    const float MIN_CONTOUR_HEIGHT_PERCENT = config->segmentationMinCharHeightPercent;

    Mat mask = getCharBoxMask(thresholds[0], charRegions);

    for (unsigned int i = 0; i < thresholds.size(); i++)
    {
      bitwise_and(thresholds[i], mask, thresholds[i]);
      vector<vector<Point> > contours;

      Mat tempImg(thresholds[i].size(), thresholds[i].type());
      thresholds[i].copyTo(tempImg);

      //Mat element = getStructuringElement( 1,
  //				    Size( 2 + 1, 2+1 ),
  //				    Point( 1, 1 ) );
      //dilate(thresholds[i], tempImg, element);
      //morphologyEx(thresholds[i], tempImg, MORPH_CLOSE, element);
      //drawAndWait(&tempImg);

      findContours(tempImg, contours, RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

      for (unsigned int j = 0; j < charRegions.size(); j++)
      {
        const float MIN_SPECKLE_HEIGHT = ((float)charRegions[j].height) * MIN_SPECKLE_HEIGHT_PERCENT;
        const float MIN_CONTOUR_AREA = ((float)charRegions[j].area()) * MIN_CONTOUR_AREA_PERCENT;

        int tallestContourHeight = 0;
        float totalArea = 0;
        for (unsigned int c = 0; c < contours.size(); c++)
        {
          if (contours[c].size() == 0)
            continue;
          if (charRegions[j].contains(contours[c][0]) == false)
            continue;

          Rect r = boundingRect(contours[c]);

          if (r.height <= MIN_SPECKLE_HEIGHT || r.width <= MIN_SPECKLE_WIDTH_PX)
          {
            // Erase this speckle
            drawContours(thresholds[i], contours, c, Scalar(0,0,0), CV_FILLED);

            if (this->config->debugCharSegmenter)
            {
              drawContours(imgDbgCleanStages[i], contours, c, COLOR_DEBUG_SPECKLES, CV_FILLED);
            }
          }
          else
          {
            if (r.height > tallestContourHeight)
              tallestContourHeight = r.height;

            totalArea += contourArea(contours[c]);
          }
          //else if (r.height > tallestContourHeight)
          //{
          //  tallestContourIndex = c;
          //  tallestContourHeight = h;
          //}
        }

        if (totalArea < MIN_CONTOUR_AREA)
        {
          // Character is not voluminous enough.  Erase it.
          if (this->config->debugCharSegmenter)
          {
            cout << "Character CLEAN: (area) removing box " << j << " in threshold " << i << " -- Area " << totalArea << " < " << MIN_CONTOUR_AREA << endl;

            Rect boxTop(charRegions[j].x, charRegions[j].y - 10, charRegions[j].width, 10);
            rectangle(imgDbgCleanStages[i], boxTop, COLOR_DEBUG_MIN_AREA, -1);
          }

          rectangle(thresholds[i], charRegions[j], Scalar(0, 0, 0), -1);
        }
        else if (tallestContourHeight < ((float) charRegions[j].height * MIN_CONTOUR_HEIGHT_PERCENT))
        {
          // This character is too short.  Black the whole thing out
          if (this->config->debugCharSegmenter)
          {
            cout << "Character CLEAN: (height) removing box " << j << " in threshold " << i << " -- Height " << tallestContourHeight << " < " << ((float) charRegions[j].height * MIN_CONTOUR_HEIGHT_PERCENT) << endl;

            Rect boxBottom(charRegions[j].x, charRegions[j].y + charRegions[j].height, charRegions[j].width, 10);
            rectangle(imgDbgCleanStages[i], boxBottom, COLOR_DEBUG_MIN_HEIGHT, -1);
          }
          rectangle(thresholds[i], charRegions[j], Scalar(0, 0, 0), -1);
        }
      }

      Mat closureElement = getStructuringElement( 1,
                           Size( 2 + 1, 2+1 ),
                           Point( 1, 1 ) );

      //morphologyEx(thresholds[i], thresholds[i], MORPH_OPEN, element);

      //dilate(thresholds[i], thresholds[i], element);
      //erode(thresholds[i], thresholds[i], element);

      morphologyEx(thresholds[i], thresholds[i], MORPH_CLOSE, closureElement);

      // Lastly, draw a clipping line between each character boxes
      for (unsigned int j = 0; j < charRegions.size(); j++)
      {
        line(thresholds[i], Point(charRegions[j].x - 1, charRegions[j].y), Point(charRegions[j].x - 1, charRegions[j].y + charRegions[j].height), Scalar(0, 0, 0));
        line(thresholds[i], Point(charRegions[j].x + charRegions[j].width + 1, charRegions[j].y), Point(charRegions[j].x + charRegions[j].width + 1, charRegions[j].y + charRegions[j].height), Scalar(0, 0, 0));
      }
    }
  }

  void CharacterSegmenter::cleanBasedOnColor(vector<Mat> thresholds, Mat colorMask, vector<Rect> charRegions)
  {
    // If I knock out x% of the contour area from this thing (after applying the color filter)
    // Consider it a bad news bear.  REmove the whole area.
    const float MIN_PERCENT_CHUNK_REMOVED = 0.6;

    for (unsigned int i = 0; i < thresholds.size(); i++)
    {
      for (unsigned int j = 0; j < charRegions.size(); j++)
      {
        Mat boxChar = Mat::zeros(thresholds[i].size(), CV_8U);
        rectangle(boxChar, charRegions[j], Scalar(255,255,255), CV_FILLED);

        bitwise_and(thresholds[i], boxChar, boxChar);

        float meanBefore = mean(boxChar, boxChar)[0];

        Mat thresholdCopy(thresholds[i].size(), CV_8U);
        bitwise_and(colorMask, boxChar, thresholdCopy);

        float meanAfter = mean(thresholdCopy, boxChar)[0];

        if (meanAfter < meanBefore * (1-MIN_PERCENT_CHUNK_REMOVED))
        {
          rectangle(thresholds[i], charRegions[j], Scalar(0,0,0), CV_FILLED);

          if (this->config->debugCharSegmenter)
          {
            //vector<Mat> tmpDebug;
            //Mat thresholdCopy2 = Mat::zeros(thresholds[i].size(), CV_8U);
            //rectangle(thresholdCopy2, charRegions[j], Scalar(255,255,255), CV_FILLED);
            //tmpDebug.push_back(addLabel(thresholdCopy2, "box Mask"));
            //bitwise_and(thresholds[i], thresholdCopy2, thresholdCopy2);
            //tmpDebug.push_back(addLabel(thresholdCopy2, "box Mask + Thresh"));
            //bitwise_and(colorMask, thresholdCopy2, thresholdCopy2);
            //tmpDebug.push_back(addLabel(thresholdCopy2, "box Mask + Thresh + Color"));
  //
  //		Mat tmpytmp = addLabel(thresholdCopy2, "box Mask + Thresh + Color");
  //		Mat tmpx = drawImageDashboard(tmpDebug, tmpytmp.type(), 1);
            //drawAndWait( &tmpx );

            cout << "Segmentation Filter Clean by Color: Removed Threshold " << i << " charregion " << j << endl;
            cout << "Segmentation Filter Clean by Color: before=" << meanBefore << " after=" << meanAfter  << endl;

            Point topLeft(charRegions[j].x, charRegions[j].y);
            circle(imgDbgCleanStages[i], topLeft, 5, COLOR_DEBUG_COLORFILTER, CV_FILLED);
          }
        }
      }
    }
  }

  void CharacterSegmenter::cleanMostlyFullBoxes(vector<Mat> thresholds, const vector<Rect> charRegions)
  {
    float MAX_FILLED = 0.95 * 255;

    for (unsigned int i = 0; i < charRegions.size(); i++)
    {
      Mat mask = Mat::zeros(thresholds[0].size(), CV_8U);
      rectangle(mask, charRegions[i], Scalar(255,255,255), -1);

      for (unsigned int j = 0; j < thresholds.size(); j++)
      {
        if (mean(thresholds[j], mask)[0] > MAX_FILLED)
        {
          // Empty the rect
          rectangle(thresholds[j], charRegions[i], Scalar(0,0,0), -1);

          if (this->config->debugCharSegmenter)
          {
            Rect inset(charRegions[i].x + 2, charRegions[i].y - 2, charRegions[i].width - 4, charRegions[i].height - 4);
            rectangle(imgDbgCleanStages[j], inset, COLOR_DEBUG_FULLBOX, 1);
          }
        }
      }
    }
  }

  vector<Rect> CharacterSegmenter::filterMostlyEmptyBoxes(vector<Mat> thresholds, const vector<Rect> charRegions)
  {
    // Of the n thresholded images, if box 3 (for example) is empty in half (for example) of the thresholded images,
    // clear all data for every box #3.

    //const float MIN_AREA_PERCENT = 0.1;
    const float MIN_CONTOUR_HEIGHT_PERCENT = config->segmentationMinCharHeightPercent;

    Mat mask = getCharBoxMask(thresholds[0], charRegions);

    vector<int> boxScores(charRegions.size());

    for (unsigned int i = 0; i < charRegions.size(); i++)
      boxScores[i] = 0;

    for (unsigned int i = 0; i < thresholds.size(); i++)
    {
      for (unsigned int j = 0; j < charRegions.size(); j++)
      {
        //float minArea = charRegions[j].area() * MIN_AREA_PERCENT;

        Mat tempImg = Mat::zeros(thresholds[i].size(), thresholds[i].type());
        rectangle(tempImg, charRegions[j], Scalar(255,255,255), CV_FILLED);
        bitwise_and(thresholds[i], tempImg, tempImg);

        vector<vector<Point> > contours;
        findContours(tempImg, contours, RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

        vector<Point> allPointsInBox;
        for (unsigned int c = 0; c < contours.size(); c++)
        {
          if (contours[c].size() == 0)
            continue;

          for (unsigned int z = 0; z < contours[c].size(); z++)
            allPointsInBox.push_back(contours[c][z]);
        }

        float height = 0;
        if (allPointsInBox.size() > 0)
        {
          height = boundingRect(allPointsInBox).height;
        }

        if (height >= ((float) charRegions[j].height * MIN_CONTOUR_HEIGHT_PERCENT))
        {
          boxScores[j] = boxScores[j] + 1;
        }
        else if (this->config->debugCharSegmenter)
        {
          drawX(imgDbgCleanStages[i], charRegions[j], COLOR_DEBUG_EMPTYFILTER, 3);
        }
      }
    }

    vector<Rect> newCharRegions;

    int maxBoxScore = 0;
    for (unsigned int i = 0; i < charRegions.size(); i++)
    {
      if (boxScores[i] > maxBoxScore)
        maxBoxScore = boxScores[i];
    }

    // Need a good char sample in at least 50% of the boxes for it to be valid.
    int MIN_FULL_BOXES = maxBoxScore * 0.49;

    // Now check each score.  If it's below the minimum, remove the charRegion
    for (unsigned int i = 0; i < charRegions.size(); i++)
    {
      if (boxScores[i] > MIN_FULL_BOXES)
        newCharRegions.push_back(charRegions[i]);
      else
      {
        // Erase the box from the Mat... mainly for debug purposes
        if (this->config->debugCharSegmenter)
        {
          cout << "Mostly Empty Filter: box index: " << i;
          cout << " this box had a score of : " << boxScores[i];;
          cout << " MIN_FULL_BOXES: " << MIN_FULL_BOXES << endl;;

          for (unsigned int z = 0; z < thresholds.size(); z++)
          {
            rectangle(thresholds[z], charRegions[i], Scalar(0,0,0), -1);

            drawX(imgDbgCleanStages[z], charRegions[i], COLOR_DEBUG_EMPTYFILTER, 1);
          }
        }
      }

      if (this->config->debugCharSegmenter)
        cout << " Box Score: " << boxScores[i] << endl;
    }

    return newCharRegions;
  }

  void CharacterSegmenter::filterEdgeBoxes(vector<Mat> thresholds, const vector<Rect> charRegions, float avgCharWidth, float avgCharHeight)
  {
    const float MIN_ANGLE_FOR_ROTATION = 0.4;
    int MIN_CONNECTED_EDGE_PIXELS = (avgCharHeight * 1.5);

    // Sometimes the rectangle won't be very tall, making it impossible to detect an edge
    // Adjust for this here.
    int alternate = thresholds[0].rows * 0.92;
    if (alternate < MIN_CONNECTED_EDGE_PIXELS && alternate > avgCharHeight)
      MIN_CONNECTED_EDGE_PIXELS = alternate;

    //
    // Pay special attention to the edge boxes.  If it's a skinny box, and the vertical height extends above our bounds... remove it.
    //while (charBoxes.size() > 0 && charBoxes[charBoxes.size() - 1].width < MIN_SEGMENT_WIDTH_EDGES)
    //  charBoxes.erase(charBoxes.begin() + charBoxes.size() - 1);
    // Now filter the "edge" boxes.  We don't want to include skinny boxes on the edges, since these could be plate boundaries
    //while (charBoxes.size() > 0 && charBoxes[0].width < MIN_SEGMENT_WIDTH_EDGES)
    //  charBoxes.erase(charBoxes.begin() + 0);

    // TECHNIQUE #1
    // Check for long vertical lines.  Once the line is too long, mask the whole region

    if (charRegions.size() <= 1)
      return;

    // Check both sides to see where the edges are
    // The first starts at the right edge of the leftmost char region and works its way left
    // The second starts at the left edge of the rightmost char region and works its way right.
    // We start by rotating the threshold image to the correct angle
    // then check each column 1 by 1.

    vector<int> leftEdges;
    vector<int> rightEdges;

    for (unsigned int i = 0; i < thresholds.size(); i++)
    {
      Mat rotated;

      if (abs(top.angle) > MIN_ANGLE_FOR_ROTATION)
      {
        // Rotate image:
        rotated = Mat(thresholds[i].size(), thresholds[i].type());
        Mat rot_mat( 2, 3, CV_32FC1 );
        Point center = Point( thresholds[i].cols/2, thresholds[i].rows/2 );

        rot_mat = getRotationMatrix2D( center, top.angle, 1.0 );
        warpAffine( thresholds[i], rotated, rot_mat, thresholds[i].size() );
      }
      else
      {
        rotated = thresholds[i];
      }

      int leftEdgeX = 0;
      int rightEdgeX = rotated.cols;
      // Do the left side
      int col = charRegions[0].x + charRegions[0].width;
      while (col >= 0)
      {
        int rowLength = getLongestBlobLengthBetweenLines(rotated, col);

        if (rowLength > MIN_CONNECTED_EDGE_PIXELS)
        {
          leftEdgeX = col;
          break;
        }

        col--;
      }

      col = charRegions[charRegions.size() - 1].x;
      while (col < rotated.cols)
      {
        int rowLength = getLongestBlobLengthBetweenLines(rotated, col);

        if (rowLength > MIN_CONNECTED_EDGE_PIXELS)
        {
          rightEdgeX = col;
          break;
        }
        col++;
      }

      if (leftEdgeX != 0)
        leftEdges.push_back(leftEdgeX);
      if (rightEdgeX != thresholds[i].cols)
        rightEdges.push_back(rightEdgeX);
    }

    int leftEdge = 0;
    int rightEdge = thresholds[0].cols;

    // Assign the edge values to the SECOND closest value
    if (leftEdges.size() > 1)
    {
      sort (leftEdges.begin(), leftEdges.begin()+leftEdges.size());
      leftEdge = leftEdges[leftEdges.size() - 2] + 1;
    }
    if (rightEdges.size() > 1)
    {
      sort (rightEdges.begin(), rightEdges.begin()+rightEdges.size());
      rightEdge = rightEdges[1] - 1;
    }

    if (leftEdge != 0 || rightEdge != thresholds[0].cols)
    {
      Mat mask = Mat::zeros(thresholds[0].size(), CV_8U);
      rectangle(mask, Point(leftEdge, 0), Point(rightEdge, thresholds[0].rows), Scalar(255,255,255), -1);

      if (abs(top.angle) > MIN_ANGLE_FOR_ROTATION)
      {
        // Rotate mask:
        Mat rot_mat( 2, 3, CV_32FC1 );
        Point center = Point( mask.cols/2, mask.rows/2 );

        rot_mat = getRotationMatrix2D( center, top.angle * -1, 1.0 );
        warpAffine( mask, mask, rot_mat, mask.size() );
      }

      // If our edge mask covers more than x% of the char region, mask the whole thing...
      const float MAX_COVERAGE_PERCENT = 0.6;
      int leftCoveragePx = leftEdge - charRegions[0].x;
      float leftCoveragePercent = ((float) leftCoveragePx) / ((float) charRegions[0].width);
      float rightCoveragePx = (charRegions[charRegions.size() -1].x + charRegions[charRegions.size() -1].width) - rightEdge;
      float rightCoveragePercent = ((float) rightCoveragePx) / ((float) charRegions[charRegions.size() -1].width);
      if ((leftCoveragePercent > MAX_COVERAGE_PERCENT) ||
          (charRegions[0].width - leftCoveragePx < config->segmentationMinBoxWidthPx))
      {
        rectangle(mask, charRegions[0], Scalar(0,0,0), -1);	// Mask the whole region
        if (this->config->debugCharSegmenter)
          cout << "Edge Filter: Entire left region is erased" << endl;
      }
      if ((rightCoveragePercent > MAX_COVERAGE_PERCENT) ||
          (charRegions[charRegions.size() -1].width - rightCoveragePx < config->segmentationMinBoxWidthPx))
      {
        rectangle(mask, charRegions[charRegions.size() -1], Scalar(0,0,0), -1);
        if (this->config->debugCharSegmenter)
          cout << "Edge Filter: Entire right region is erased" << endl;
      }

      for (unsigned int i = 0; i < thresholds.size(); i++)
      {
        bitwise_and(thresholds[i], mask, thresholds[i]);
      }

      if (this->config->debugCharSegmenter)
      {
        cout << "Edge Filter: left=" << leftEdge << " right=" << rightEdge << endl;
        Mat bordered = addLabel(mask, "Edge Filter #1");
        imgDbgGeneral.push_back(bordered);

        Mat invertedMask(mask.size(), mask.type());
        bitwise_not(mask, invertedMask);
        for (unsigned int z = 0; z < imgDbgCleanStages.size(); z++)
          fillMask(imgDbgCleanStages[z], invertedMask, Scalar(0,0,255));
      }
    }

  }

  int CharacterSegmenter::getLongestBlobLengthBetweenLines(Mat img, int col)
  {
    int longestBlobLength = 0;

    bool onSegment = false;
    bool wasbetweenLines = false;
    float curSegmentLength = 0;
    for (int row = 0; row < img.rows; row++)
    {
      bool isbetweenLines = false;

      bool isOn = img.at<uchar>(row, col);
      // check two rows at a time.
      if (!isOn && col < img.cols)
        isOn = img.at<uchar>(row, col);

      if (isOn)
      {
        // We're on a segment.  Increment the length
        isbetweenLines = top.isPointBelowLine(Point(col, row)) && !bottom.isPointBelowLine(Point(col, row));
        float incrementBy = 1;

        // Add a little extra to the score if this is outside of the lines
        if (!isbetweenLines)
          incrementBy = 1.1;

        onSegment = true;
        curSegmentLength += incrementBy;
      }
      if (isOn && isbetweenLines)
      {
        wasbetweenLines = true;
      }

      if (onSegment && (isOn == false || (row == img.rows - 1)))
      {
        if (wasbetweenLines && curSegmentLength > longestBlobLength)
          longestBlobLength = curSegmentLength;

        onSegment = false;
        isbetweenLines = false;
        curSegmentLength = 0;
      }
    }

    return longestBlobLength;
  }

  // Checks to see if a skinny, tall line (extending above or below the char Height) is inside the given box.
  // Returns the contour index if true.  -1 otherwise
  int CharacterSegmenter::isSkinnyLineInsideBox(Mat threshold, Rect box, vector<vector<Point> > contours, vector<Vec4i> hierarchy, float avgCharWidth, float avgCharHeight)
  {
    float MIN_EDGE_CONTOUR_HEIGHT = avgCharHeight * 1.25;

    // Sometimes the threshold is smaller than the MIN_EDGE_CONTOUR_HEIGHT.
    // In that case, adjust to be smaller
    int alternate = threshold.rows * 0.92;
    if (alternate < MIN_EDGE_CONTOUR_HEIGHT && alternate > avgCharHeight)
      MIN_EDGE_CONTOUR_HEIGHT = alternate;

    Rect slightlySmallerBox(box.x, box.y, box.width, box.height);
    Mat boxMask = Mat::zeros(threshold.size(), CV_8U);
    rectangle(boxMask, slightlySmallerBox, Scalar(255, 255, 255), -1);

    for (unsigned int i = 0; i < contours.size(); i++)
    {
      // Only bother with the big boxes
      if (boundingRect(contours[i]).height < MIN_EDGE_CONTOUR_HEIGHT)
        continue;

      Mat tempImg = Mat::zeros(threshold.size(), CV_8U);
      drawContours(tempImg, contours, i, Scalar(255,255,255), -1, 8, hierarchy, 1);
      bitwise_and(tempImg, boxMask, tempImg);

      vector<vector<Point> > subContours;
      findContours(tempImg, subContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
      int tallestContourIdx = -1;
      int tallestContourHeight = 0;
      int tallestContourWidth = 0;
      float tallestContourArea = 0;
      for (unsigned int s = 0; s < subContours.size(); s++)
      {
        Rect r = boundingRect(subContours[s]);
        if (r.height > tallestContourHeight)
        {
          tallestContourIdx = s;
          tallestContourHeight = r.height;
          tallestContourWidth = r.width;
          tallestContourArea = contourArea(subContours[s]);
        }
      }

      if (tallestContourIdx != -1)
      {
        //cout << "Edge Filter: " << tallestContourHeight << " -- " << avgCharHeight << endl;
        if (tallestContourHeight >= avgCharHeight * 0.9 &&
            ((tallestContourWidth < config->segmentationMinBoxWidthPx) || (tallestContourArea < avgCharWidth * avgCharHeight * 0.1)))
        {
          cout << "Edge Filter: Avg contour width: " << avgCharWidth << " This guy is: " << tallestContourWidth << endl;
          cout << "Edge Filter: tallestContourArea: " << tallestContourArea << " Minimum: " << avgCharWidth * avgCharHeight * 0.1 << endl;
          return i;
        }
      }
    }

    return -1;
  }

  Mat CharacterSegmenter::getCharBoxMask(Mat img_threshold, vector<Rect> charBoxes)
  {
    Mat mask = Mat::zeros(img_threshold.size(), CV_8U);
    for (unsigned int i = 0; i < charBoxes.size(); i++)
      rectangle(mask, charBoxes[i], Scalar(255, 255, 255), -1);

    return mask;
  }

}
