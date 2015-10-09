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

#include "detectorcpu.h"

using namespace cv;
using namespace std;

namespace alpr
{


  DetectorCPU::DetectorCPU(Config* config) : Detector(config) {



    if( this->plate_cascade.load( get_detector_file() ) )
    {
      this->loaded = true;
    }
    else
    {
      this->loaded = false;
      printf("--(!)Error loading CPU classifier %s\n", get_detector_file().c_str());
    }
  }


  DetectorCPU::~DetectorCPU() {
  }

  vector<PlateRegion> DetectorCPU::detect(Mat frame, std::vector<cv::Rect> regionsOfInterest)
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


    vector<PlateRegion> detectedRegions;   
    for (int i = 0; i < regionsOfInterest.size(); i++)
    {
      // Sanity check.  If roi width or height is less than minimum possible plate size,
      // then skip it
      if ((regionsOfInterest[i].width < config->minPlateSizeWidthPx) || 
          (regionsOfInterest[i].height < config->minPlateSizeHeightPx))
        continue;
      
      Mat cropped = frame_gray(regionsOfInterest[i]);
      vector<PlateRegion> subRegions = doCascade(cropped, regionsOfInterest[i].x, regionsOfInterest[i].y);

      for (int j = 0; j < subRegions.size(); j++)
        detectedRegions.push_back(subRegions[j]);
    }
    return detectedRegions;
  }

  vector<PlateRegion> DetectorCPU::doCascade(Mat frame, int offset_x, int offset_y)
  {

    int w = frame.size().width;
    int h = frame.size().height;

    float scale_factor = computeScaleFactor(w, h);
    
    vector<Rect> plates;

    equalizeHist( frame, frame );
    
    if (scale_factor != 1.0)
      resize(frame, frame, Size(w * scale_factor, h * scale_factor));

    //-- Detect plates
    timespec startTime;
    getTimeMonotonic(&startTime);

    float maxWidth = ((float) w) * (config->maxPlateWidthPercent / 100.0f) * scale_factor;
    float maxHeight = ((float) h) * (config->maxPlateHeightPercent / 100.0f) * scale_factor;
    Size minSize(config->minPlateSizeWidthPx * scale_factor, config->minPlateSizeHeightPx * scale_factor);
    Size maxSize(maxWidth, maxHeight);

    plate_cascade.detectMultiScale( frame, plates, config->detection_iteration_increase, config->detectionStrictness,
                                      0,
                                      //0|CV_HAAR_SCALE_IMAGE,
                                      minSize, maxSize );


    if (config->debugTiming)
    {
      timespec endTime;
      getTimeMonotonic(&endTime);
      cout << "LBP Time: " << diffclock(startTime, endTime) << "ms." << endl;
    }

    for( unsigned int i = 0; i < plates.size(); i++ )
    {
      plates[i].x = (plates[i].x / scale_factor);
      plates[i].y = (plates[i].y / scale_factor);
      plates[i].width = plates[i].width / scale_factor;
      plates[i].height = plates[i].height / scale_factor;
      
      // Ensure that the rectangle isn't < 0 or > maxWidth/Height
      plates[i] = expandRect(plates[i], 0, 0, w, h);
      
      plates[i].x = plates[i].x + offset_x;
      plates[i].y = plates[i].y + offset_y;
    }

    vector<PlateRegion> orderedRegions = aggregateRegions(plates);

    return orderedRegions;

  }

}
