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

#include "detectormask.h"
#include "prewarp.h"

using namespace cv;
using namespace std;
  
namespace alpr
{
  

  DetectorMask::DetectorMask(Config* config, PreWarp* prewarp) {
    mask_loaded = false;
    resized_mask_loaded = false;
    this->config = config;
    this->prewarp = prewarp;
    last_prewarp_hash = "";
  }

  DetectorMask::~DetectorMask() {
  }

  void DetectorMask::setMask(Mat orig_mask) {
    this->mask = orig_mask;
    if (orig_mask.channels() > 2)
      cvtColor( orig_mask, this->mask, CV_BGR2GRAY );
    else
      this->mask = orig_mask;
    
    // Threshold the mask so that the values are either 0 or 255 (no shades of gray))
    threshold(this->mask, this->mask, 1, 255, cv::THRESH_BINARY);
    
    
    // Calculate the biggest rectangle that covers all the whitespace
    // Rather than using contours, go row by row, column by column until you hit
    // a white pixel and stop.  Should be faster and a little simpler
    unsigned int top_bound = 0, bottom_bound = 0, left_bound = 0, right_bound = 0;
    
    cout << mask.col(20).cols << " - " << mask.col(20).rows << endl;
    for (top_bound = 0; top_bound < mask.rows; top_bound++)
      if (countNonZero(mask.row(top_bound)) > 0) break;
    for (bottom_bound = mask.rows - 1; bottom_bound >= 0; bottom_bound--)
      if (countNonZero(mask.row(bottom_bound)) > 0) break;
    
    for (left_bound = 0; left_bound < mask.cols; left_bound++)
      if (countNonZero(mask.col(left_bound)) > 0) break;
    for (right_bound = mask.rows - 1; right_bound >= 0; right_bound--)
      if (countNonZero(mask.col(right_bound)) > 0) break;
    
    if (left_bound >= right_bound || top_bound >= bottom_bound)
    {
      cerr << "Invalid mask" << endl;
      return;
    }
    
    scan_area.x = left_bound;
    scan_area.y = top_bound;
    scan_area.width = right_bound - left_bound;
    scan_area.height = bottom_bound - top_bound;
    
//    Mat debug(this->mask.size(), mask.type());
//    this->mask.copyTo(debug);
//    cvtColor(debug, debug, CV_GRAY2BGR);
//    rectangle(debug, scan_area, Scalar(0,255,0), 2);
//    drawAndWait(debug);
    mask_loaded = true;
  }
  
  cv::Size DetectorMask::mask_size() {
    return mask.size();
  }

  // Provided a region of interest, truncate it if the mask cuts off a portion of it.
  // No reason to analyze extra content
  cv::Rect DetectorMask::getRoiInsideMask(cv::Rect roi) {
    
    if (prewarp->valid)
    {
      cv::Rect warped_scan_area = prewarp->projectRect(scan_area, mask.cols, mask.rows, false);

      Rect roi_intersection = roi & scan_area;
      return roi_intersection;
      
    }
    else
    {
      Rect roi_intersection = roi & scan_area;
      return roi_intersection;
    }
  }

  
  // Checks if the provided region is partially covered by the mask
  // If so, it is disqualified
  bool DetectorMask::region_is_masked(cv::Rect region) {
    int MIN_WHITENESS = 253;
    
    // If the mean pixel value over the crop is very white (e.g., > 253 out of 255)
    // then this is in the white area of the mask and we'll use it
    Mat mask_crop = mask(region);
    double mean_value = mean(mask_crop)[0];
    
    return mean_value >= MIN_WHITENESS;
  }

  Mat DetectorMask::apply_mask(Mat image) {
    if (!mask_loaded)
      return image;
    
    if (!resized_mask_loaded || last_prewarp_hash != prewarp->toString())
    {
      resize(mask, resized_mask, image.size());
      
      if (prewarp->valid)
      {
        resized_mask = prewarp->warpImage(resized_mask);
      }
      
      last_prewarp_hash = prewarp->toString();
      
      resized_mask_loaded = true;
    }
    
    if (image.size() != resized_mask.size())
    {
      return image;
    }
    
    Mat response(image.size(), image.type());
    image.copyTo(response, resized_mask);
    
    return response;
  }

}