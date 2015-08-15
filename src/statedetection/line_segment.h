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

#ifndef OPENALPR_LINE_SEGMENT_H
#define OPENALPR_LINE_SEGMENT_H

#include "opencv2/core/core.hpp"

class LineSegment
{

public:
  cv::Point p1, p2;
  float slope;
  float length;
  float angle;

  // LineSegment(Point point1, Point point2);
  LineSegment();
  LineSegment(int x1, int y1, int x2, int y2);
  LineSegment(cv::Point p1, cv::Point p2);

  void init(int x1, int y1, int x2, int y2);

  bool isPointBelowLine(cv::Point tp);

  float getPointAt(float x);

  cv::Point closestPointOnSegmentTo(cv::Point p);

  cv::Point intersection(LineSegment line);

  LineSegment getParallelLine(float distance);

  cv::Point midpoint();

  inline std::string str()
  {
    std::stringstream ss;
    ss << "(" << p1.x << ", " << p1.y << ") : (" << p2.x << ", " << p2.y << ")";
    return ss.str() ;
  }
private:

  double distanceBetweenPoints(cv::Point p1, cv::Point p2);
  float angleBetweenPoints(cv::Point p1, cv::Point p2);
  
};


#endif //OPENALPR_LINE_SEGMENT_H
