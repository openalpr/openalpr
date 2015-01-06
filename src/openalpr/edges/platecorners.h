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

#ifndef OPENALPR_PLATECORNERS_H
#define OPENALPR_PLATECORNERS_H

#include "opencv2/imgproc/imgproc.hpp"
#include "platelines.h"
#include "utility.h"
#include "config.h"
#include "textlinecollection.h"
#include "scorekeeper.h"

#define NO_LINE -1

#define SCORING_MISSING_SEGMENT_PENALTY_VERTICAL        10
#define SCORING_MISSING_SEGMENT_PENALTY_HORIZONTAL      1

#define SCORING_PLATEHEIGHT_WEIGHT                      2.2
#define SCORING_TOP_BOTTOM_SPACE_VS_CHARHEIGHT_WEIGHT	2.0
#define SCORING_ANGLE_MATCHES_LPCHARS_WEIGHT            1.1

#define SCORING_DISTANCE_WEIGHT_VERTICAL                4.0

#define SCORING_LINE_CONFIDENCE_WEIGHT                  18.0

namespace alpr
{

  class PlateCorners
  {

    public:
      PlateCorners(cv::Mat inputImage, PlateLines* plateLines, PipelineData* pipelineData, std::vector<TextLine> textLines) ;

      virtual ~PlateCorners();

      std::vector<cv::Point> findPlateCorners();

      float confidence;

    private:

      PipelineData* pipelineData;
      cv::Mat inputImage;

      std::vector<TextLine> textLines;
      TextLineCollection tlc;

      float bestHorizontalScore;
      float bestVerticalScore;
      LineSegment bestTop;
      LineSegment bestBottom;
      LineSegment bestLeft;
      LineSegment bestRight;

      PlateLines* plateLines;

      void scoreHorizontals( int h1, int h2 );
      void scoreVerticals( int v1, int v2 );

  };

}
#endif // OPENALPR_PLATELINES_H
