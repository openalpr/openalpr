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


#ifndef OPENALPR_TEXTLINE_H
#define	OPENALPR_TEXTLINE_H

#include "utility.h"
#include "opencv2/imgproc/imgproc.hpp"

namespace alpr
{

  class TextLine {
  public:
    TextLine(std::vector<cv::Point> textArea, std::vector<cv::Point> linePolygon);
    TextLine(std::vector<cv::Point2f> textArea, std::vector<cv::Point2f> linePolygon);
    virtual ~TextLine();


    std::vector<cv::Point> linePolygon;
    std::vector<cv::Point> textArea;
    LineSegment topLine;
    LineSegment bottomLine;

    LineSegment charBoxTop;
    LineSegment charBoxBottom;
    LineSegment charBoxLeft;
    LineSegment charBoxRight;

    float lineHeight;
    float angle;

    cv::Mat drawDebugImage(cv::Mat baseImage);
  private:

    void initialize(std::vector<cv::Point> textArea, std::vector<cv::Point> linePolygon);
  };

}

#endif	/* OPENALPR_TEXTLINE_H */

