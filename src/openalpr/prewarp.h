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


#ifndef OPENALPR_PREWARP_H
#define	OPENALPR_PREWARP_H

#include "config.h"
#include "utility.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "detection/detector.h"

namespace alpr
{

  class PreWarp {
  public:
    PreWarp(Config* config);
    virtual ~PreWarp();

    cv::Mat warpImage(cv::Mat image);
    std::vector<cv::Point2f> projectPoints(std::vector<cv::Point2f> points, bool inverse);
    std::vector<cv::Rect> projectRects(std::vector<cv::Rect> rects, int maxWidth, int maxHeight, bool inverse);
    void projectPlateRegions(std::vector<PlateRegion>& plateRegions, int maxWidth, int maxHeight, bool inverse);

    bool valid;
    
  private:
    Config* config;
    cv::Mat transform;
    
    float w, h, rotationx, rotationy, rotationz, stretchX, dist, panX, panY;
    
    cv::Mat findTransform(float w, float h, float rotationx, float rotationy, float rotationz, float panX, float panY, float stretchX, float dist);
  };

}

#endif	/* OPENALPR_PREWARP_H */

