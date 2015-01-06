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

#ifndef OPENALPR_REGIONDETECTOR_H
#define OPENALPR_REGIONDETECTOR_H

#include <iostream>
#include <stdio.h>


#include "utility.h"
#include "support/timing.h"
#include "constants.h"

namespace alpr
{

  struct PlateRegion
  {
    cv::Rect rect;
    std::vector<PlateRegion> children;
  };


  class Detector
  {

    public:
      Detector(Config* config);
      virtual ~Detector();

      bool isLoaded();
      std::vector<PlateRegion> detect(cv::Mat frame);
      virtual std::vector<PlateRegion> detect(cv::Mat frame, std::vector<cv::Rect> regionsOfInterest);

    protected:
      Config* config;

      bool loaded;
      float scale_factor;

      std::vector<PlateRegion> aggregateRegions(std::vector<cv::Rect> regions);



  };

}

#endif // OPENALPR_REGIONDETECTOR_H
