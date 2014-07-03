/*
 * Copyright (c) 2013 New Designs Unlimited, LLC
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

#ifndef OPENALPR_LICENSEPLATECANDIDATE_H
#define OPENALPR_LICENSEPLATECANDIDATE_H

#include <iostream>
#include <stdio.h>
#include <vector>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"

#include "utility.h"
#include "constants.h"
#include "platelines.h"
#include "characterregion.h"
#include "segmentation/charactersegmenter.h"
#include "platecorners.h"
#include "config.h"
#include "pipeline_data.h"

//vector<Rect> getCharacterRegions(Mat frame, vector<Rect> regionsOfInterest);
//vector<RotatedRect> getCharSegmentsBetweenLines(Mat img, vector<vector<Point> > contours, LineSegment top, LineSegment bottom);

class LicensePlateCandidate
{

  public:
    LicensePlateCandidate(PipelineData* pipeline_data);
    virtual ~LicensePlateCandidate();


    void recognize();


  private:
    PipelineData* pipeline_data;
    Config* config;

    CharacterSegmenter* charSegmenter;

    cv::Mat filterByCharacterHue(std::vector<std::vector<cv::Point> > charRegionContours);
    std::vector<cv::Point> findPlateCorners(cv::Mat inputImage, PlateLines plateLines, CharacterRegion charRegion);	// top-left, top-right, bottom-right, bottom-left

    std::vector<cv::Point2f> transformPointsToOriginalImage(cv::Mat bigImage, cv::Mat smallImage, cv::Rect region, std::vector<cv::Point> corners);
    cv::Mat deSkewPlate(cv::Mat inputImage, std::vector<cv::Point2f> corners);

};

#endif // OPENALPR_LICENSEPLATECANDIDATE_H
