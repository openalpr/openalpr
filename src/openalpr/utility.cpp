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

#include "utility.h"

Rect expandRect(Rect original, int expandXPixels, int expandYPixels, int maxX, int maxY)
{
  Rect expandedRegion = Rect(original);

  float halfX = round((float) expandXPixels / 2.0);
  float halfY = round((float) expandYPixels / 2.0);
  expandedRegion.x = expandedRegion.x - halfX;
  expandedRegion.width =  expandedRegion.width + expandXPixels;
  expandedRegion.y = expandedRegion.y - halfY;
  expandedRegion.height =  expandedRegion.height + expandYPixels;

  if (expandedRegion.x < 0)
    expandedRegion.x = 0;
  if (expandedRegion.y < 0)
    expandedRegion.y = 0;
  if (expandedRegion.x + expandedRegion.width > maxX)
    expandedRegion.width = maxX - expandedRegion.x;
  if (expandedRegion.y + expandedRegion.height > maxY)
    expandedRegion.height = maxY - expandedRegion.y;

  return expandedRegion;
}

Mat drawImageDashboard(vector<Mat> images, int imageType, int numColumns)
{
  int numRows = ceil((float) images.size() / (float) numColumns);

  Mat dashboard(Size(images[0].cols * numColumns, images[0].rows * numRows), imageType);

  for (int i = 0; i < numColumns * numRows; i++)
  {
    if (i < images.size())
      images[i].copyTo(dashboard(Rect((i%numColumns) * images[i].cols, floor((float) i/numColumns) * images[i].rows, images[i].cols, images[i].rows)));
    else
    {
      Mat black = Mat::zeros(images[0].size(), imageType);
      black.copyTo(dashboard(Rect((i%numColumns) * images[0].cols, floor((float) i/numColumns) * images[0].rows, images[0].cols, images[0].rows)));
    }
  }

  return dashboard;
}

Mat addLabel(Mat input, string label)
{
  const int border_size = 1;
  const Scalar border_color(0,0,255);
  const int extraHeight = 20;
  const Scalar bg(222,222,222);
  const Scalar fg(0,0,0);

  Rect destinationRect(border_size, extraHeight, input.cols, input.rows);
  Mat newImage(Size(input.cols + (border_size), input.rows + extraHeight + (border_size )), input.type());
  input.copyTo(newImage(destinationRect));

  cout << " Adding label " << label << endl;
  if (input.type() == CV_8U)
    cvtColor(newImage, newImage, CV_GRAY2BGR);

  rectangle(newImage, Point(0,0), Point(input.cols, extraHeight), bg, CV_FILLED);
  putText(newImage, label, Point(5, extraHeight - 5), CV_FONT_HERSHEY_PLAIN  , 0.7, fg);

  rectangle(newImage, Point(0,0), Point(newImage.cols - 1, newImage.rows -1), border_color, border_size);

  return newImage;
}

void drawAndWait(cv::Mat* frame)
{
  cv::imshow("Temp Window", *frame);

  while (cv::waitKey(50) == -1)
  {
    // loop
  }

  cv::destroyWindow("Temp Window");
}

void displayImage(Config* config, string windowName, cv::Mat frame)
{
  if (config->debugShowImages)
    imshow(windowName, frame);
}

vector<Mat> produceThresholds(const Mat img_gray, Config* config)
{
  const int THRESHOLD_COUNT = 4;
  //Mat img_equalized = equalizeBrightness(img_gray);

  timespec startTime;
  getTime(&startTime);

  vector<Mat> thresholds;

  for (int i = 0; i < THRESHOLD_COUNT; i++)
    thresholds.push_back(Mat(img_gray.size(), CV_8U));

  int i = 0;

  // Adaptive
  //adaptiveThreshold(img_gray, thresholds[i++], 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV , 7, 3);
  //adaptiveThreshold(img_gray, thresholds[i++], 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV , 13, 3);
  //adaptiveThreshold(img_gray, thresholds[i++], 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV , 17, 3);

  // Wolf
  int k = 0, win=18;
  //NiblackSauvolaWolfJolion (img_gray, thresholds[i++], WOLFJOLION, win, win, 0.05 + (k * 0.35));
  //bitwise_not(thresholds[i-1], thresholds[i-1]);
  NiblackSauvolaWolfJolion (img_gray, thresholds[i++], WOLFJOLION, win, win, 0.05 + (k * 0.35));
  bitwise_not(thresholds[i-1], thresholds[i-1]);

  k = 1;
  win = 22;
  NiblackSauvolaWolfJolion (img_gray, thresholds[i++], WOLFJOLION, win, win, 0.05 + (k * 0.35));
  bitwise_not(thresholds[i-1], thresholds[i-1]);
  //NiblackSauvolaWolfJolion (img_gray, thresholds[i++], WOLFJOLION, win, win, 0.05 + (k * 0.35));
  //bitwise_not(thresholds[i-1], thresholds[i-1]);

  // Sauvola
  k = 1;
  NiblackSauvolaWolfJolion (img_gray, thresholds[i++], SAUVOLA, 12, 12, 0.18 * k);
  bitwise_not(thresholds[i-1], thresholds[i-1]);
  k=2;
  NiblackSauvolaWolfJolion (img_gray, thresholds[i++], SAUVOLA, 12, 12, 0.18 * k);
  bitwise_not(thresholds[i-1], thresholds[i-1]);

  if (config->debugTiming)
  {
    timespec endTime;
    getTime(&endTime);
    cout << "  -- Produce Threshold Time: " << diffclock(startTime, endTime) << "ms." << endl;
  }

  return thresholds;
  //threshold(img_equalized, img_threshold, 100, 255, THRESH_BINARY);
}

double median(int array[], int arraySize)
{
  if (arraySize == 0)
  {
    //std::cerr << "Median calculation requested on empty array" << endl;
    return 0;
  }

  std::sort(&array[0], &array[arraySize]);
  return arraySize % 2 ? array[arraySize / 2] : (array[arraySize / 2 - 1] + array[arraySize / 2]) / 2;
}

Mat equalizeBrightness(Mat img)
{
  // Divide the image by its morphologically closed counterpart
  Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(19,19));
  Mat closed;
  morphologyEx(img, closed, MORPH_CLOSE, kernel);

  img.convertTo(img, CV_32FC1); // divide requires floating-point
  divide(img, closed, img, 1, CV_32FC1);
  normalize(img, img, 0, 255, NORM_MINMAX);
  img.convertTo(img, CV_8U); // convert back to unsigned int

  return img;
}

void drawRotatedRect(Mat* img, RotatedRect rect, Scalar color, int thickness)
{
  Point2f rect_points[4];
  rect.points( rect_points );
  for( int j = 0; j < 4; j++ )
    line( *img, rect_points[j], rect_points[(j+1)%4], color, thickness, 8 );
}

void fillMask(Mat img, const Mat mask, Scalar color)
{
  for (int row = 0; row < img.rows; row++)
  {
    for (int col = 0; col < img.cols; col++)
    {
      int m = (int) mask.at<uchar>(row, col);

      if (m)
      {
        for (int z = 0; z < 3; z++)
        {
          int prevVal = img.at<Vec3b>(row, col)[z];
          img.at<Vec3b>(row, col)[z] = ((int) color[z]) | prevVal;
        }
      }
    }
  }
}

void drawX(Mat img, Rect rect, Scalar color, int thickness)
{
  Point tl(rect.x, rect.y);
  Point tr(rect.x + rect.width, rect.y);
  Point bl(rect.x, rect.y + rect.height);
  Point br(rect.x + rect.width, rect.y + rect.height);

  line(img, tl, br, color, thickness);
  line(img, bl, tr, color, thickness);
}

double distanceBetweenPoints(Point p1, Point p2)
{
  float asquared = (p2.x - p1.x)*(p2.x - p1.x);
  float bsquared = (p2.y - p1.y)*(p2.y - p1.y);

  return sqrt(asquared + bsquared);
}

float angleBetweenPoints(Point p1, Point p2)
{
  int deltaY = p2.y - p1.y;
  int deltaX = p2.x - p1.x;

  return atan2((float) deltaY, (float) deltaX) * (180 / CV_PI);
}

Size getSizeMaintainingAspect(Mat inputImg, int maxWidth, int maxHeight)
{
  float aspect = ((float) inputImg.cols) / ((float) inputImg.rows);

  if (maxWidth / aspect > maxHeight)
  {
    return Size(maxHeight * aspect, maxHeight);
  }
  else
  {
    return Size(maxWidth, maxWidth / aspect);
  }
}

LineSegment::LineSegment()
{
  init(0, 0, 0, 0);
}

LineSegment::LineSegment(Point p1, Point p2)
{
  init(p1.x, p1.y, p2.x, p2.y);
}

LineSegment::LineSegment(int x1, int y1, int x2, int y2)
{
  init(x1, y1, x2, y2);
}

void LineSegment::init(int x1, int y1, int x2, int y2)
{
  this->p1 = Point(x1, y1);
  this->p2 = Point(x2, y2);

  if (p2.x - p1.x == 0)
    this->slope = 0.00000000001;
  else
    this->slope = (float) (p2.y - p1.y) / (float) (p2.x - p1.x);

  this->length = distanceBetweenPoints(p1, p2);

  this->angle = angleBetweenPoints(p1, p2);
}

bool LineSegment::isPointBelowLine( Point tp )
{
  return ((p2.x - p1.x)*(tp.y - p1.y) - (p2.y - p1.y)*(tp.x - p1.x)) > 0;
}

float LineSegment::getPointAt(float x)
{
  return slope * (x - p2.x) + p2.y;
}

Point LineSegment::closestPointOnSegmentTo(Point p)
{
  float top = (p.x - p1.x) * (p2.x - p1.x) + (p.y - p1.y)*(p2.y - p1.y);

  float bottom = distanceBetweenPoints(p2, p1);
  bottom = bottom * bottom;

  float u = top / bottom;

  float x = p1.x + u * (p2.x - p1.x);
  float y = p1.y + u * (p2.y - p1.y);

  return Point(x, y);
}

Point LineSegment::intersection(LineSegment line)
{
  float c1, c2;
  float intersection_X = -1, intersection_Y= -1;

  c1 = p1.y - slope * p1.x; // which is same as y2 - slope * x2

  c2 = line.p2.y - line.slope * line.p2.x; // which is same as y2 - slope * x2

  if( (slope - line.slope) == 0)
  {
    //std::cout << "No Intersection between the lines" << endl;
  }
  else if (p1.x == p2.x)
  {
    // Line1 is vertical
    return Point(p1.x, line.getPointAt(p1.x));
  }
  else if (line.p1.x == line.p2.x)
  {
    // Line2 is vertical
    return Point(line.p1.x, getPointAt(line.p1.x));
  }
  else
  {
    intersection_X = (c2 - c1) / (slope - line.slope);
    intersection_Y = slope * intersection_X + c1;
  }

  return Point(intersection_X, intersection_Y);
}

Point LineSegment::midpoint()
{
  // Handle the case where the line is vertical
  if (p1.x == p2.x)
  {
    float ydiff = p2.y-p1.y;
    float y = p1.y + (ydiff/2);
    return Point(p1.x, y);
  }
  float diff = p2.x - p1.x;
  float midX = ((float) p1.x) + (diff / 2);
  int midY = getPointAt(midX);

  return Point(midX, midY);
}

LineSegment LineSegment::getParallelLine(float distance)
{
  float diff_x = p2.x - p1.x;
  float diff_y = p2.y - p1.y;
  float angle = atan2( diff_x, diff_y);
  float dist_x = distance * cos(angle);
  float dist_y = -distance * sin(angle);

  int offsetX = (int)round(dist_x);
  int offsetY = (int)round(dist_y);

  LineSegment result(p1.x + offsetX, p1.y + offsetY,
                     p2.x + offsetX, p2.y + offsetY);

  return result;
}
