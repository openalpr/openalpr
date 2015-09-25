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

// This class finds lines of text given an array of contours

#ifndef OPENALPR_LINEFINDER_H
#define	OPENALPR_LINEFINDER_H

#include "opencv2/imgproc/imgproc.hpp"
#include "textcontours.h"
#include "textline.h"
#include "pipeline_data.h"
#include "segmentation/histogramhorizontal.h"

namespace alpr
{

  class CharPointInfo
  {
  public:
    CharPointInfo(std::vector<cv::Point> contour, int index);

    cv::Rect boundingBox;
    cv::Point top;
    cv::Point bottom;
    int contourIndex;

  };

  class LineFinder {
  public:
    LineFinder(PipelineData* pipeline_data);
    virtual ~LineFinder();

    std::vector<std::vector<cv::Point> > findLines(cv::Mat image, const TextContours contours);
  private:
    PipelineData* pipeline_data;

    // Returns 4 points, counter clockwise that bound the detected character area
    std::vector<cv::Point> getBestLine(const TextContours contours, std::vector<CharPointInfo> charPoints);
    
    // Extends the top and bottom lines to the left and right edge of the image.  Returns 4 points, counter clockwise.
    std::vector<cv::Point> extendToEdges(cv::Size imageSize, std::vector<cv::Point> charArea);

    
    std::vector<cv::Point> findNextBestLine(cv::Size imageSize, std::vector<cv::Point> bestLine);
    // Gets a polygon that covers the entire area we wish to run a horizontal histogram over
    // This needs to be done to handle rotation/skew
    std::vector<cv::Point> calculateCroppedRegionForHistogram(cv::Size imageSize, std::vector<cv::Point> charArea);
  };
}

#endif	/* OPENALPR_LINEFINDER_H */

