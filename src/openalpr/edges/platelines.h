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

#ifndef OPENALPR_PLATELINES_H
#define OPENALPR_PLATELINES_H

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "utility.h"
#include "binarize_wolf.h"
#include "config.h"
#include "pipeline_data.h"

namespace alpr
{

  struct PlateLine
  {
    LineSegment line;
    float confidence;
  };

  class PlateLines
  {

    public:
      PlateLines(PipelineData* pipelineData);
      virtual ~PlateLines();

      void processImage(cv::Mat img, std::vector<TextLine> textLines, float sensitivity=1.0);

      std::vector<PlateLine> horizontalLines;
      std::vector<PlateLine> verticalLines;

      std::vector<cv::Point> winningCorners;

    private:

      PipelineData* pipelineData;
      bool debug;

      cv::Mat customGrayscaleConversion(cv::Mat src);
      void findLines(cv::Mat inputImage);
      std::vector<PlateLine> getLines(cv::Mat edges, float sensitivityMultiplier, bool vertical);
  };

}

#endif // OPENALPR_PLATELINES_H
