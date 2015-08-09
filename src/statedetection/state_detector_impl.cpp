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

#include "state_detector_impl.h"

namespace alpr
{
  StateDetectorImpl::StateDetectorImpl(const std::string country, const std::string runtimeDir)
  {


    if (featureMatcher.isLoaded() == false)
    {
      std::cout << "Can not create detector or descriptor extractor or descriptor matcher of given types" << std::endl;
      return;
    }

    featureMatcher.loadRecognitionSet(runtimeDir, country);
  }

  StateDetectorImpl::~StateDetectorImpl() { }

  bool StateDetectorImpl::isLoaded() {
    return false;
  }

  void StateDetectorImpl::setTopN(int topN) {
  }

  std::vector<StateCandidate> StateDetectorImpl::detect(std::vector<char> imageBytes) {
    cv::Mat img = cv::imdecode(cv::Mat(imageBytes), 1);

    return this->detect(img);
  }

  std::vector<StateCandidate> StateDetectorImpl::detect(unsigned char *pixelData, int bytesPerPixel, int imgWidth,
                                                        int imgHeight) {
    int arraySize = imgWidth * imgHeight * bytesPerPixel;
    cv::Mat imgData = cv::Mat(arraySize, 1, CV_8U, pixelData);
    cv::Mat img = imgData.reshape(bytesPerPixel, imgHeight);

    return this->detect(img);

  }

  std::vector<StateCandidate> StateDetectorImpl::detect(cv::Mat image) {
    std::vector<StateCandidate> results;


    cv::Mat debugImg(image.size(), image.type());
    image.copyTo(debugImg);
    std::vector<int> matchesArray(featureMatcher.numTrainingElements());

    RecognitionResult result = featureMatcher.recognize(image, true, &debugImg, true, matchesArray );

    if (result.haswinner == false)
      return results;

    StateCandidate top_candidate;
    top_candidate.confidence = result.confidence;
    top_candidate.state_code = result.winner;

    results.push_back(top_candidate);

    return results;
  }
}