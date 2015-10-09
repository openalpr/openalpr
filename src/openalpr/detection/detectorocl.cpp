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

#include "detectorocl.h"
#include <support/tinythread.h>

#if OPENCV_MAJOR_VERSION == 3

using namespace cv;
using namespace std;

tthread::mutex ocl_detector_mutex_m;


namespace alpr
{


  DetectorOCL::DetectorOCL(Config* config) : Detector(config) {

    tthread::lock_guard<tthread::mutex> guard(ocl_detector_mutex_m);

    cv::ocl::setUseOpenCL(true);

	if (config->debugDetector)
	{
		try{
			cout << "\r\nUse of OpenCL LBP detector selected in config file." << endl;
			cv::ocl::Device ocldevice;

			std::vector<cv::ocl::PlatformInfo> platforms;
			getPlatfomsInfo(platforms);
			if (platforms.size()>0) cout << "OpenCL device(s) found:" << endl;
			int n = 0;
			for (size_t i = 0; i < platforms.size(); i++)
			{
				const cv::ocl::PlatformInfo* platform = &platforms[i];
				for (int j = 0; j < platform->deviceNumber(); j++)
				{
					platform->getDevice(ocldevice, j);
					cout << n << " " << ocldevice.name() << " (" << ocldevice.version() << ")" << endl;
					n++;
				}
			}
			if (n > 1)
			{
				ocldevice = cv::ocl::Device::getDefault();
				if (ocldevice.available())
				{
					cout << "\r\nCurrent OpenCL device: \r\n  " << ocldevice.name() << " (" << ocldevice.version() << ").\r\n" << endl;
				}
				else
				{
					cout << "\r\nOpenCL error: The selected device is not available.\r\n" << endl;
				}
				cout << "Select the OpenCL device by adjusting the environment variable OPENCV_OPENCL_DEVICE, e.g.\r\n-In Windows type at the command prompt:\r\n  set OPENCV_OPENCL_DEVICE=::1\r\n" << endl;
			}
		}
		catch (...)
		{
			cout << "OpenCL error: No OpenCL device found.\r\n" << endl;
		}
	}

    if (!ocl::haveOpenCL())
    {
      this->loaded = false;
      cerr << "OpenCL not detected" << endl;
    }
	else if( this->plate_cascade.load( get_detector_file() ) )
		{
			this->loaded = true;
		}
		else
		{
			this->loaded = false;
			cerr << "--(!)Error loading cascade " << get_detector_file() << "\n" << endl;
		}
  }


  DetectorOCL::~DetectorOCL() {
  }

  vector<PlateRegion> DetectorOCL::detect(Mat frame, std::vector<cv::Rect> regionsOfInterest)
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

  vector<PlateRegion> DetectorOCL::doCascade(Mat orig_frame, int offset_x, int offset_y)
  {


    int w = orig_frame.size().width;
    int h = orig_frame.size().height;

    float scale_factor = computeScaleFactor(w, h);

    vector<Rect> plates;

    //-- Detect plates
    timespec startTime;
    getTimeMonotonic(&startTime);

    float maxWidth = ((float) w) * (config->maxPlateWidthPercent / 100.0f) * scale_factor;
    float maxHeight = ((float) h) * (config->maxPlateHeightPercent / 100.0f) * scale_factor;

    Size minSize(config->minPlateSizeWidthPx * scale_factor, config->minPlateSizeHeightPx * scale_factor);
    Size maxSize(maxWidth, maxHeight);

    // If we have an OpenCL core available, use it.  Otherwise use CPU
    if (ocl_detector_mutex_m.try_lock())
    {
      UMat openclFrame;
      orig_frame.copyTo(openclFrame);

      equalizeHist( openclFrame, openclFrame );

      if (scale_factor != 1.0)
        resize(openclFrame, openclFrame, Size(w * scale_factor, h * scale_factor));

      plate_cascade.detectMultiScale( openclFrame, plates, config->detection_iteration_increase, config->detectionStrictness,
                                      0,
                                      minSize, maxSize );

      ocl_detector_mutex_m.unlock();
    }
    else
    {
      equalizeHist( orig_frame, orig_frame );

      if (scale_factor != 1.0)
        resize(orig_frame, orig_frame, Size(w * scale_factor, h * scale_factor));

      plate_cascade.detectMultiScale( orig_frame, plates, config->detection_iteration_increase, config->detectionStrictness,
                                      0,
                                      minSize, maxSize );
    }





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

#endif