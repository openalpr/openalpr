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

#ifndef OPENALPR_REGIONDETECTOR_H
#define OPENALPR_REGIONDETECTOR_H

#include <iostream>
#include <stdio.h>

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/ocl/ocl.hpp"

#include "utility.h"
#include "support/timing.h"
#include "constants.h"

struct PlateRegion
{
  Rect rect;
  vector<PlateRegion> children;
};

class RegionDetector
{

  public:
    RegionDetector(Config* config);
    virtual ~RegionDetector();

    bool isLoaded();
    vector<PlateRegion> detect(Mat frame);

  private:
    Config* config;

    float scale_factor;
    CascadeClassifier* plate_cascade;

    bool loaded;

    vector<PlateRegion> doCascade(Mat frame);

    vector<PlateRegion> aggregateRegions(vector<Rect> regions);
};

#endif // OPENALPR_REGIONDETECTOR_H
