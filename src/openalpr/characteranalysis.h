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

using namespace cv;
using namespace std;

class CharacterAnalysis
{

  public:
    CharacterAnalysis(Mat img, Config* config);
    virtual ~CharacterAnalysis();

    bool hasPlateMask;
    Mat plateMask;

    Mat bestThreshold;
    vector<vector<Point> > bestContours;
    vector<Vec4i> bestHierarchy;
    vector<bool> bestCharSegments;
    int bestCharSegmentsCount;

    LineSegment topLine;
    LineSegment bottomLine;
    vector<Point> linePolygon;
    vector<Point> charArea;

    LineSegment charBoxTop;
    LineSegment charBoxBottom;
    LineSegment charBoxLeft;
    LineSegment charBoxRight;

    bool thresholdsInverted;

    vector<Mat> thresholds;
    vector<vector<vector<Point> > > allContours;
    vector<vector<Vec4i> > allHierarchy;
    vector<vector<bool> > charSegments;

    void analyze();

    Mat getCharacterMask();

  private:
    Config* config;

    Mat img_gray;

    Mat findOuterBoxMask( );

    bool isPlateInverted();
    vector<bool> filter(Mat img, vector<vector<Point> > contours, vector<Vec4i> hierarchy);

    vector<bool> filterByBoxSize(vector<vector<Point> > contours, vector<bool> goodIndices, int minHeightPx, int maxHeightPx);
    vector<bool> filterByParentContour( vector< vector< Point> > contours, vector<Vec4i> hierarchy, vector<bool> goodIndices);
    vector<bool> filterContourHoles(vector<vector<Point> > contours, vector<Vec4i> hierarchy, vector<bool> goodIndices);
    vector<bool> filterByOuterMask(vector<vector<Point> > contours, vector<Vec4i> hierarchy, vector<bool> goodIndices);

    vector<Point> getCharArea();
    vector<Point> getBestVotedLines(Mat img, vector<vector<Point> > contours, vector<bool> goodIndices);
    //vector<Point> getCharSegmentsBetweenLines(Mat img, vector<vector<Point> > contours, vector<Point> outerPolygon);
    vector<bool> filterBetweenLines(Mat img, vector<vector<Point> > contours, vector<Vec4i> hierarchy, vector<Point> outerPolygon, vector<bool> goodIndices);

    bool verifySize(Mat r, float minHeightPx, float maxHeightPx);

    int getGoodIndicesCount(vector<bool> goodIndices);

};

#endif // OPENALPR_CHARACTERANALYSIS_H
