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

#include "platelines.h"

using namespace cv;
using namespace std;

const float MIN_CONFIDENCE = 0.3;

namespace alpr
{

  PlateLines::PlateLines(PipelineData* pipelineData)
  {
    this->pipelineData = pipelineData;

    this->debug = pipelineData->config->debugPlateLines;

    if (debug)
      cout << "PlateLines constructor" << endl;
  }

  PlateLines::~PlateLines()
  {
  }

  void PlateLines::processImage(Mat inputImage, vector<TextLine> textLines, float sensitivity)
  {
    if (this->debug)
      cout << "PlateLines findLines" << endl;

    timespec startTime;
    getTimeMonotonic(&startTime);


    // Ignore input images that are pure white or pure black
    Scalar avgPixelIntensity = mean(inputImage);
    if (avgPixelIntensity[0] >= 252)
      return;
    else if (avgPixelIntensity[0] <= 3)
      return;

    // Do a bilateral filter to clean the noise but keep edges sharp
    Mat smoothed(inputImage.size(), inputImage.type());
    bilateralFilter(inputImage, smoothed, 3, 45, 45);

    Mat edges(inputImage.size(), inputImage.type());
    Canny(smoothed, edges, 66, 133);

    // Create a mask that is dilated based on the detected characters


    Mat mask = Mat::zeros(inputImage.size(), CV_8U);

    for (unsigned int i = 0; i < textLines.size(); i++)
    {
      vector<vector<Point> > polygons;
      polygons.push_back(textLines[i].textArea);
      fillPoly(mask, polygons, Scalar(255,255,255));
    }



    dilate(mask, mask, getStructuringElement( 1, Size( 1 + 1, 2*1+1 ), Point( 1, 1 ) ));
    bitwise_not(mask, mask);

    // AND canny edges with the character mask
    bitwise_and(edges, mask, edges);


    vector<PlateLine> hlines = this->getLines(edges, sensitivity, false);
    vector<PlateLine> vlines = this->getLines(edges, sensitivity, true);
    for (unsigned int i = 0; i < hlines.size(); i++)
      this->horizontalLines.push_back(hlines[i]);
    for (unsigned int i = 0; i < vlines.size(); i++)
      this->verticalLines.push_back(vlines[i]);

    // if debug is enabled, draw the image
    if (this->debug)
    {
      Mat debugImgHoriz(edges.size(), edges.type());
      Mat debugImgVert(edges.size(), edges.type());
      edges.copyTo(debugImgHoriz);
      edges.copyTo(debugImgVert);
      cvtColor(debugImgHoriz,debugImgHoriz,CV_GRAY2BGR);
      cvtColor(debugImgVert,debugImgVert,CV_GRAY2BGR);

      for( size_t i = 0; i < this->horizontalLines.size(); i++ )
      {
        line( debugImgHoriz, this->horizontalLines[i].line.p1, this->horizontalLines[i].line.p2, Scalar(0,0,255), 1, CV_AA);
      }

      for( size_t i = 0; i < this->verticalLines.size(); i++ )
      {
        line( debugImgVert, this->verticalLines[i].line.p1, this->verticalLines[i].line.p2, Scalar(0,0,255), 1, CV_AA);
      }

      vector<Mat> images;
      images.push_back(debugImgHoriz);
      images.push_back(debugImgVert);

      Mat dashboard = drawImageDashboard(images, debugImgVert.type(), 1);
      displayImage(pipelineData->config, "Hough Lines", dashboard);
    }

    if (pipelineData->config->debugTiming)
    {
      timespec endTime;
      getTimeMonotonic(&endTime);
      cout << "Plate Lines Time: " << diffclock(startTime, endTime) << "ms." << endl;
    }

  }




  vector<PlateLine> PlateLines::getLines(Mat edges, float sensitivityMultiplier, bool vertical)
  {
    if (this->debug)
      cout << "PlateLines::getLines" << endl;

    int HORIZONTAL_SENSITIVITY = pipelineData->config->plateLinesSensitivityHorizontal;
    int VERTICAL_SENSITIVITY = pipelineData->config->plateLinesSensitivityVertical;

    vector<Vec2f> allLines;
    vector<PlateLine> filteredLines;

    int sensitivity;
    if (vertical)
      sensitivity = VERTICAL_SENSITIVITY * (1.0 / sensitivityMultiplier);
    else
      sensitivity = HORIZONTAL_SENSITIVITY * (1.0 / sensitivityMultiplier);

    HoughLines( edges, allLines, 1, CV_PI/180, sensitivity, 0, 0 );

    for( size_t i = 0; i < allLines.size(); i++ )
    {
      float rho = allLines[i][0], theta = allLines[i][1];
      Point pt1, pt2;
      double a = cos(theta), b = sin(theta);
      double x0 = a*rho, y0 = b*rho;

      double angle = theta * (180 / CV_PI);
      pt1.x = cvRound(x0 + 1000*(-b));
      pt1.y = cvRound(y0 + 1000*(a));
      pt2.x = cvRound(x0 - 1000*(-b));
      pt2.y = cvRound(y0 - 1000*(a));

      if (vertical)
      {
        if (angle < 20 || angle > 340 || (angle > 160 && angle < 210))
        {
          // good vertical

          LineSegment line;
          if (pt1.y <= pt2.y)
            line = LineSegment(pt2.x, pt2.y, pt1.x, pt1.y);
          else
            line = LineSegment(pt1.x, pt1.y, pt2.x, pt2.y);

          // Get rid of the -1000, 1000 stuff.  Terminate at the edges of the image
          // Helps with debugging/rounding issues later
          LineSegment top(0, 0, edges.cols, 0);
          LineSegment bottom(0, edges.rows, edges.cols, edges.rows);
          Point p1 = line.intersection(bottom);
          Point p2 = line.intersection(top);

          PlateLine plateLine;
          plateLine.line = LineSegment(p1.x, p1.y, p2.x, p2.y);
          plateLine.confidence = (1.0 - MIN_CONFIDENCE) * ((float) (allLines.size() - i)) / ((float)allLines.size()) + MIN_CONFIDENCE;
          filteredLines.push_back(plateLine);
        }
      }
      else
      {
        if ( (angle > 70 && angle < 110) || (angle > 250 && angle < 290))
        {
          // good horizontal

          LineSegment line;
          if (pt1.x <= pt2.x)
            line = LineSegment(pt1.x, pt1.y, pt2.x, pt2.y);
          else
            line =LineSegment(pt2.x, pt2.y, pt1.x, pt1.y);

          // Get rid of the -1000, 1000 stuff.  Terminate at the edges of the image
          // Helps with debugging/ rounding issues later
          int newY1 = line.getPointAt(0);
          int newY2 = line.getPointAt(edges.cols);

          PlateLine plateLine;
          plateLine.line = LineSegment(0, newY1, edges.cols, newY2);
          plateLine.confidence = (1.0 - MIN_CONFIDENCE) * ((float) (allLines.size() - i)) / ((float)allLines.size()) + MIN_CONFIDENCE;
          filteredLines.push_back(plateLine);
        }
      }
    }

    return filteredLines;
  }

  Mat PlateLines::customGrayscaleConversion(Mat src)
  {
    Mat img_hsv;
    cvtColor(src,img_hsv,CV_BGR2HSV);

    Mat grayscale = Mat(img_hsv.size(), CV_8U );
    Mat hue(img_hsv.size(), CV_8U );

    for (int row = 0; row < img_hsv.rows; row++)
    {
      for (int col = 0; col < img_hsv.cols; col++)
      {
        int h = (int) img_hsv.at<Vec3b>(row, col)[0];
        //int s = (int) img_hsv.at<Vec3b>(row, col)[1];
        int v = (int) img_hsv.at<Vec3b>(row, col)[2];

        int pixval = pow(v, 1.05);

        if (pixval > 255)
          pixval = 255;
        grayscale.at<uchar>(row, col) = pixval;

        hue.at<uchar>(row, col) = h * (255.0 / 180.0);
      }
    }

    //displayImage(config, "Hue", hue);
    return grayscale;
  }

}
