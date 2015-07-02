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
    
    vector<double> data;
    
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

      data.push_back(columnCount);
      for (; columnCount > 0; columnCount--)
        histoImg.at<uchar>(inputImage.rows - columnCount, col) = 255;
    }
    
//    int         MAX_PEAK=200;
//    int         emi_peaks[MAX_PEAK];
//    int         absorp_peaks[MAX_PEAK];
//    int         emi_count = 0;
//    int         absorp_count = 0;
//    double      delta = highestPeak * (1.0 / 3.0);
//    int         emission_first = 0;
//    
//    detect_peak(data.data(), data.size(), emi_peaks, &emi_count, MAX_PEAK,
//            absorp_peaks, &absorp_count, MAX_PEAK,
//            delta, emission_first);
//    
//    Mat colorDebugImg;
//    cvtColor(histoImg, colorDebugImg, CV_GRAY2BGR );
//    
//    for (int i = 0; i < emi_count; i++)
//    {
//      cout << "EMI PEAK: " << emi_peaks[i] << " : " << data[emi_peaks[i]] << endl;
//      circle(colorDebugImg, Point(emi_peaks[i], histoImg.rows -  data[emi_peaks[i]]), 2, Scalar(0,255,0), -1);
//    }
//    
//    for (int i = 0; i < absorp_count; i++)
//    {
//      cout << "ABSORB PEAK: " << absorp_peaks[i] << " : " << data[absorp_peaks[i]] << endl;
//      circle(colorDebugImg, Point(absorp_peaks[i], histoImg.rows - data[absorp_peaks[i]]), 2, Scalar(0,0,255), -1);
//    }
//    
//    
//    drawAndWait(&colorDebugImg);
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

  int VerticalHistogram::detect_peak(
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

}