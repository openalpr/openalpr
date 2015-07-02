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


#include <opencv2/imgproc/imgproc.hpp>

#include "textline.h"

using namespace cv;

namespace alpr
{

  TextLine::TextLine(std::vector<cv::Point2f> textArea, std::vector<cv::Point2f> linePolygon, cv::Size imgSize) {
    std::vector<Point> textAreaInts, linePolygonInts;

    for (unsigned int i = 0; i < textArea.size(); i++)
      textAreaInts.push_back(Point(round(textArea[i].x), round(textArea[i].y)));
    for (unsigned int i = 0; i < linePolygon.size(); i++)
      linePolygonInts.push_back(Point(round(linePolygon[i].x), round(linePolygon[i].y)));

    initialize(textAreaInts, linePolygonInts, imgSize);
  }

  TextLine::TextLine(std::vector<cv::Point> textArea, std::vector<cv::Point> linePolygon, cv::Size imgSize) {
    initialize(textArea, linePolygon, imgSize);
  }


  TextLine::~TextLine() {
  }



  void TextLine::initialize(std::vector<cv::Point> textArea, std::vector<cv::Point> linePolygon, cv::Size imgSize) {
    if (textArea.size() > 0)
    {
      if (this->textArea.size() > 0)
        this->textArea.clear();
      if (this->linePolygon.size() > 0)
        this->linePolygon.clear();

      for (unsigned int i = 0; i < textArea.size(); i++)
        this->textArea.push_back(textArea[i]);

      this->topLine = LineSegment(linePolygon[0].x, linePolygon[0].y, linePolygon[1].x, linePolygon[1].y);
      this->bottomLine = LineSegment(linePolygon[3].x, linePolygon[3].y, linePolygon[2].x, linePolygon[2].y);
      
      // Adjust the line polygon so that it always touches the edges
      // This is needed after applying perspective transforms, so just fix it here
      if (linePolygon[0].x != 0)
      {
        linePolygon[0].x = 0;
        linePolygon[0].y = topLine.getPointAt(linePolygon[0].x);
      }
      if (linePolygon[1].x != imgSize.width)
      {
        linePolygon[1].x = imgSize.width;
        linePolygon[1].y = topLine.getPointAt(linePolygon[1].x);
      }
      if (linePolygon[2].x != imgSize.width)
      {
        linePolygon[2].x = imgSize.width;
        linePolygon[2].y = bottomLine.getPointAt(linePolygon[2].x);
      }
      if (linePolygon[3].x != 0)
      {
        linePolygon[3].x = 0;
        linePolygon[3].y = bottomLine.getPointAt(linePolygon[3].x);
      }
      
      
      for (unsigned int i = 0; i < linePolygon.size(); i++)
        this->linePolygon.push_back(linePolygon[i]);


      this->charBoxTop = LineSegment(textArea[0].x, textArea[0].y, textArea[1].x, textArea[1].y);
      this->charBoxBottom = LineSegment(textArea[3].x, textArea[3].y, textArea[2].x, textArea[2].y);
      this->charBoxLeft = LineSegment(textArea[3].x, textArea[3].y, textArea[0].x, textArea[0].y);
      this->charBoxRight = LineSegment(textArea[2].x, textArea[2].y, textArea[1].x, textArea[1].y);

      // Calculate line height
      float x = ((float) linePolygon[1].x) / 2;
      Point midpoint = Point(x, bottomLine.getPointAt(x));
      Point acrossFromMidpoint = topLine.closestPointOnSegmentTo(midpoint);
      this->lineHeight = distanceBetweenPoints(midpoint, acrossFromMidpoint);

      // Subtract a pixel since the height is a little overestimated by the bounding box
      this->lineHeight = this->lineHeight - 1;

      this->angle = (topLine.angle + bottomLine.angle) / 2;

    }
  }


  cv::Mat TextLine::drawDebugImage(cv::Mat baseImage) {
    cv::Mat debugImage(baseImage.size(), baseImage.type());

    baseImage.copyTo(debugImage);

    cv::cvtColor(debugImage, debugImage, CV_GRAY2BGR);


    fillConvexPoly(debugImage, linePolygon.data(), linePolygon.size(), Scalar(0,0,165));

    fillConvexPoly(debugImage, textArea.data(), textArea.size(), Scalar(125,255,0));

    line(debugImage, topLine.p1, topLine.p2, Scalar(255,0,0), 1);
    line(debugImage, bottomLine.p1, bottomLine.p2, Scalar(255,0,0), 1);

    line(debugImage, charBoxTop.p1, charBoxTop.p2, Scalar(0,125,125), 1);
    line(debugImage, charBoxLeft.p1, charBoxLeft.p2, Scalar(0,125,125), 1);
    line(debugImage, charBoxRight.p1, charBoxRight.p2, Scalar(0,125,125), 1);
    line(debugImage, charBoxBottom.p1, charBoxBottom.p2, Scalar(0,125,125), 1);


    return debugImage;
  }

}