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

#ifndef OPENALPR_CHARACTERSEGMENTER_H
#define OPENALPR_CHARACTERSEGMENTER_H

#include "opencv2/imgproc/imgproc.hpp"
#include "constants.h"
#include "binarize_wolf.h"
#include "utility.h"
#include "characterregion.h"
#include "colorfilter.h"
#include "verticalhistogram.h"
#include "config.h"

using namespace cv;
using namespace std;

//const float MIN_BOX_WIDTH_PX = 4; // 4 pixels

const Scalar COLOR_DEBUG_EDGE(0,0,255); // Red
const Scalar COLOR_DEBUG_SPECKLES(0,0,255); // Red
const Scalar COLOR_DEBUG_MIN_HEIGHT(255,0,0); // Blue
const Scalar COLOR_DEBUG_MIN_AREA(255,0,0); // Blue
const Scalar COLOR_DEBUG_FULLBOX(255,255,0); // Blue-green
const Scalar COLOR_DEBUG_COLORFILTER(255,0,255); // Magenta
const Scalar COLOR_DEBUG_EMPTYFILTER(0,255,255); // Yellow

class CharacterSegmenter
{

  public:
    CharacterSegmenter(Mat img, bool invertedColors, Config* config);
    virtual ~CharacterSegmenter();

    vector<Rect> characters;
    int confidence;

    vector<Mat> getThresholds();

  private:
    Config* config;

    CharacterAnalysis* charAnalysis;

    LineSegment top;
    LineSegment bottom;

    vector<Mat> imgDbgGeneral;
    vector<Mat> imgDbgCleanStages;

    vector<bool> filter(Mat img, vector<vector<Point> > contours, vector<Vec4i> hierarchy);
    vector<bool> filterByBoxSize(vector< vector< Point> > contours, vector<bool> goodIndices, float minHeightPx, float maxHeightPx);
    vector<bool> filterBetweenLines(Mat img, vector<vector<Point> > contours, vector<Vec4i> hierarchy, vector<Point> outerPolygon, vector<bool> goodIndices);
    vector<bool> filterContourHoles(vector<vector<Point> > contours, vector<Vec4i> hierarchy, vector<bool> goodIndices);

    vector<Point> getBestVotedLines(Mat img, vector<vector<Point> > contours, vector<bool> goodIndices);
    int getGoodIndicesCount(vector<bool> goodIndices);

    Mat getCharacterMask(Mat img_threshold, vector<vector<Point> > contours, vector<Vec4i> hierarchy, vector<bool> goodIndices);
    Mat getCharBoxMask(Mat img_threshold, vector<Rect> charBoxes);

    void removeSmallContours(vector<Mat> thresholds, vector<vector<vector<Point > > > allContours, float avgCharWidth, float avgCharHeight);

    Mat getVerticalHistogram(Mat img, Mat mask);
    vector<Rect> getHistogramBoxes(VerticalHistogram histogram, float avgCharWidth, float avgCharHeight, float* score);
    vector<Rect> getBestCharBoxes(Mat img, vector<Rect> charBoxes, float avgCharWidth);
    vector<Rect> combineCloseBoxes( vector<Rect> charBoxes, float avgCharWidth);

    vector<Rect> get1DHits(Mat img, int yOffset);

    void cleanCharRegions(vector<Mat> thresholds, vector<Rect> charRegions);
    void cleanBasedOnColor(vector<Mat> thresholds, Mat colorMask, vector<Rect> charRegions);
    void cleanMostlyFullBoxes(vector<Mat> thresholds, const vector<Rect> charRegions);
    vector<Rect> filterMostlyEmptyBoxes(vector<Mat> thresholds, const  vector<Rect> charRegions);
    void filterEdgeBoxes(vector<Mat> thresholds, const vector<Rect> charRegions, float avgCharWidth, float avgCharHeight);

    int getLongestBlobLengthBetweenLines(Mat img, int col);

    int isSkinnyLineInsideBox(Mat threshold, Rect box, vector<vector<Point> > contours, vector<Vec4i> hierarchy, float avgCharWidth, float avgCharHeight);

    vector<Point> getEncapsulatingLines(Mat img, vector<vector<Point> > contours, vector<bool> goodIndices);
};

#endif // OPENALPR_CHARACTERSEGMENTER_H
