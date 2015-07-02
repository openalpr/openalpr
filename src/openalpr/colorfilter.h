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

#ifndef OPENALPR_COLORFILTER_H
#define OPENALPR_COLORFILTER_H

#include <iomanip>
#include "opencv2/imgproc/imgproc.hpp"

#include "constants.h"
#include "utility.h"
#include "config.h"

namespace alpr
{

  class ColorFilter
  {

    public:
      ColorFilter(cv::Mat image, cv::Mat characterMask, Config* config);
      virtual ~ColorFilter();

      cv::Mat colorMask;

    private:

      Config* config;
      bool debug;

      cv::Mat hsv;
      cv::Mat charMask;

      bool grayscale;

      void preprocessImage();
      void findCharColors();

      bool imageIsGrayscale(cv::Mat image);
      int getMajorityOpinion(std::vector<float> values, float minPercentAgreement, float maxValDifference);
  };
  
}
#endif // OPENALPR_COLORFILTER_H
