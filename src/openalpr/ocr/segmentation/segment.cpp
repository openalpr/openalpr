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

#include "segment.h"

namespace alpr
{

  Segment::Segment(cv::Rect newSegment)
  {
    this->segment = newSegment;
  }

  Segment::~Segment()
  {

  }

  bool Segment::matches(cv::Rect newSegment)
  {
    // Compare the two segments with a given leniency
    const float WIDTH_LENIENCY_MIN = 0.25;
    const float WIDTH_LENIENCY_MAX = 0.20;

    float left_min = segment.x - (((float)segment.width) * WIDTH_LENIENCY_MIN);
    float left_max = segment.x + (((float)segment.width) * WIDTH_LENIENCY_MAX);
    float right_min = (segment.x + segment.width) - (((float)segment.width) * WIDTH_LENIENCY_MIN);
    float right_max = (segment.x + segment.width) + (((float)segment.width) * WIDTH_LENIENCY_MAX);

    int newSegRight = newSegment.x + newSegment.width;
    if (newSegment.x >= left_min && newSegment.x <= left_max && 
      newSegRight >= right_min && newSegRight <= right_max)
      return true;

    return false;
  }

}