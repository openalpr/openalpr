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

#ifndef OPENALPR_VERTICALHISTOGRAM_H
#define OPENALPR_VERTICALHISTOGRAM_H

#include "opencv2/imgproc/imgproc.hpp"
#include "histogram.h"
#include "utility.h"

namespace alpr
{



  class HistogramVertical : public Histogram
  {

  public:
    HistogramVertical(cv::Mat inputImage, cv::Mat mask);


  };

}
  
#endif // OPENALPR_VERTICALHISTOGRAM_H
