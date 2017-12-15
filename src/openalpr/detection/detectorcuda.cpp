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


#ifndef OPENCV32

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

#else
    cuda_cascade = cuda::CascadeClassifier::create(get_detector_file());
    if( !this->cuda_cascade.get()->empty() )
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

#endif

  DetectorCUDA::~DetectorCUDA() {
  }

  vector<Rect> DetectorCUDA::find_plates(Mat frame, cv::Size min_plate_size, cv::Size max_plate_size)
  {
    //-- Detect plates
    vector<Rect> plates;

    timespec startTime;
    getTimeMonotonic(&startTime);
#ifndef OPENCV32

    gpu::GpuMat cudaFrame, plateregions_buffer;

#else

    cuda::GpuMat cudaFrame, plateregions_buffer;

#endif
    Mat plateregions_downloaded;

    cudaFrame.upload(frame);
#ifndef OPENCV32

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

#else

    cuda_cascade->setScaleFactor((double) config->detection_iteration_increase);
    cuda_cascade->setMaxNumObjects(config->detectionStrictness);
    cuda_cascade->setMinObjectSize(min_plate_size);
    cuda_cascade->detectMultiScale(cudaFrame, plateregions_buffer);
    cuda_cascade->convert(plateregions_buffer,plates);
    if (config->debugTiming)
    {
      timespec endTime;
      getTimeMonotonic(&endTime);
      cout << "LBP Time: " << diffclock(startTime, endTime) << "ms." << endl;
    }
#endif

    return plates;
  }

}

#endif
