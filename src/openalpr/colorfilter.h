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

#ifndef COLORFILTER_H
#define COLORFILTER_H

#include <iomanip>
#include "opencv2/imgproc/imgproc.hpp"

#include "constants.h"
#include "utility.h"
#include "config.h"

using namespace cv;
using namespace std;

class ColorFilter
{

  public:
    ColorFilter(Mat image, Mat characterMask, Config* config);
    virtual ~ColorFilter();

    Mat colorMask;

  private:

    Config* config;
    bool debug;

    Mat hsv;
    Mat charMask;

    bool grayscale;

    void preprocessImage();
    void findCharColors();

    bool imageIsGrayscale(Mat image);
    int getMajorityOpinion(vector<float> values, float minPercentAgreement, float maxValDifference);
};

#endif // COLORFILTER_H
