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

#include "histogram.h"

using namespace cv;
using namespace std;

namespace alpr
{

  Histogram::Histogram()
  {

  }
  
  Histogram::~Histogram()
  {
    histoImg.release();
    colHeights.clear();
  }

  void Histogram::analyzeImage(cv::Mat inputImage, cv::Mat mask, bool use_y_axis)
  {

    int max_col_size = 0;

    int columnCount;

    if (use_y_axis)
    {
      // Calculate the histogram for vertical stripes
      for (int col = 0; col < inputImage.cols; col++)
      {
        columnCount = 0;

        for (int row = 0; row < inputImage.rows; row++)
        {
          if (inputImage.at<uchar>(row, col) > 0 && mask.at<uchar>(row, col) > 0)
            columnCount++;
        }

        this->colHeights.push_back(columnCount);

        if (columnCount > max_col_size)
          max_col_size = columnCount;
      }
    }
    else
    {
      // Calculate the histogram for horizontal stripes
      for (int row = 0; row < inputImage.rows; row++)
      {
        columnCount = 0;

        for (int col = 0; col < inputImage.cols; col++)
        {
          if (inputImage.at<uchar>(row, col) > 0 && mask.at<uchar>(row, col) > 0)
            columnCount++;
        }

        this->colHeights.push_back(columnCount);

        if (columnCount > max_col_size)
          max_col_size = columnCount;
      }
    }
    
    
    int histo_width = this->colHeights.size();
    int histo_height = max_col_size + 10;
        
    histoImg = Mat::zeros(Size(histo_width, histo_height), CV_8U);
    
    // Draw the columns onto an Mat image
    for (unsigned int col = 0; col < histoImg.cols; col++)
    {
      if (col >= this->colHeights.size())
        break;
      
      int columnCount = this->colHeights[col];
      for (; columnCount > 0; columnCount--)
        histoImg.at<uchar>(histo_height - columnCount, col) = 255;
    }

    
  }

  int Histogram::getLocalMinimum(int leftX, int rightX)
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

  int Histogram::getLocalMaximum(int leftX, int rightX)
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

  int Histogram::getHeightAt(int x)
  {
    return colHeights[x];
  }
  
  

  int Histogram::detect_peak(
      const double*   data, /* the data */
      int             data_count, /* row count of data */
      int*            emi_peaks, /* emission peaks will be put here */
      int*            num_emi_peaks, /* number of emission peaks found */
      int             max_emi_peaks, /* maximum number of emission peaks */
      int*            absop_peaks, /* absorption peaks will be put here */
      int*            num_absop_peaks, /* number of absorption peaks found */
      int             max_absop_peaks, /* maximum number of absorption peaks
                                              */
      double          delta, /* delta used for distinguishing peaks */
      int             emi_first /* should we search emission peak first of
                                       absorption peak first? */
  )
  {
    int     i;
    double  mx;
    double  mn;
    int     mx_pos = 0;
    int     mn_pos = 0;
    int     is_detecting_emi = emi_first;


    mx = data[0];
    mn = data[0];

    *num_emi_peaks = 0;
    *num_absop_peaks = 0;

    for(i = 1; i < data_count; ++i)
    {
      if(data[i] > mx)
      {
        mx_pos = i;
        mx = data[i];
      }
      if(data[i] < mn)
      {
        mn_pos = i;
        mn = data[i];
      }

      if(is_detecting_emi &&
         data[i] < mx - delta)
      {
        if(*num_emi_peaks >= max_emi_peaks) /* not enough spaces */
          return 1;

        emi_peaks[*num_emi_peaks] = mx_pos;
        ++ (*num_emi_peaks);

        is_detecting_emi = 0;

        i = mx_pos - 1;

        mn = data[mx_pos];
        mn_pos = mx_pos;
      }
      else if((!is_detecting_emi) &&
              data[i] > mn + delta)
      {
        if(*num_absop_peaks >= max_absop_peaks)
          return 2;

        absop_peaks[*num_absop_peaks] = mn_pos;
        ++ (*num_absop_peaks);

        is_detecting_emi = 1;

        i = mn_pos - 1;

        mx = data[mn_pos];
        mx_pos = mn_pos;
      }
    }

    return 0;
  }

  
  vector<pair<int, int> > Histogram::get1DHits(int yOffset)
  {
    
    vector<pair<int,int> > hits;
    
    bool onSegment = false;
    int curSegmentLength = 0;
    for (int col = 0; col < histoImg.cols; col++)
    {
      bool isOn = histoImg.at<uchar>(histoImg.rows - 1 - yOffset, col);
      if (isOn)
      {
        // We're on a segment.  Increment the length
        onSegment = true;
        curSegmentLength++;
      }

      if (onSegment && (isOn == false || (col == histoImg.cols - 1)))
      {
        
        // A segment just ended or we're at the very end of the row and we're on a segment
        pair<int, int> pair(col - curSegmentLength, col);
        hits.push_back(pair);
        
        onSegment = false;
        curSegmentLength = 0;
      }
    }

    
    return hits;
  }
}