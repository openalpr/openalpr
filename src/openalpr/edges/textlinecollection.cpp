/* 
 * File:   textlinecollection.cpp
 * Author: mhill
 * 
 * Created on October 25, 2014, 4:06 PM
 */

#include "textlinecollection.h"

using namespace cv;
using namespace std;

namespace alpr
{

  TextLineCollection::TextLineCollection(std::vector<TextLine> textLines) {


    charHeight = 0;
    charAngle = 0;
    for (unsigned int i = 0; i < textLines.size(); i++)
    {
      charHeight += textLines[i].lineHeight;
      charAngle += textLines[i].angle;

    }
    charHeight = charHeight / textLines.size();
    charAngle = charAngle / textLines.size();

    this->topCharArea = textLines[0].charBoxTop;
    this->bottomCharArea = textLines[0].charBoxBottom;
    for (unsigned int i = 1; i < textLines.size(); i++)
    {

      if (this->topCharArea.isPointBelowLine(textLines[i].charBoxTop.midpoint()) == false)
        this->topCharArea = textLines[i].charBoxTop;

      if (this->bottomCharArea.isPointBelowLine(textLines[i].charBoxBottom.midpoint()))
        this->bottomCharArea = textLines[i].charBoxBottom;

    }

    longerSegment = this->bottomCharArea;
    shorterSegment = this->topCharArea;
    if (this->topCharArea.length > this->bottomCharArea.length)
    {
      longerSegment = this->topCharArea;
      shorterSegment = this->bottomCharArea;
    }

    findCenterHorizontal();
    findCenterVertical();
    // Center Vertical Line


  }

  cv::Mat TextLineCollection::getDebugImage(cv::Size imageSize) {

    Mat debugImage = Mat::zeros(imageSize, CV_8U);
    line(debugImage, this->centerHorizontalLine.p1, this->centerHorizontalLine.p2, Scalar(255,255,255), 2);
    line(debugImage, this->centerVerticalLine.p1, this->centerVerticalLine.p2, Scalar(255,255,255), 2);

    return debugImage;

  }


  // Returns 1 for above, 0 for within, and -1 for below
  int TextLineCollection::isAboveText(LineSegment line) {
    // Test four points (left and right corner of top and bottom line)

    Point topLeft = line.closestPointOnSegmentTo(topCharArea.p1);
    Point topRight = line.closestPointOnSegmentTo(topCharArea.p2);

    bool lineIsBelowTop = topCharArea.isPointBelowLine(topLeft) || topCharArea.isPointBelowLine(topRight);

    if (!lineIsBelowTop)
      return 1;

    Point bottomLeft = line.closestPointOnSegmentTo(bottomCharArea.p1);
    Point bottomRight = line.closestPointOnSegmentTo(bottomCharArea.p2);

    bool lineIsBelowBottom = bottomCharArea.isPointBelowLine(bottomLeft) &&
                            bottomCharArea.isPointBelowLine(bottomRight);

    if (lineIsBelowBottom)
      return -1;

    return 0;

  }

  // Returns 1 for left, 0 for within, and -1 for to the right
  int TextLineCollection::isLeftOfText(LineSegment line) {

    LineSegment leftSide = LineSegment(bottomCharArea.p1, topCharArea.p1);

    Point topLeft = line.closestPointOnSegmentTo(leftSide.p2);
    Point bottomLeft = line.closestPointOnSegmentTo(leftSide.p1);

    bool lineIsAboveLeft = (!leftSide.isPointBelowLine(topLeft)) && (!leftSide.isPointBelowLine(bottomLeft));

    if (lineIsAboveLeft)
      return 1;

    LineSegment rightSide = LineSegment(bottomCharArea.p2, topCharArea.p2);

    Point topRight = line.closestPointOnSegmentTo(rightSide.p2);
    Point bottomRight = line.closestPointOnSegmentTo(rightSide.p1);


    bool lineIsBelowRight = rightSide.isPointBelowLine(topRight) && rightSide.isPointBelowLine(bottomRight);

    if (lineIsBelowRight)
      return -1;

    return 0;
  }

  void TextLineCollection::findCenterHorizontal() {
    // To find the center horizontal line:
    // Find the longer of the lines (if multiline)
    // Get the nearest point on the bottom-most line for the 
    // left and right 



    Point leftP1 =  shorterSegment.closestPointOnSegmentTo(longerSegment.p1);
    Point leftP2 = longerSegment.p1;
    LineSegment left = LineSegment(leftP1, leftP2);

    Point leftMidpoint = left.midpoint();



    Point rightP1 =  shorterSegment.closestPointOnSegmentTo(longerSegment.p2);
    Point rightP2 = longerSegment.p2;
    LineSegment right = LineSegment(rightP1, rightP2);

    Point rightMidpoint = right.midpoint();

    this->centerHorizontalLine = LineSegment(leftMidpoint, rightMidpoint);

  }

  void TextLineCollection::findCenterVertical() {
    // To find the center vertical line:
    // Choose the longest line (if multiline)
    // Get the midpoint
    // Draw a line up/down using the closest point on the bottom line


    Point p1 = longerSegment.midpoint();

    Point p2 = shorterSegment.closestPointOnSegmentTo(p1);

    // Draw bottom to top
    if (p1.y < p2.y)
      this->centerVerticalLine = LineSegment(p1, p2);
    else
      this->centerVerticalLine = LineSegment(p2, p1);
  }

}