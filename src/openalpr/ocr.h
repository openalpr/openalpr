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

#ifndef OPENALPR_OCR_H
#define OPENALPR_OCR_H

#include <iostream>
#include <stdio.h>

#include "utility.h"
#include "postprocess.h"
#include "config.h"

#include "constants.h"
#include "opencv2/imgproc/imgproc.hpp"

#include "baseapi.h"
using namespace  tesseract;
using namespace std;
using namespace cv;

class OCR
{

  public:
    OCR(Config* config);
    virtual ~OCR();

    void performOCR(vector<Mat> thresholds, vector<Rect> charRegions);

    PostProcess* postProcessor;
    //string recognizedText;
    //float confidence;
    //float overallConfidence;

  private:
    Config* config;

    TessBaseAPI *tesseract;

};

#endif // OPENALPR_OCR_H
