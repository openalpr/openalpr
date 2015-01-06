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

#include "verticalhistogram.h"

using namespace cv;
using namespace std;

namespace alpr
{

  VerticalHistogram::VerticalHistogram(Mat inputImage, Mat mask)
  {
    analyzeImage(inputImage, mask);
  }

  VerticalHistogram::~VerticalHistogram()
  {
    histoImg.release();
    colHeights.clear();
  }

  void VerticalHistogram::analyzeImage(Mat inputImage, Mat mask)
  {
    highestPeak = 0;
    lowestValley = inputImage.rows;

    histoImg = Mat::zeros(inputImage.size(), CV_8U);

    int columnCount;

    for (int col = 0; col < inputImage.cols; col++)
    {
      columnCount = 0;

      for (int row = 0; row < inputImage.rows; row++)
      {
        if (inputImage.at<uchar>(row, col) > 0 && mask.at<uchar>(row, col) > 0)
          columnCount++;
      }

      this->colHeights.push_back(columnCount);

      if (columnCount < lowestValley)
        lowestValley = columnCount;
      if (columnCount > highestPeak)
        highestPeak = columnCount;

      for (; columnCount > 0; columnCount--)
        histoImg.at<uchar>(inputImage.rows - columnCount, col) = 255;
    }
  }

  int VerticalHistogram::getLocalMinimum(int leftX, int rightX)
  {
    int minimum = histoImg.rows + 1;
    int lowestX = leftX;

    for (int i = leftX; i <= rightX; i++)
    {
      if (colHeights[i] < minimum)
      {
        lowestX = i;
        minimum = colHeights[i];
      }
    }

    return lowestX;
  }

  int VerticalHistogram::getLocalMaximum(int leftX, int rightX)
  {
    int maximum = -1;
    int highestX = leftX;

    for (int i = leftX; i <= rightX; i++)
    {
      if (colHeights[i] > maximum)
      {
        highestX = i;
        maximum = colHeights[i];
      }
    }

    return highestX;
  }

  int VerticalHistogram::getHeightAt(int x)
  {
    return colHeights[x];
  }

  void VerticalHistogram::findValleys()
  {
    //int MINIMUM_PEAK_HEIGHT = (int) (((float) highestPeak) * 0.75);

    int totalWidth = colHeights.size();

    int midpoint = ((highestPeak - lowestValley) / 2) + lowestValley;

    HistogramDirection prevDirection = FALLING;

    int relativePeakHeight = 0;
    //int valleyStart = 0;

    for (int i = 0; i < totalWidth; i++)
    {
      bool aboveMidpoint = (colHeights[i] >= midpoint);

      if (aboveMidpoint)
      {
        if (colHeights[i] > relativePeakHeight)
          relativePeakHeight = colHeights[i];

        prevDirection = FLAT;
      }
      else
      {
        relativePeakHeight = 0;

        HistogramDirection direction = getHistogramDirection(i);

        if ((prevDirection == FALLING || prevDirection == FLAT) && direction == RISING)
        {
        }
        else if ((prevDirection == FALLING || prevDirection == FLAT) && direction == RISING)
        {
        }
      }
    }
  }

  HistogramDirection VerticalHistogram::getHistogramDirection(unsigned int index)
  {
    int EXTRA_WIDTH_TO_AVERAGE = 2;

    float trailingAverage = 0;
    float forwardAverage = 0;

    int trailStartIndex = index - EXTRA_WIDTH_TO_AVERAGE;
    if (trailStartIndex < 0)
      trailStartIndex = 0;
    unsigned int forwardEndIndex = index + EXTRA_WIDTH_TO_AVERAGE;
    if (forwardEndIndex >= colHeights.size())
      forwardEndIndex = colHeights.size() - 1;

    for (int i = index; i >= trailStartIndex; i--)
    {
      trailingAverage += colHeights[i];
    }
    trailingAverage = trailingAverage / ((float) (1 + index - trailStartIndex));

    for (unsigned int i = index; i <= forwardEndIndex; i++)
    {
      forwardAverage += colHeights[i];
    }
    forwardAverage = forwardAverage / ((float) (1 + forwardEndIndex - index));

    float diff = forwardAverage - trailingAverage;
    float minDiff = ((float) (highestPeak - lowestValley)) * 0.10;

    if (diff > minDiff)
      return RISING;
    else if (diff < minDiff)
      return FALLING;
    else
      return FLAT;
  }

}