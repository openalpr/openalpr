/*
 * Copyright (c) 2015 OpenALPR Technology, Inc.
 * Open source Automated License Plate Recognition [http://www.openalpr.com]
 *
 * This file is part of OpenALPR.
 *
 * OpenALPR is free software: you can redistribute it and/or modify
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

#ifndef OPENALPR_HISTOGRAM_H
#define OPENALPR_HISTOGRAM_H

#include "opencv2/imgproc/imgproc.hpp"
#include "utility.h"

namespace alpr
{

  class Histogram
  {
  public:
    Histogram();
    virtual ~Histogram();

    cv::Mat histoImg;

    // Returns the lowest X position between two points.
    int getLocalMinimum(int leftX, int rightX);
    // Returns the highest X position between two points.
    int getLocalMaximum(int leftX, int rightX);

    int getHeightAt(int x);

    std::vector<std::pair<int, int> > get1DHits(int yOffset);

  protected:

    std::vector<int> colHeights;


    void analyzeImage(cv::Mat inputImage, cv::Mat mask, bool use_y_axis);

    int detect_peak(const double *data, int data_count, int *emi_peaks,
                    int *num_emi_peaks, int max_emi_peaks, int *absop_peaks,
                    int *num_absop_peaks, int max_absop_peaks, double delta,
                    int emi_first);
  };

}
#endif //OPENALPR_HISTOGRAM_H
