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

#ifndef OPENALPR_EDGEFINDER_H
#define	OPENALPR_EDGEFINDER_H

#include "opencv2/imgproc/imgproc.hpp"
#include "pipeline_data.h"
#include "transformation.h"
#include "platelines.h"
#include "platecorners.h"

namespace alpr
{
  
  class EdgeFinder {
  public:
    EdgeFinder(PipelineData* pipeline_data);
    virtual ~EdgeFinder();

    std::vector<cv::Point2f> findEdgeCorners();


  private:
    PipelineData* pipeline_data;

    std::vector<cv::Point2f> detection(bool high_contrast);
    
    std::vector<cv::Point> highContrastDetection(cv::Mat newCrop, std::vector<TextLine> newLines);
    std::vector<cv::Point> normalDetection(cv::Mat newCrop, std::vector<TextLine> newLines);
    
    
    bool is_high_contrast(const cv::Mat crop);
  };

}
#endif	/* OPENALPR_EDGEFINDER_H */

