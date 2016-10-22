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

#include "detector.h"

using namespace cv;
using namespace std;

namespace alpr
{

  Detector::Detector(Config* config, PreWarp* prewarp) : detector_mask(config, prewarp)
  {
    this->config = config;

    // Load the mask specified in the config if it exists
    if (config->detection_mask_image.length() > 0 && fileExists(config->detection_mask_image.c_str()))
    {
      // Load the image mask
      if (config->debugDetector)
        cout << "Loading detector mask: " << config->detection_mask_image << endl;
      Mat newMask = cv::imread(config->detection_mask_image);

      setMask(newMask);
    }
  }

  Detector::~Detector()
  {

  }
  
  void Detector::setMask(cv::Mat mask) {
    detector_mask.setMask(mask);
  }

  bool Detector::isLoaded()
  {
    return this->loaded;
  }

  vector<PlateRegion> Detector::detect(cv::Mat frame)
  {
    std::vector<cv::Rect> regionsOfInterest;
    regionsOfInterest.push_back(Rect(0, 0, frame.cols, frame.rows));
    return this->detect(frame, regionsOfInterest);
  }

  vector<PlateRegion> Detector::detect(Mat frame, std::vector<cv::Rect> regionsOfInterest)
  {

    Mat frame_gray;
    
    if (frame.channels() > 2)
    {
      cvtColor( frame, frame_gray, CV_BGR2GRAY );
    }
    else
    {
      frame.copyTo(frame_gray);
    }

    // Apply the detection mask if it has been specified by the user
    if (detector_mask.mask_loaded)
      frame_gray = detector_mask.apply_mask(frame_gray);

    // Setup debug mask image
    Mat mask_debug_img;
    if (detector_mask.mask_loaded && config->debugDetector)
    {
      frame_gray.copyTo(mask_debug_img);
      cvtColor(frame_gray, mask_debug_img, CV_GRAY2BGR);
    }
    
    vector<PlateRegion> detectedRegions;   
    for (int i = 0; i < regionsOfInterest.size(); i++)
    {
      Rect roi = regionsOfInterest[i];
      
      // Adjust the ROI to be inside the detection mask (if it exists)
      if (detector_mask.mask_loaded)
        roi = detector_mask.getRoiInsideMask(roi);

      // Draw ROIs on debug mask image
      if (detector_mask.mask_loaded && config->debugDetector)
        rectangle(mask_debug_img, roi, Scalar(0,255,255), 3);
      
      // Sanity check.  If roi width or height is less than minimum possible plate size,
      // then skip it
      if ((roi.width < config->minPlateSizeWidthPx) || 
          (roi.height < config->minPlateSizeHeightPx))
        continue;
      
      Mat cropped = frame_gray(roi);

      int w = cropped.size().width;
      int h = cropped.size().height;
      int offset_x = roi.x;
      int offset_y = roi.y;
      float scale_factor = computeScaleFactor(w, h);

      if (scale_factor != 1.0)
        resize(cropped, cropped, Size(w * scale_factor, h * scale_factor));

    
      float maxWidth = ((float) w) * (config->maxPlateWidthPercent / 100.0f) * scale_factor;
      float maxHeight = ((float) h) * (config->maxPlateHeightPercent / 100.0f) * scale_factor;
      Size minPlateSize(config->minPlateSizeWidthPx, config->minPlateSizeHeightPx);
      Size maxPlateSize(maxWidth, maxHeight);
    
      vector<Rect> allRegions = find_plates(cropped, minPlateSize, maxPlateSize);
      
      // Aggregate the Rect regions into a hierarchical representation
      for( unsigned int i = 0; i < allRegions.size(); i++ )
      {
        allRegions[i].x = (allRegions[i].x / scale_factor);
        allRegions[i].y = (allRegions[i].y / scale_factor);
        allRegions[i].width = allRegions[i].width / scale_factor;
        allRegions[i].height = allRegions[i].height / scale_factor;

        // Ensure that the rectangle isn't < 0 or > maxWidth/Height
        allRegions[i] = expandRect(allRegions[i], 0, 0, w, h);

        allRegions[i].x = allRegions[i].x + offset_x;
        allRegions[i].y = allRegions[i].y + offset_y;
      }
      
      // Check the rectangles and make sure that they're definitely not masked
      vector<Rect> regions_not_masked;
      for (unsigned int i = 0; i < allRegions.size(); i++)
      {
        if (detector_mask.mask_loaded)
        {
          if (!detector_mask.region_is_masked(allRegions[i]))
            regions_not_masked.push_back(allRegions[i]);
        }
        else
          regions_not_masked.push_back(allRegions[i]);
      }
      
      vector<PlateRegion> orderedRegions = aggregateRegions(regions_not_masked);

      

      for (unsigned int j = 0; j < orderedRegions.size(); j++)
        detectedRegions.push_back(orderedRegions[j]);
    }

    // Show debug mask image
    if (detector_mask.mask_loaded && config->debugDetector && config->debugShowImages)
    {
      imshow("Detection Mask", mask_debug_img);
    }
    
    return detectedRegions;
  }
  
  std::string Detector::get_detector_file() {
    if (config->detectorFile.length() == 0)
      return config->getCascadeRuntimeDir() + config->country + ".xml";
    
    return config->getCascadeRuntimeDir() + config->detectorFile;
  }


  float Detector::computeScaleFactor(int width, int height) {
    
    float scale_factor = 1.0;
    
    if (width > config->maxDetectionInputWidth)
    {
      // The frame is too wide
      scale_factor = ((float) config->maxDetectionInputWidth) / ((float) width);

      if (config->debugDetector)
        std::cout << "Input detection image is too wide.  Resizing with scale: " << scale_factor << endl;
    }
    else if (height > config->maxDetectionInputHeight)
    {
      // The frame is too tall
      scale_factor = ((float) config->maxDetectionInputHeight) / ((float) height);

      if (config->debugDetector)
        std::cout << "Input detection image is too tall.  Resizing with scale: " << scale_factor << endl;
    }
    
    return scale_factor;
    
  }

  bool rectHasLargerArea(cv::Rect a, cv::Rect b) { return a.area() < b.area(); };

  vector<PlateRegion> Detector::aggregateRegions(vector<Rect> regions)
  {
    // Combines overlapping regions into a parent->child order.
    // The largest regions will be parents, and they will have children if they are within them.
    // This way, when processing regions later, we can process the parents first, and only delve into the children
    // If there was no plate match.  Otherwise, we would process everything and that would be wasteful.

    vector<PlateRegion> orderedRegions;
    vector<PlateRegion> topLevelRegions;

    // Sort the list of rect regions smallest to largest
    std::sort(regions.begin(), regions.end(), rectHasLargerArea);

    // Create new PlateRegions and attach the rectangles to each
    for (unsigned int i = 0; i < regions.size(); i++)
    {
      PlateRegion newRegion;
      newRegion.rect = regions[i];
      orderedRegions.push_back(newRegion);
    }

    for (unsigned int i = 0; i < orderedRegions.size(); i++)
    {
      bool foundParent = false;
      for (unsigned int k = i + 1; k < orderedRegions.size(); k++)
      {
        Point center( orderedRegions[i].rect.x + (orderedRegions[i].rect.width / 2),
                      orderedRegions[i].rect.y + (orderedRegions[i].rect.height / 2));

        // Check if the center of the smaller rectangle is inside the bigger rectangle.
        // If so, add it to the children and continue on.
        if (orderedRegions[k].rect.contains(center))
        {
          orderedRegions[k].children.push_back(orderedRegions[i]);
          foundParent = true;
          break;
        }

      }

      if (foundParent == false)
      {
        // We didn't find any parents for this rectangle.  Add it to the top level regions
        topLevelRegions.push_back(orderedRegions[i]);
      }

    }


    return topLevelRegions;
  }

}