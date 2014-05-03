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

#ifndef OPENALPR_PLATELINES_H
#define OPENALPR_PLATELINES_H

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "utility.h"
#include "binarize_wolf.h"
#include "config.h"
#include "characterregion.h"

using namespace cv;
using namespace std;

class PlateLines
{

  public:
    PlateLines(Config* config);
    virtual ~PlateLines();

    void processImage(Mat img, CharacterRegion* charRegion, float sensitivity=1.0);

    vector<LineSegment> horizontalLines;
    vector<LineSegment> verticalLines;

    vector<Point> winningCorners;

  private:
    Config* config;
    bool debug;

    Mat customGrayscaleConversion(Mat src);
    void findLines(Mat inputImage);
    vector<LineSegment> getLines(Mat edges, float sensitivityMultiplier, bool vertical);
};

#endif // OPENALPR_PLATELINES_H
