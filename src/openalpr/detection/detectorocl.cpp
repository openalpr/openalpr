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


  DetectorOCL::DetectorOCL(Config* config, PreWarp* prewarp) : Detector(config, prewarp) {

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


  vector<Rect> DetectorOCL::find_plates(Mat orig_frame, cv::Size min_plate_size, cv::Size max_plate_size)
  {


    vector<Rect> plates;

    //-- Detect plates
    timespec startTime;
    getTimeMonotonic(&startTime);

    // If we have an OpenCL core available, use it.  Otherwise use CPU
    if (ocl_detector_mutex_m.try_lock())
    {
      UMat openclFrame;
      orig_frame.copyTo(openclFrame);

      equalizeHist( openclFrame, openclFrame );

      plate_cascade.detectMultiScale( openclFrame, plates, config->detection_iteration_increase, config->detectionStrictness,
                                      CV_HAAR_DO_CANNY_PRUNING,
                                      min_plate_size, max_plate_size );

      ocl_detector_mutex_m.unlock();
    }
    else
    {
      equalizeHist( orig_frame, orig_frame );

      plate_cascade.detectMultiScale( orig_frame, plates, config->detection_iteration_increase, config->detectionStrictness,
                                      CV_HAAR_DO_CANNY_PRUNING,
                                      min_plate_size, max_plate_size );
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