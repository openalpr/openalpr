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

#ifndef OPENALPR_REGIONDETECTOR_H
#define OPENALPR_REGIONDETECTOR_H

#include <string>
#include <vector>


#include "utility.h"
#include "detector_types.h"
#include "support/timing.h"
#include "constants.h"
#include "detectormask.h"
#include "prewarp.h"

namespace alpr
{



  class Detector
  {

    public:
      Detector(Config* config, PreWarp* prewarp);
      virtual ~Detector();

      bool isLoaded();
      std::vector<PlateRegion> detect(cv::Mat frame);
      std::vector<PlateRegion> detect(cv::Mat frame, std::vector<cv::Rect> regionsOfInterest);

      virtual std::vector<cv::Rect> find_plates(cv::Mat frame, cv::Size min_plate_size, cv::Size max_plate_size)=0;
      
      void setMask(cv::Mat mask);
      
    protected:
      Config* config;
      
      bool loaded;
      
      DetectorMask detector_mask;

      std::string get_detector_file();
      
      float computeScaleFactor(int width, int height);
      std::vector<PlateRegion> aggregateRegions(std::vector<cv::Rect> regions);



  };

}

#endif // OPENALPR_REGIONDETECTOR_H
