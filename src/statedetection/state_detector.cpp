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

#include "state_detector.h"
#include "state_detector_impl.h"

using namespace std;

namespace alpr {

  StateDetector::StateDetector(const std::string country, const std::string configFile, const std::string runtimeDir)  {
    impl = new StateDetectorImpl(country, runtimeDir);
  }

  StateDetector::~StateDetector() {
    delete impl;
  }

  bool StateDetector::isLoaded() {
    return impl->isLoaded();
  }

  void StateDetector::setTopN(int topN) {
    impl->setTopN(topN);
  }

  vector<StateCandidate> StateDetector::detect(vector<char> imageBytes) {
    return impl->detect(imageBytes);
  }

  vector<StateCandidate> StateDetector::detect(unsigned char *pixelData, int bytesPerPixel, int imgWidth,
                                                    int imgHeight) {
    return impl->detect(pixelData, bytesPerPixel, imgWidth, imgHeight);
  }

}
