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

#ifndef OPENALPR_CHARACTERANALYSIS_H
#define OPENALPR_CHARACTERANALYSIS_H

#include "opencv2/imgproc/imgproc.hpp"
#include "utility.h"
#include "config.h"
#include "pipeline_data.h"
#include "textcontours.h"
#include "platemask.h"

class CharacterAnalysis
{

  public:
    CharacterAnalysis(PipelineData* pipeline_data);
    virtual ~CharacterAnalysis();


    cv::Mat bestThreshold;
    
    TextContours bestContours;
    //std::vector<std::vector<cv::Point> > bestContours;
    //std::vector<cv::Vec4i> bestHierarchy;
    //std::vector<bool> bestCharSegments;
    //int bestCharSegmentsCount;

    LineSegment topLine;
    LineSegment bottomLine;
    std::vector<cv::Point> linePolygon;
    std::vector<cv::Point> charArea;

    LineSegment charBoxTop;
    LineSegment charBoxBottom;
    LineSegment charBoxLeft;
    LineSegment charBoxRight;

    bool thresholdsInverted;
    bool isTwoLine;

    std::vector<TextContours> allTextContours;
    //std::vector<std::vector<std::vector<cv::Point> > > allContours;
    //std::vector<std::vector<cv::Vec4i> > allHierarchy;
    //std::vector<std::vector<bool> > charSegments;

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

    std::vector<cv::Point> getCharArea();
    std::vector<cv::Point> getBestVotedLines(cv::Mat img, TextContours textContours);
    void filterBetweenLines(cv::Mat img, TextContours& textContours, std::vector<cv::Point> outerPolygon );

    bool verifySize(cv::Mat r, float minHeightPx, float maxHeightPx);


};

#endif // OPENALPR_CHARACTERANALYSIS_H
