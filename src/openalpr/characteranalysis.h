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

#ifndef OPENALPR_CHARACTERANALYSIS_H
#define OPENALPR_CHARACTERANALYSIS_H

#include "opencv2/imgproc/imgproc.hpp"
#include "constants.h"
#include "utility.h"
#include "config.h"
#include "pipeline_data.h"

class CharacterAnalysis
{

  public:
    CharacterAnalysis(PipelineData* pipeline_data);
    virtual ~CharacterAnalysis();

    bool hasPlateMask;
    cv::Mat plateMask;

    cv::Mat bestThreshold;
    std::vector<std::vector<cv::Point> > bestContours;
    std::vector<cv::Vec4i> bestHierarchy;
    std::vector<bool> bestCharSegments;
    int bestCharSegmentsCount;

    LineSegment topLine;
    LineSegment bottomLine;
    std::vector<cv::Point> linePolygon;
    std::vector<cv::Point> charArea;

    LineSegment charBoxTop;
    LineSegment charBoxBottom;
    LineSegment charBoxLeft;
    LineSegment charBoxRight;

    bool thresholdsInverted;

    std::vector<std::vector<std::vector<cv::Point> > > allContours;
    std::vector<std::vector<cv::Vec4i> > allHierarchy;
    std::vector<std::vector<bool> > charSegments;

    void analyze();

    cv::Mat getCharacterMask();

  private:
    PipelineData* pipeline_data;
    Config* config;

    cv::Mat findOuterBoxMask( );

    bool isPlateInverted();
    std::vector<bool> filter(cv::Mat img, std::vector<std::vector<cv::Point> > contours, std::vector<cv::Vec4i> hierarchy);

    std::vector<bool> filterByBoxSize(std::vector<std::vector<cv::Point> > contours, std::vector<bool> goodIndices, int minHeightPx, int maxHeightPx);
    std::vector<bool> filterByParentContour( std::vector< std::vector<cv::Point> > contours, std::vector<cv::Vec4i> hierarchy, std::vector<bool> goodIndices);
    std::vector<bool> filterContourHoles(std::vector<std::vector<cv::Point> > contours, std::vector<cv::Vec4i> hierarchy, std::vector<bool> goodIndices);
    std::vector<bool> filterByOuterMask(std::vector<std::vector<cv::Point> > contours, std::vector<cv::Vec4i> hierarchy, std::vector<bool> goodIndices);

    std::vector<cv::Point> getCharArea();
    std::vector<cv::Point> getBestVotedLines(cv::Mat img, std::vector<std::vector<cv::Point> > contours, std::vector<bool> goodIndices);
    //vector<Point> getCharSegmentsBetweenLines(Mat img, vector<vector<Point> > contours, vector<Point> outerPolygon);
    std::vector<bool> filterBetweenLines(cv::Mat img, std::vector<std::vector<cv::Point> > contours, std::vector<cv::Vec4i> hierarchy, std::vector<cv::Point> outerPolygon, std::vector<bool> goodIndices);

    bool verifySize(cv::Mat r, float minHeightPx, float maxHeightPx);

    int getGoodIndicesCount(std::vector<bool> goodIndices);

};

#endif // OPENALPR_CHARACTERANALYSIS_H
