/*
 * Copyright (c) 2015 New Designs Unlimited, LLC
 * Opensource Automated License Plate Recognition [http://www.openalpr.com]
 *
 * This file is part of OpenAlpr.
 *
 * OpenAlpr is free software: you can redistribute it and/or modify
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



    if( this->plate_cascade.load( config->getCascadeRuntimeDir() + config->country + ".xml" ) )
    {
      this->loaded = true;
    }
    else
    {
      this->loaded = false;
      printf("--(!)Error loading CPU classifier\n");
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
      frame_gray = frame;
    }


    vector<PlateRegion> detectedRegions;
    for (int i = 0; i < regionsOfInterest.size(); i++)
    {
      Mat cropped = frame_gray(regionsOfInterest[i]);
      vector<PlateRegion> subRegions = doCascade(cropped, regionsOfInterest[i].x, regionsOfInterest[i].y);

      for (int j = 0; j < subRegions.size(); j++)
        detectedRegions.push_back(subRegions[j]);
    }
    return detectedRegions;
  }

  vector<PlateRegion> DetectorCPU::doCascade(Mat frame, int offset_x, int offset_y)
  {


    if (frame.cols > config->maxDetectionInputWidth)
    {
      // The frame is too wide
      this->scale_factor = ((float) config->maxDetectionInputWidth) / ((float) frame.cols);

      if (config->debugGeneral)
        std::cout << "Input detection image is too wide.  Resizing with scale: " << this->scale_factor << endl;
    }
    else if (frame.rows > config->maxDetectionInputHeight)
    {
      // The frame is too tall
      this->scale_factor = ((float) config->maxDetectionInputHeight) / ((float) frame.rows);

      if (config->debugGeneral)
        std::cout << "Input detection image is too tall.  Resizing with scale: " << this->scale_factor << endl;
    }

    int w = frame.size().width;
    int h = frame.size().height;

    vector<Rect> plates;

    equalizeHist( frame, frame );
    resize(frame, frame, Size(w * this->scale_factor, h * this->scale_factor));

    //-- Detect plates
    timespec startTime;
    getTime(&startTime);

    float maxWidth = ((float) w) * (config->maxPlateWidthPercent / 100.0f) * this->scale_factor;
    float maxHeight = ((float) h) * (config->maxPlateHeightPercent / 100.0f) * this->scale_factor;
    Size minSize(config->minPlateSizeWidthPx * this->scale_factor, config->minPlateSizeHeightPx * this->scale_factor);
    Size maxSize(maxWidth, maxHeight);

    plate_cascade.detectMultiScale( frame, plates, config->detection_iteration_increase, config->detectionStrictness,
                                      0,
                                      //0|CV_HAAR_SCALE_IMAGE,
                                      minSize, maxSize );


    if (config->debugTiming)
    {
      timespec endTime;
      getTime(&endTime);
      cout << "LBP Time: " << diffclock(startTime, endTime) << "ms." << endl;
    }

    for( unsigned int i = 0; i < plates.size(); i++ )
    {
      plates[i].x = (plates[i].x / scale_factor) + offset_x;
      plates[i].y = (plates[i].y / scale_factor) + offset_y;
      plates[i].width = plates[i].width / scale_factor;
      plates[i].height = plates[i].height / scale_factor;
    }

    vector<PlateRegion> orderedRegions = aggregateRegions(plates);

    return orderedRegions;

  }

}
