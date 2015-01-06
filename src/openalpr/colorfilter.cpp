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

#include "colorfilter.h"

using namespace cv;
using namespace std;

namespace alpr
{
  

  ColorFilter::ColorFilter(Mat image, Mat characterMask, Config* config)
  {
    timespec startTime;
    getTime(&startTime);

    this->config = config;

    this->debug = config->debugColorFiler;

    this->grayscale = imageIsGrayscale(image);

    if (this->debug)
      cout << "ColorFilter: isGrayscale = " << grayscale << endl;

    this->hsv = Mat(image.size(), image.type());
    cvtColor( image, this->hsv, CV_BGR2HSV );
    preprocessImage();

    this->charMask = characterMask;

    this->colorMask = Mat(image.size(), CV_8U);

    findCharColors();

    if (config->debugTiming)
    {
      timespec endTime;
      getTime(&endTime);
      cout << "  -- ColorFilter Time: " << diffclock(startTime, endTime) << "ms." << endl;
    }
  }

  ColorFilter::~ColorFilter()
  {
  }

  bool ColorFilter::imageIsGrayscale(Mat image)
  {
    // Check whether the original image is grayscale.  If it is, we shouldn't attempt any color filter
    for (int row = 0; row < image.rows; row++)
    {
      for (int col = 0; col < image.cols; col++)
      {
        int r = (int) image.at<Vec3b>(row, col)[0];
        int g = (int) image.at<Vec3b>(row, col)[1];
        int b = (int) image.at<Vec3b>(row, col)[2];

        if (r == g == b)
        {
          // So far so good
        }
        else
        {
          // Image is color.
          return false;
        }
      }
    }

    return true;
  }

  void ColorFilter::preprocessImage()
  {
    // Equalize the brightness on the HSV channel "V"
    vector<Mat> channels;
    split(this->hsv,channels);
    Mat img_equalized = equalizeBrightness(channels[2]);
    merge(channels,this->hsv);
  }

  // Gets the hue/sat/val for areas that we believe are license plate characters
  // Then uses that to filter the whole image and provide a mask.
  void ColorFilter::findCharColors()
  {
    int MINIMUM_SATURATION = 45;

    if (this->debug)
      cout << "ColorFilter::findCharColors" << endl;

    //charMask.copyTo(this->colorMask);
    this->colorMask = Mat::zeros(charMask.size(), CV_8U);
    bitwise_not(this->colorMask, this->colorMask);

    Mat erodedCharMask(charMask.size(), CV_8U);
    Mat element = getStructuringElement( 1,
                                         Size( 2 + 1, 2+1 ),
                                         Point( 1, 1 ) );
    erode(charMask, erodedCharMask, element);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(erodedCharMask, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

    vector<float> hMeans, sMeans, vMeans;
    vector<float> hStdDevs, sStdDevs, vStdDevs;

    for (unsigned int i = 0; i < contours.size(); i++)
    {
      if (hierarchy[i][3] != -1)
        continue;

      Mat singleCharMask = Mat::zeros(hsv.size(), CV_8U);

      drawContours(singleCharMask, contours,
                   i, // draw this contour
                   cv::Scalar(255,255,255), // in
                   CV_FILLED,
                   8,
                   hierarchy
                  );

      // get rid of the outline by drawing a 1 pixel width black line
      drawContours(singleCharMask, contours,
                   i, // draw this contour
                   cv::Scalar(0,0,0), // in
                   1,
                   8,
                   hierarchy
                  );

      //drawAndWait(&singleCharMask);

      Scalar mean;
      Scalar stddev;
      meanStdDev(hsv, mean, stddev, singleCharMask);

      if (this->debug)
      {
        cout << "ColorFilter " << setw(3) << i << ". Mean:  h: " << setw(7) << mean[0] << " s: " << setw(7) <<mean[1] << " v: " << setw(7) << mean[2]
             << " | Std: h: " << setw(7) <<stddev[0] << " s: " << setw(7) <<stddev[1] << " v: " << stddev[2] << endl;
      }

      if (mean[0] == 0 && mean[1] == 0 && mean[2] == 0)
        continue;

      hMeans.push_back(mean[0]);
      sMeans.push_back(mean[1]);
      vMeans.push_back(mean[2]);
      hStdDevs.push_back(stddev[0]);
      sStdDevs.push_back(stddev[1]);
      vStdDevs.push_back(stddev[2]);
    }

    if (hMeans.size() == 0)
      return;

    int bestHueIndex = this->getMajorityOpinion(hMeans, .65, 30);
    int bestSatIndex = this->getMajorityOpinion(sMeans, .65, 35);
    int bestValIndex = this->getMajorityOpinion(vMeans, .65, 30);

    if (sMeans[bestSatIndex] < MINIMUM_SATURATION)
      return;

    bool doHueFilter = false, doSatFilter = false, doValFilter = false;
    float hueMin, hueMax;
    float satMin, satMax;
    float valMin, valMax;

    if (this->debug)
      cout << "ColorFilter Winning indices:" << endl;
    if (bestHueIndex != -1)
    {
      doHueFilter = true;
      hueMin = hMeans[bestHueIndex] - (2 * hStdDevs[bestHueIndex]);
      hueMax = hMeans[bestHueIndex] + (2 * hStdDevs[bestHueIndex]);

      if (abs(hueMin - hueMax) < 20)
      {
        hueMin = hMeans[bestHueIndex] - 20;
        hueMax = hMeans[bestHueIndex] + 20;
      }

      if (hueMin < 0)
        hueMin = 0;
      if (hueMax > 180)
        hueMax = 180;

      if (this->debug)
        cout << "ColorFilter Hue: " << bestHueIndex << " : " << setw(7) << hMeans[bestHueIndex] << " -- " << hueMin << "-" << hueMax << endl;
    }
    if (bestSatIndex != -1)
    {
      doSatFilter = true;

      satMin = sMeans[bestSatIndex] - (2 * sStdDevs[bestSatIndex]);
      satMax = sMeans[bestSatIndex] + (2 * sStdDevs[bestSatIndex]);

      if (abs(satMin - satMax) < 20)
      {
        satMin = sMeans[bestSatIndex] - 20;
        satMax = sMeans[bestSatIndex] + 20;
      }

      if (satMin < 0)
        satMin = 0;
      if (satMax > 255)
        satMax = 255;

      if (this->debug)
        cout << "ColorFilter Sat: " << bestSatIndex << " : " << setw(7) << sMeans[bestSatIndex] << " -- " << satMin << "-" << satMax << endl;
    }
    if (bestValIndex != -1)
    {
      doValFilter = true;

      valMin = vMeans[bestValIndex] - (1.5 * vStdDevs[bestValIndex]);
      valMax = vMeans[bestValIndex] + (1.5 * vStdDevs[bestValIndex]);

      if (abs(valMin - valMax) < 20)
      {
        valMin = vMeans[bestValIndex] - 20;
        valMax = vMeans[bestValIndex] + 20;
      }

      if (valMin < 0)
        valMin = 0;
      if (valMax > 255)
        valMax = 255;

      if (this->debug)
        cout << "ColorFilter Val: " << bestValIndex << " : " << setw(7) << vMeans[bestValIndex] << " -- " << valMin << "-" << valMax  << endl;
    }

    Mat imgDebugHueOnly = Mat::zeros(hsv.size(), hsv.type());
    Mat imgDebug = Mat::zeros(hsv.size(), hsv.type());
    Mat imgDistanceFromCenter = Mat::zeros(hsv.size(), CV_8U);
    Mat debugMask = Mat::zeros(hsv.size(), CV_8U);
    bitwise_not(debugMask, debugMask);

    for (int row = 0; row < charMask.rows; row++)
    {
      for (int col = 0; col < charMask.cols; col++)
      {
        int h = (int) hsv.at<Vec3b>(row, col)[0];
        int s = (int) hsv.at<Vec3b>(row, col)[1];
        int v = (int) hsv.at<Vec3b>(row, col)[2];

        bool hPasses = true;
        bool sPasses = true;
        bool vPasses = true;

        int vDistance = abs(v - vMeans[bestValIndex]);

        imgDebugHueOnly.at<Vec3b>(row, col)[0] = h;
        imgDebugHueOnly.at<Vec3b>(row, col)[1] = 255;
        imgDebugHueOnly.at<Vec3b>(row, col)[2] = 255;

        imgDebug.at<Vec3b>(row, col)[0] = 255;
        imgDebug.at<Vec3b>(row, col)[1] = 255;
        imgDebug.at<Vec3b>(row, col)[2] = 255;

        if (doHueFilter && (h < hueMin || h > hueMax))
        {
          hPasses = false;
          imgDebug.at<Vec3b>(row, col)[0] = 0;
          debugMask.at<uchar>(row, col) = 0;
        }
        if (doSatFilter && (s < satMin || s > satMax))
        {
          sPasses = false;
          imgDebug.at<Vec3b>(row, col)[1] = 0;
        }
        if (doValFilter && (v < valMin || v > valMax))
        {
          vPasses = false;
          imgDebug.at<Vec3b>(row, col)[2] = 0;
        }

        //if (pixelPasses)
        //  colorMask.at<uchar>(row, col) = 255;
        //else
        //imgDebug.at<Vec3b>(row, col)[0] = hPasses & 255;
        //imgDebug.at<Vec3b>(row, col)[1] = sPasses & 255;
        //imgDebug.at<Vec3b>(row, col)[2] = vPasses & 255;

        if ((hPasses) ||  (hPasses && sPasses))//(hPasses && vPasses) || (sPasses && vPasses) ||
          this->colorMask.at<uchar>(row, col) = 255;
        else
          this->colorMask.at<uchar>(row, col) = 0;

        if ((hPasses && sPasses) || (hPasses && vPasses) || (sPasses && vPasses))
        {
          vDistance = pow(vDistance, 0.9);
        }
        else
        {
          vDistance = pow(vDistance, 1.1);
        }
        if (vDistance > 255)
          vDistance = 255;
        imgDistanceFromCenter.at<uchar>(row, col) = vDistance;
      }
    }

    vector<Mat> debugImagesSet;

    if (this->debug)
    {
      debugImagesSet.push_back(addLabel(charMask, "Charecter mask"));
      //debugImagesSet1.push_back(erodedCharMask);
      Mat maskCopy(colorMask.size(), colorMask.type());
      colorMask.copyTo(maskCopy);
      debugImagesSet.push_back(addLabel(maskCopy, "color Mask Before"));
    }

    Mat bigElement = getStructuringElement( 1,
                                            Size( 3 + 1, 3+1 ),
                                            Point( 1, 1 ) );

    Mat smallElement = getStructuringElement( 1,
                       Size( 1 + 1, 1+1 ),
                       Point( 1, 1 ) );

    morphologyEx(this->colorMask, this->colorMask, MORPH_CLOSE, bigElement);
    //dilate(this->colorMask, this->colorMask, bigElement);

    Mat combined(charMask.size(), charMask.type());
    bitwise_and(charMask, colorMask, combined);

    if (this->debug)
    {
      debugImagesSet.push_back(addLabel(colorMask, "Color Mask After"));

      debugImagesSet.push_back(addLabel(combined, "Combined"));

      //displayImage(config, "COLOR filter Mask", colorMask);
      debugImagesSet.push_back(addLabel(imgDebug, "Color filter Debug"));

      cvtColor(imgDebugHueOnly, imgDebugHueOnly, CV_HSV2BGR);
      debugImagesSet.push_back(addLabel(imgDebugHueOnly, "Color Filter Hue"));

      equalizeHist(imgDistanceFromCenter, imgDistanceFromCenter);
      debugImagesSet.push_back(addLabel(imgDistanceFromCenter, "COLOR filter Distance"));

      debugImagesSet.push_back(addLabel(debugMask, "COLOR Hues off"));

      Mat dashboard = drawImageDashboard(debugImagesSet, imgDebugHueOnly.type(), 3);
      displayImage(config, "Color Filter Images", dashboard);
    }
  }

  // Goes through an array of values, picks the winner based on the highest percentage of other values that are within the maxValDifference
  // Return -1 if it fails.
  int ColorFilter::getMajorityOpinion(vector<float> values, float minPercentAgreement, float maxValDifference)
  {
    float bestPercentAgreement = 0;
    float lowestOverallDiff = 1000000000;
    int bestPercentAgreementIndex = -1;

    for (unsigned int i = 0; i < values.size(); i++)
    {
      int valuesInRange = 0;
      float overallDiff = 0;
      for (unsigned int j = 0; j < values.size(); j++)
      {
        float diff = abs(values[i] - values[j]);
        if (diff < maxValDifference)
          valuesInRange++;

        overallDiff += diff;
      }

      float percentAgreement = ((float) valuesInRange) / ((float) values.size());
      if (overallDiff < lowestOverallDiff && percentAgreement >= bestPercentAgreement && percentAgreement >= minPercentAgreement)
      {
        bestPercentAgreement = percentAgreement;
        bestPercentAgreementIndex = i;
        lowestOverallDiff = overallDiff;
      }
    }

    return bestPercentAgreementIndex;
  }

}