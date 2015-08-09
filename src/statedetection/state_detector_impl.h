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

#ifndef SRC_STATE_DETECTOR_IMPL_H
#define SRC_STATE_DETECTOR_IMPL_H

#include "state_detector.h"
#include "featurematcher.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace alpr {

  class StateDetectorImpl {
    public:
      StateDetectorImpl(const std::string country, const std::string runtimeDir);
      virtual ~StateDetectorImpl();

      bool isLoaded();

      // Maximum number of candidates to return
      void setTopN(int topN);

      std::vector<StateCandidate> detect(std::vector<char> imageBytes);
      std::vector<StateCandidate> detect(unsigned char* pixelData, int bytesPerPixel, int imgWidth, int imgHeight);
      std::vector<StateCandidate> detect(cv::Mat image);

      FeatureMatcher featureMatcher;
  };

}

#endif //SRC_STATE_DETECTOR_IMPL_H
