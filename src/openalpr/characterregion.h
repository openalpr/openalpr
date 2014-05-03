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

#ifndef OPENALPR_CHARACTERREGION_H
#define OPENALPR_CHARACTERREGION_H

#include "opencv2/imgproc/imgproc.hpp"
#include "constants.h"
#include "utility.h"
#include "characteranalysis.h"
#include "config.h"

using namespace cv;
using namespace std;

class CharacterRegion
{

  public:
    CharacterRegion(Mat img, Config* config);
    virtual ~CharacterRegion();

    CharacterAnalysis *charAnalysis;

    int confidence;
    Mat getPlateMask();

    LineSegment getTopLine();
    LineSegment getBottomLine();
    //vector<Point> getLinePolygon();
    vector<Point> getCharArea();

    LineSegment getCharBoxTop();
    LineSegment getCharBoxBottom();
    LineSegment getCharBoxLeft();
    LineSegment getCharBoxRight();

    bool thresholdsInverted();

  protected:
    Config* config;
    bool debug;

    Mat findOuterBoxMask(vector<Mat> thresholds, vector<vector<vector<Point> > > allContours, vector<vector<Vec4i> > allHierarchy);

    vector<bool> filter(Mat img, vector<vector<Point> > contours, vector<Vec4i> hierarchy);
    vector<bool> filterByBoxSize(Mat img, vector<vector<Point> > contours, vector<bool> goodIndices, float minHeightPx, float maxHeightPx);
    vector<bool> filterByParentContour( vector< vector< Point> > contours, vector<Vec4i> hierarchy, vector<bool> goodIndices);
    vector<bool> filterContourHoles(vector<vector<Point> > contours, vector<Vec4i> hierarchy, vector<bool> goodIndices);

    vector<Point> getBestVotedLines(Mat img, vector<vector<Point> > contours, vector<bool> goodIndices);
    //vector<Point> getCharSegmentsBetweenLines(Mat img, vector<vector<Point> > contours, vector<Point> outerPolygon);
    vector<bool> filterBetweenLines(Mat img, vector<vector<Point> > contours, vector<Vec4i> hierarchy, vector<Point> outerPolygon, vector<bool> goodIndices);
    Mat getCharacterMask(Mat img, vector<vector<Point> > contours, vector<Vec4i> hierarchy, vector<bool> goodIndices);

    vector<Rect> wrapContours(vector<vector<Point> > contours);
    bool verifySize(Mat r, float minHeightPx, float maxHeightPx);

    int getGoodIndicesCount(vector<bool> goodIndices);

    bool isPlateInverted(Mat threshold, vector<vector<Point> > contours, vector<Vec4i> hierarchy, vector<bool> goodIndices);

};

#endif // OPENALPR_CHARACTERREGION_H
