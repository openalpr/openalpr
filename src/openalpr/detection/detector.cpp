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

  Detector::Detector(Config* config)
  {
    this->config = config;

  }

  Detector::~Detector()
  {

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
    // Must be implemented by subclass
    std::vector<PlateRegion> rois;
    return rois;
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