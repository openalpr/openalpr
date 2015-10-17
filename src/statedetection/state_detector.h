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

#ifndef OPENALPR_STATE_DETECTOR_H
#define OPENALPR_STATE_DETECTOR_H

#include <string>
#include <vector>

namespace alpr {

  struct StateCandidate
  {
    std::string state_code;
    float confidence;
  };

  class StateDetectorImpl;
  class StateDetector {

    public:
      StateDetector(const std::string country, const std::string configFile, const std::string runtimeDir);
      virtual ~StateDetector();

      bool isLoaded();

      // Maximum number of candidates to return
      void setTopN(int topN);

      // Given an image of a license plate, provide the likely state candidates
      std::vector<StateCandidate> detect(std::vector<char> imageBytes);
      std::vector<StateCandidate> detect(unsigned char* pixelData, int bytesPerPixel, int imgWidth, int imgHeight);

      StateDetectorImpl* impl;
  };

}

#endif //OPENALPR_STATE_DETECTOR_H
