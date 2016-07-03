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

#ifndef OPENALPR_CHARACTERSEGMENTER_H
#define OPENALPR_CHARACTERSEGMENTER_H

#include "opencv2/imgproc/imgproc.hpp"
#include "constants.h"
#include "binarize_wolf.h"
#include "utility.h"
#include "histogramvertical.h"
#include "config.h"
#include "textdetection/textcontours.h"
#include "pipeline_data.h"

namespace alpr
{


  const cv::Scalar COLOR_DEBUG_EDGE(0,0,255); // Red
  const cv::Scalar COLOR_DEBUG_SPECKLES(0,0,255); // Red
  const cv::Scalar COLOR_DEBUG_MIN_HEIGHT(255,0,0); // Blue
  const cv::Scalar COLOR_DEBUG_MIN_AREA(255,0,0); // Blue
  const cv::Scalar COLOR_DEBUG_FULLBOX(255,255,0); // Blue-green
  const cv::Scalar COLOR_DEBUG_COLORFILTER(255,0,255); // Magenta
  const cv::Scalar COLOR_DEBUG_EMPTYFILTER(0,255,255); // Yellow

  class CharacterSegmenter
  {

    public:
      CharacterSegmenter(PipelineData* pipeline_data);
      virtual ~CharacterSegmenter();

      void segment();
      
      int confidence;


    private:
      Config* config;
      PipelineData* pipeline_data;


      LineSegment top;
      LineSegment bottom;

      std::vector<cv::Mat> imgDbgGeneral;
      std::vector<cv::Mat> imgDbgCleanStages;

      cv::Mat getCharBoxMask(cv::Mat img_threshold, std::vector<cv::Rect> charBoxes);

      void removeSmallContours(std::vector<cv::Mat> thresholds, float avgCharHeight, TextLine textLine);

      std::vector<cv::Rect> getHistogramBoxes(HistogramVertical histogram, float avgCharWidth, float avgCharHeight, float* score);
      std::vector<cv::Rect> getBestCharBoxes(cv::Mat img, std::vector<cv::Rect> charBoxes, float avgCharWidth);
      
      int getCharGap(cv::Rect leftBox, cv::Rect rightBox);
      std::vector<cv::Rect> combineCloseBoxes( std::vector<cv::Rect> charBoxes);

      std::vector<cv::Rect> get1DHits(cv::Mat img, int yOffset);

      void cleanCharRegions(std::vector<cv::Mat> thresholds, std::vector<cv::Rect> charRegions);
      void cleanBasedOnColor(std::vector<cv::Mat> thresholds, cv::Mat colorMask, std::vector<cv::Rect> charRegions);
      std::vector<cv::Rect> filterMostlyEmptyBoxes(std::vector<cv::Mat> thresholds, const  std::vector<cv::Rect> charRegions);
      cv::Mat filterEdgeBoxes(std::vector<cv::Mat> thresholds, const std::vector<cv::Rect> charRegions, float avgCharWidth, float avgCharHeight);

      int getLongestBlobLengthBetweenLines(cv::Mat img, int col);

      int isSkinnyLineInsideBox(cv::Mat threshold, cv::Rect box, std::vector<std::vector<cv::Point> > contours, std::vector<cv::Vec4i> hierarchy, float avgCharWidth, float avgCharHeight);

      std::vector<cv::Rect> convert1DHitsToRect(std::vector<std::pair<int, int> >  hits, LineSegment top, LineSegment bottom);
  };

}

#endif // OPENALPR_CHARACTERSEGMENTER_H
