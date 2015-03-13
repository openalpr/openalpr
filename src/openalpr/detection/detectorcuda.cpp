/*
 * Copyright (c) 2013 New Designs Unlimited, LLC
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

#include "detectorcuda.h"


using namespace cv;
using namespace std;

#ifdef COMPILE_GPU

namespace alpr
{

  DetectorCUDA::DetectorCUDA(Config* config) : Detector(config) {



    if( this->cuda_cascade.load( config->getCascadeRuntimeDir() + config->country + ".xml" ) )
    {
      this->loaded = true;
      printf("--(!)Loaded CUDA classifier\n");
    }
    else
    {
      this->loaded = false;
      printf("--(!)Error loading CUDA classifier\n");
    }
  }


  DetectorCUDA::~DetectorCUDA() {
  }

  vector<PlateRegion> DetectorCUDA::detect(Mat frame, std::vector<cv::Rect> regionsOfInterest)
  {

    Mat frame_gray;
    cvtColor( frame, frame_gray, CV_BGR2GRAY );

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

  vector<PlateRegion> DetectorCUDA::doCascade(Mat frame, int offset_x, int offset_y)
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

    gpu::GpuMat cudaFrame, plateregions_buffer;
    Mat plateregions_downloaded;

    cudaFrame.upload(frame);
    int numdetected = cuda_cascade.detectMultiScale(cudaFrame, plateregions_buffer, (double) config->detection_iteration_increase, config->detectionStrictness, minSize);
    plateregions_buffer.colRange(0, numdetected).download(plateregions_downloaded);

    for (int i = 0; i < numdetected; ++i)
    {
      plates.push_back(plateregions_downloaded.ptr<cv::Rect>()[i]);
    }



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

#endif