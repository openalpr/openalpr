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
#include "charactersegmenter.h"
#include "platecorners.h"
#include "config.h"

using namespace std;
using namespace cv;

//vector<Rect> getCharacterRegions(Mat frame, vector<Rect> regionsOfInterest);
//vector<RotatedRect> getCharSegmentsBetweenLines(Mat img, vector<vector<Point> > contours, LineSegment top, LineSegment bottom);

class LicensePlateCandidate
{

  public:
    LicensePlateCandidate(Mat frame, Rect regionOfInterest, Config* config);
    virtual ~LicensePlateCandidate();

    float confidence;		// 0-100
    //vector<Point> points;	// top-left, top-right, bottom-right, bottom-left
    vector<Point2f> plateCorners;

    void recognize();

    Mat deskewed;
    CharacterSegmenter* charSegmenter;

  private:

    Config* config;

    Mat frame;
    Rect plateRegion;

    Mat filterByCharacterHue(vector<vector<Point> > charRegionContours);
    vector<Point> findPlateCorners(Mat inputImage, PlateLines plateLines, CharacterRegion charRegion);	// top-left, top-right, bottom-right, bottom-left

    vector<Point2f> transformPointsToOriginalImage(Mat bigImage, Mat smallImage, Rect region, vector<Point> corners);
    Mat deSkewPlate(Mat inputImage, vector<Point2f> corners);

};

#endif // OPENALPR_LICENSEPLATECANDIDATE_H
