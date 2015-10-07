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

#include <opencv2/core/core.hpp>
#include <functional>
#include <cctype>

#include "utility.h"

using namespace cv;
using namespace std;

namespace alpr
{

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
    if (expandedRegion.width < 0)
      expandedRegion.width = 0;
    if (expandedRegion.height < 0)
      expandedRegion.height = 0;
    
    return expandedRegion;
  }

  Mat drawImageDashboard(vector<Mat> images, int imageType, unsigned int numColumns)
  {
    unsigned int numRows = ceil((float) images.size() / (float) numColumns);

    Mat dashboard(Size(images[0].cols * numColumns, images[0].rows * numRows), imageType);

    for (unsigned int i = 0; i < numColumns * numRows; i++)
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

  void drawAndWait(cv::Mat frame)
  {
    drawAndWait(&frame);
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
    {
      imshow(windowName, frame);
      cv::waitKey(5);
    }
  }

  vector<Mat> produceThresholds(const Mat img_gray, Config* config)
  {
    const int THRESHOLD_COUNT = 3;
    //Mat img_equalized = equalizeBrightness(img_gray);

    timespec startTime;
    getTimeMonotonic(&startTime);

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
    //k=2;
    //NiblackSauvolaWolfJolion (img_gray, thresholds[i++], SAUVOLA, 12, 12, 0.18 * k);
    //bitwise_not(thresholds[i-1], thresholds[i-1]);

    if (config->debugTiming)
    {
      timespec endTime;
      getTimeMonotonic(&endTime);
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


// Compares two strings and computes the edit distance between them
// http://en.wikipedia.org/wiki/Levenshtein_distance
// max is the cutoff (i.e., max distance) where we stop trying to find distance
int levenshteinDistance (const std::string &s1, const std::string &s2, int max)
{
    const char* word1 = s1.c_str();
    int len1 = s1.length();
    const char* word2 = s2.c_str();
    int len2 = s2.length();
    max--;
  
    //int matrix[2][len2 + 1];
    std::vector<std::vector<int> > matrix;
    for (unsigned int i = 0; i < 2; i++)
    {
      std::vector<int> top_elem;
      matrix.push_back(top_elem);
      for (unsigned int j = 0; j < len2 + 1; j++)
        matrix[i].push_back(0);
    }
    int i;
    int j;
    
    /*
      Initialize the 0 row of "matrix".

        0  
        1  
        2  
        3  

     */

    for (j = 0; j <= len2; j++) {
        matrix[0][j] = j;
    }

    /* Loop over column. */
    for (i = 1; i <= len1; i++) {
        char c1;
        /* The first value to consider of the ith column. */
        int min_j;
        /* The last value to consider of the ith column. */
        int max_j;
        /* The smallest value of the matrix in the ith column. */
        int col_min;
        /* The next column of the matrix to fill in. */
        int next;
        /* The previously-filled-in column of the matrix. */
        int prev;

        c1 = word1[i-1];
        min_j = 1;
        if (i > max) {
            min_j = i - max;
        }
        max_j = len2;
        if (len2 > max + i) {
            max_j = max + i;
        }
        col_min = INT_MAX;
        next = i % 2;
        if (next == 1) {
            prev = 0;
        }
        else {
            prev = 1;
        }
        matrix[next][0] = i;
        /* Loop over rows. */
        for (j = 1; j <= len2; j++) {
            if (j < min_j || j > max_j) {
                /* Put a large value in there. */
                matrix[next][j] = max + 1;
            }
            else {
                char c2;

                c2 = word2[j-1];
                if (c1 == c2) {
                    /* The character at position i in word1 is the same as
                       the character at position j in word2. */
                    matrix[next][j] = matrix[prev][j-1];
                }
                else {
                    /* The character at position i in word1 is not the
                       same as the character at position j in word2, so
                       work out what the minimum cost for getting to cell
                       i, j is. */
                    int del;
                    int insert;
                    int substitute;
                    int minimum;

                    del = matrix[prev][j] + 1;
                    insert = matrix[next][j-1] + 1;
                    substitute = matrix[prev][j-1] + 1;
                    minimum = del;
                    if (insert < minimum) {
                        minimum = insert;
                    }
                    if (substitute < minimum) {
                        minimum = substitute;
                    }
                    matrix[next][j] = minimum;
                }
            }
            /* Find the minimum value in the ith column. */
            if (matrix[next][j] < col_min) {
                col_min = matrix[next][j];
            }
        }
        if (col_min > max) {
            /* All the elements of the ith column are greater than the
               maximum, so no match less than or equal to max can be
               found by looking at succeeding columns. */
            return max + 1;
        }
    }
    int returnval = matrix[len1 % 2][len2];
    if (returnval > max + 1)
      returnval = max + 1;
    return returnval;
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

  float LineSegment::getXPointAt(float y)
  {
    float y_intercept = getPointAt(0);
    return (y - y_intercept) / slope;
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

  // Given a contour and a mask, this function determines what percentage of the contour (area)
  // is inside the masked area. 
  float getContourAreaPercentInsideMask(cv::Mat mask, std::vector<std::vector<cv::Point> > contours, std::vector<cv::Vec4i> hierarchy, int contourIndex)
  {


    Mat innerArea = Mat::zeros(mask.size(), CV_8U);


    drawContours(innerArea, contours,
                 contourIndex, // draw this contour
                 cv::Scalar(255,255,255), // in
                 CV_FILLED,
                 8,
                 hierarchy,
                 2
                );


    int startingPixels = cv::countNonZero(innerArea);
    //drawAndWait(&innerArea);

    bitwise_and(innerArea, mask, innerArea);

    int endingPixels = cv::countNonZero(innerArea);
    //drawAndWait(&innerArea);

    return ((float) endingPixels) / ((float) startingPixels);

  }

  std::string toString(int value)
  {
    stringstream ss;
    ss << value;
    return ss.str();
  }
  std::string toString(long value)
  {
    stringstream ss;
    ss << value;
    return ss.str();
  }
  std::string toString(unsigned int value)
  {
    return toString((int) value);
  }
  std::string toString(float value)
  {
    stringstream ss;
    ss << value;
    return ss.str();
  }
  std::string toString(double value)
  {
    stringstream ss;
    ss << value;
    return ss.str();
  }

// trim from start
  std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
  }

// trim from end
  std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
  }

// trim from both ends
  std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
  }

}