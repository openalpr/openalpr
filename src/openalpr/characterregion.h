/*
 * Copyright (c) 2014 New Designs Unlimited, LLC
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

#ifndef OPENALPR_CHARACTERREGION_H
#define OPENALPR_CHARACTERREGION_H

#include "opencv2/imgproc/imgproc.hpp"
#include "constants.h"
#include "utility.h"
#include "textdetection/characteranalysis.h"
#include "config.h"
#include "pipeline_data.h"

class CharacterRegion
{

  public:
    CharacterRegion(PipelineData* pipeline_data);
    virtual ~CharacterRegion();


    int confidence;

    LineSegment getTopLine();
    LineSegment getBottomLine();

    LineSegment getCharBoxTop();
    LineSegment getCharBoxBottom();
    LineSegment getCharBoxLeft();
    LineSegment getCharBoxRight();


  protected:
    Config* config;
    bool debug;

    CharacterAnalysis *charAnalysis;
    cv::Mat findOuterBoxMask(std::vector<cv::Mat> thresholds, std::vector<std::vector<std::vector<cv::Point> > > allContours, std::vector<std::vector<cv::Vec4i> > allHierarchy);


    bool isPlateInverted(cv::Mat threshold, std::vector<std::vector<cv::Point> > contours, std::vector<cv::Vec4i> hierarchy, std::vector<bool> goodIndices);

};

#endif // OPENALPR_CHARACTERREGION_H
