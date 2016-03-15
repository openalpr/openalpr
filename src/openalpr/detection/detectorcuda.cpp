/*
 * Copyright (c) 2013 OpenALPR Technology, Inc.
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

#include "detectorcuda.h"

#ifdef COMPILE_GPU

using namespace cv;
using namespace std;


namespace alpr
{

  DetectorCUDA::DetectorCUDA(Config* config, PreWarp* prewarp) : Detector(config, prewarp) {



    if( this->cuda_cascade.load( get_detector_file() ) )
    {
      this->loaded = true;
      printf("--(!)Loaded CUDA classifier\n");
    }
    else
    {
      this->loaded = false;
      printf("--(!)Error loading CPU classifier %s\n", get_detector_file().c_str());
    }
  }


  DetectorCUDA::~DetectorCUDA() {
  }

  vector<Rect> DetectorCUDA::find_plates(Mat frame, cv::Size min_plate_size, cv::Size max_plate_size)
  {
    //-- Detect plates
    vector<Rect> plates;
    
    timespec startTime;
    getTimeMonotonic(&startTime);

    gpu::GpuMat cudaFrame, plateregions_buffer;
    Mat plateregions_downloaded;

    cudaFrame.upload(frame);
    int numdetected = cuda_cascade.detectMultiScale(cudaFrame, plateregions_buffer, 
            (double) config->detection_iteration_increase, config->detectionStrictness, 
            min_plate_size);
    
    plateregions_buffer.colRange(0, numdetected).download(plateregions_downloaded);

    for (int i = 0; i < numdetected; ++i)
    {
      plates.push_back(plateregions_downloaded.ptr<cv::Rect>()[i]);
    }

    if (config->debugTiming)
    {
      timespec endTime;
      getTimeMonotonic(&endTime);
      cout << "LBP Time: " << diffclock(startTime, endTime) << "ms." << endl;
    }
    
    return plates;
  }

}

#endif