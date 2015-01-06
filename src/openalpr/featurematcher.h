/*
 * Copyright (c) 2015 New Designs Unlimited, LLC
 * Opensource Automated License Plate Recognition [http://www.openalpr.com]
 *
 * This file is part of OpenAlpr.
 *
 * OpenAlpr is free software: you can redistribute it and/or modify
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

#ifndef OPENALPR_FEATUREMATCHER_H
#define OPENALPR_FEATUREMATCHER_H

#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/video/tracking.hpp"

#include "support/filesystem.h"
#include "constants.h"
#include "utility.h"
#include "config.h"

namespace alpr
{

  struct RecognitionResult
  {
    bool haswinner;
    std::string winner;
    int confidence;
  } ;

  class FeatureMatcher
  {

    public:
      FeatureMatcher(Config* config);
      virtual ~FeatureMatcher();

      RecognitionResult recognize( const cv::Mat& queryImg, bool drawOnImage, cv::Mat* outputImage,
                                   bool debug_on, std::vector<int> debug_matches_array );

      bool loadRecognitionSet(std::string country);

      bool isLoaded();

      int numTrainingElements();

    private:
      Config* config;

      cv::Ptr<cv::DescriptorMatcher> descriptorMatcher;
      cv::Ptr<cv::FastFeatureDetector> detector;
      cv::Ptr<cv::BRISK> extractor;

      std::vector<std::vector<cv::KeyPoint> > trainingImgKeypoints;

      void _surfStyleMatching(const cv::Mat& queryDescriptors, std::vector<std::vector<cv::DMatch> > matchesKnn, std::vector<cv::DMatch>& matches12);

      void crisscrossFiltering(const std::vector<cv::KeyPoint> queryKeypoints, const std::vector<cv::DMatch> inputMatches, std::vector<cv::DMatch> &outputMatches);

      std::vector<std::string> billMapping;

      void surfStyleMatching( const cv::Mat& queryDescriptors, std::vector<cv::KeyPoint> queryKeypoints,
                              std::vector<cv::DMatch>& matches12 );

  };

}
#endif // OPENALPR_FEATUREMATCHER_H
