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

#ifndef OPENALPR_UTILITY_H
#define OPENALPR_UTILITY_H

#include <iostream>
#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "support/timing.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "binarize_wolf.h"
#include <vector>
#include "config.h"

/*
struct LineSegment
{
   float x1;
   float y1;
   float x2;
   float y2;
};
*/

class LineSegment
{

  public:
    Point p1, p2;
    float slope;
    float length;
    float angle;

    // LineSegment(Point point1, Point point2);
    LineSegment();
    LineSegment(int x1, int y1, int x2, int y2);
    LineSegment(Point p1, Point p2);

    void init(int x1, int y1, int x2, int y2);

    bool isPointBelowLine(Point tp);

    float getPointAt(float x);

    Point closestPointOnSegmentTo(Point p);

    Point intersection(LineSegment line);

    LineSegment getParallelLine(float distance);

    Point midpoint();

    inline std::string str()
    {
      std::stringstream ss;
      ss << "(" << p1.x << ", " << p1.y << ") : (" << p2.x << ", " << p2.y << ")";
      return ss.str() ;
    }

};

double median(int array[], int arraySize);

vector<Mat> produceThresholds(const Mat img_gray, Config* config);

Mat drawImageDashboard(vector<Mat> images, int imageType, int numColumns);

void displayImage(Config* config, string windowName, cv::Mat frame);
void drawAndWait(cv::Mat* frame);

double distanceBetweenPoints(Point p1, Point p2);

void drawRotatedRect(Mat* img, RotatedRect rect, Scalar color, int thickness);

void drawX(Mat img, Rect rect, Scalar color, int thickness);
void fillMask(Mat img, const Mat mask, Scalar color);

float angleBetweenPoints(Point p1, Point p2);

Size getSizeMaintainingAspect(Mat inputImg, int maxWidth, int maxHeight);

Mat equalizeBrightness(Mat img);

Rect expandRect(Rect original, int expandXPixels, int expandYPixels, int maxX, int maxY);

Mat addLabel(Mat input, string label);

#endif // OPENALPR_UTILITY_H
