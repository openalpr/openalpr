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

#ifndef OPENALPR_LICENSEPLATECANDIDATE_H
#define OPENALPR_LICENSEPLATECANDIDATE_H

#include <vector>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"

#include "utility.h"
#include "constants.h"
#include "edges/platelines.h"
#include "transformation.h"
#include "textdetection/characteranalysis.h"
#include "edges/platecorners.h"
#include "config.h"
#include "pipeline_data.h"

namespace alpr
{

  class LicensePlateCandidate
  {

    public:
      LicensePlateCandidate(PipelineData* pipeline_data);
      virtual ~LicensePlateCandidate();


      void recognize();


    private:
      PipelineData* pipeline_data;
      Config* config;


      cv::Mat filterByCharacterHue(std::vector<std::vector<cv::Point> > charRegionContours);
      std::vector<cv::Point> findPlateCorners(cv::Mat inputImage, PlateLines plateLines, CharacterAnalysis textAnalysis);	// top-left, top-right, bottom-right, bottom-left

      cv::Size getCropSize(std::vector<cv::Point2f> areaCorners);

  };
  
}
#endif // OPENALPR_LICENSEPLATECANDIDATE_H
