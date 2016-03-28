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
    if (orig_mask.cols <= 0 || orig_mask.rows <= 0)
    {
      resized_mask_loaded = false;
      mask_loaded = false;
      return;
    }
      
    this->mask = orig_mask;
    if (orig_mask.channels() > 2)
      cvtColor( orig_mask, this->mask, CV_BGR2GRAY );
    else
      this->mask = orig_mask;
    
    resized_mask_loaded = false;
    mask_loaded = true;
  }
  
  cv::Size DetectorMask::mask_size() {
    if (resized_mask_loaded)
      return resized_mask.size();
    return mask.size();
  }

  // Provided a region of interest, truncate it if the mask cuts off a portion of it.
  // No reason to analyze extra content
  cv::Rect DetectorMask::getRoiInsideMask(cv::Rect roi) {
      Rect roi_intersection = roi & scan_area;
      return roi_intersection;  
  }

  
  // Checks if the provided region is partially covered by the mask
  // If so, it is disqualified
  bool DetectorMask::region_is_masked(cv::Rect region) {
    int MIN_WHITENESS = 248;
    
    // If the mean pixel value over the crop is very white (e.g., > 253 out of 255)
    // then this is in the white area of the mask and we'll use it
    
    // Make sure the region doesn't extend beyond the bounds of our image
    expandRect(region, 0, 0, resized_mask.cols, resized_mask.rows);
    Mat mask_crop = resized_mask(region);
    double mean_value = mean(mask_crop)[0];
    
    if (config->debugDetector)
    {
      cout << "region_is_masked: Mean whiteness: " << mean_value << endl;
    }
    return mean_value < MIN_WHITENESS;
  }
  
  void DetectorMask::resize_mask(cv::Mat image) {
    
    resize(mask, resized_mask, image.size());

    if (prewarp->valid) 
    {
      resized_mask = prewarp->warpImage(resized_mask);
    }

    // Threshold the mask so that the values are either 0 or 255 (no shades of gray))
    // This can happen with jpeg compression
    threshold(this->mask, this->mask, 55, 255, cv::THRESH_BINARY);
     
    // Calculate the biggest rectangle that covers all the whitespace
    // Rather than using contours, go row by row, column by column until you hit
    // a white pixel and stop.  Should be faster and a little simpler
    int top_bound = 0, bottom_bound = 0, left_bound = 0, right_bound = 0;
    
    for (top_bound = 0; top_bound < resized_mask.rows; top_bound++)
      if (countNonZero(resized_mask.row(top_bound)) > 0) break;
    for (bottom_bound = resized_mask.rows - 1; bottom_bound >= 0; bottom_bound--)
      if (countNonZero(resized_mask.row(bottom_bound)) > 0) break;
    
    for (left_bound = 0; left_bound < resized_mask.cols; left_bound++)
      if (countNonZero(resized_mask.col(left_bound)) > 0) break;
    for (right_bound = resized_mask.cols - 1; right_bound >= 0; right_bound--)
      if (countNonZero(resized_mask.col(right_bound)) > 0) break;
    
    if (left_bound >= right_bound || top_bound >= bottom_bound)
    {
      // Invalid mask, set it to 0 width/height
      scan_area.x = 0;
      scan_area.y = 0;
      scan_area.width = 0;
      scan_area.height = 0;
    }
    else
    {
      scan_area.x = left_bound;
      scan_area.y = top_bound;
      scan_area.width = right_bound - left_bound;
      scan_area.height = bottom_bound - top_bound;
    }
      
    //cout << scan_area << endl;
  }

  Mat DetectorMask::apply_mask(Mat image) {
    if (!mask_loaded)
      return image;
       
    if (!resized_mask_loaded || image.size() != resized_mask.size() || 
            last_prewarp_hash != prewarp->toString())
    {
      resize_mask(image);
      
      last_prewarp_hash = prewarp->toString();
      
      resized_mask_loaded = true;
    }
    
    if (image.size() != resized_mask.size() && config->debugDetector)
    {
      cout << "Mask does not match image size" << endl;
      return image;
    }
    
    Mat response = Mat::zeros(image.size(), image.type());
    bitwise_and(image, resized_mask, response);
    
    return response;
  }

}