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

#ifndef OPENALPR_PLATECORNERS_H
#define OPENALPR_PLATECORNERS_H

#include "opencv2/imgproc/imgproc.hpp"
#include "characterregion.h"
#include "platelines.h"
#include "utility.h"
#include "config.h"

using namespace cv;
using namespace std;

#define NO_LINE -1

#define SCORING_MISSING_SEGMENT_PENALTY_VERTICAL	10
#define SCORING_MISSING_SEGMENT_PENALTY_HORIZONTAL	15

#define SCORING_BOXINESS_WEIGHT				0.8
#define SCORING_PLATEHEIGHT_WEIGHT			2.2
#define SCORING_TOP_BOTTOM_SPACE_VS_CHARHEIGHT_WEIGHT	0.05
#define SCORING_ANGLE_MATCHES_LPCHARS_WEIGHT		1.1
#define SCORING_VERTICALDISTANCE_WEIGHT		0.1

#define SCORING_VERTICALDISTANCE_FROMEDGE_WEIGHT	0.05

class PlateCorners
{

  public:
    PlateCorners(Mat inputImage, PlateLines* plateLines, CharacterRegion* charRegion, Config* config);
    virtual ~PlateCorners();

    vector<Point> findPlateCorners();

    float confidence;

  private:

    Config* config;
    Mat inputImage;
    float charHeight;
    float charAngle;

    float bestHorizontalScore;
    float bestVerticalScore;
    LineSegment bestTop;
    LineSegment bestBottom;
    LineSegment bestLeft;
    LineSegment bestRight;

    PlateLines* plateLines;
    CharacterRegion* charRegion;

    void scoreHorizontals( int h1, int h2 );
    void scoreVerticals( int v1, int v2 );

};

#endif // OPENALPR_PLATELINES_H
