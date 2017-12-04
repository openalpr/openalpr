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

#ifndef OPENALPR_DETECTORMASK_H
#define	OPENALPR_DETECTORMASK_H

#include <string>
#include "opencv2/imgproc/imgproc.hpp"
#include "config.h"
#include "prewarp.h"

namespace alpr
{

  class DetectorMask {
  public:
    
    DetectorMask(Config* config, PreWarp* prewarp);
    virtual ~DetectorMask();

    void setMask(cv::Mat mask);
    
    cv::Rect getRoiInsideMask(cv::Rect roi);
    
    cv::Size mask_size();
    
    bool region_is_masked(cv::Rect region);
    
    cv::Mat apply_mask(cv::Mat image);
    
    bool mask_loaded;
    
  private:

    void resize_mask(cv::Mat image);
    
    PreWarp* prewarp;
    std::string last_prewarp_hash;
    
    cv::Mat mask;
    
    cv::Mat resized_mask;
    bool resized_mask_loaded;
    
    Config* config;
    cv::Rect scan_area;
   
  };

}
#endif	/* DETECTORMASK_H */

