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

#ifndef OPENALPR_DETECTORCUDA_H
#define	OPENALPR_DETECTORCUDA_H


#include <stdio.h>
#include <iostream>
#include <vector>

#ifdef COMPILE_GPU

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/gpu/gpu.hpp"

#include "detector.h"
#include "detectorcpu.h"



namespace alpr
{

  class DetectorCUDA : public Detector {
  public:
      DetectorCUDA(Config* config);
      virtual ~DetectorCUDA();

      std::vector<PlateRegion> detect(cv::Mat frame, std::vector<cv::Rect> regionsOfInterest);

  private:

      cv::gpu::CascadeClassifier_GPU cuda_cascade;

      std::vector<PlateRegion> doCascade(cv::Mat frame, int offset_x, int offset_y);
  };

}

#endif 

#endif	/* OPENALPR_DETECTORCUDA_H */

