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

#ifndef OPENALPR_DETECTOROPENCL_H
#define	OPENALPR_DETECTOROPENCL_H

#include <vector>

#if OPENCV_MAJOR_VERSION == 3

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/core/ocl.hpp"

#include "detector.h"

namespace alpr
{

  class DetectorOCL : public Detector {
  public:
    DetectorOCL(Config* config, PreWarp* prewarp);
    virtual ~DetectorOCL();

    std::vector<cv::Rect> find_plates(cv::Mat frame, cv::Size min_plate_size, cv::Size max_plate_size);

  private:

    cv::CascadeClassifier plate_cascade;

  };

}

#endif

#endif	/* OPENALPR_DETECTOROPENCL_H */

