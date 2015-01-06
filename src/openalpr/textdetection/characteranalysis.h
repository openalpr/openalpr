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

#ifndef OPENALPR_CHARACTERANALYSIS_H
#define OPENALPR_CHARACTERANALYSIS_H

#include <algorithm>
#include "opencv2/imgproc/imgproc.hpp"
#include "utility.h"
#include "config.h"
#include "pipeline_data.h"
#include "textcontours.h"
#include "platemask.h"
#include "linefinder.h"

namespace alpr
{

  class CharacterAnalysis
  {

    public:
      CharacterAnalysis(PipelineData* pipeline_data);
      virtual ~CharacterAnalysis();

      int confidence;

      cv::Mat bestThreshold;

      TextContours bestContours;


      std::vector<TextContours> allTextContours;

      void analyze();

      cv::Mat getCharacterMask();

    private:
      PipelineData* pipeline_data;
      Config* config;

      cv::Mat findOuterBoxMask( );

      bool isPlateInverted();
      void filter(cv::Mat img, TextContours& textContours);

      void filterByBoxSize(TextContours& textContours, int minHeightPx, int maxHeightPx);
      void filterByParentContour( TextContours& textContours );
      void filterContourHoles(TextContours& textContours);
      void filterByOuterMask(TextContours& textContours);

      std::vector<cv::Point> getCharArea(LineSegment topLine, LineSegment bottomLine);
      void filterBetweenLines(cv::Mat img, TextContours& textContours, std::vector<TextLine> textLines );

      bool verifySize(cv::Mat r, float minHeightPx, float maxHeightPx);


  };

}

#endif // OPENALPR_CHARACTERANALYSIS_H
