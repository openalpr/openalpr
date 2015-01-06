/*
 * Copyright (c) 2015 New Designs Unlimited, LLC
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

#ifndef OPENALPR_VERTICALHISTOGRAM_H
#define OPENALPR_VERTICALHISTOGRAM_H

#include "opencv2/imgproc/imgproc.hpp"

namespace alpr
{

  struct Valley
  {
    int startIndex;
    int endIndex;
    int width;
    int pixelsWithin;
  };

  enum HistogramDirection { RISING, FALLING, FLAT };

  class VerticalHistogram
  {

    public:
      VerticalHistogram(cv::Mat inputImage, cv::Mat mask);
      virtual ~VerticalHistogram();

      cv::Mat histoImg;

      // Returns the lowest X position between two points.
      int getLocalMinimum(int leftX, int rightX);
      // Returns the highest X position between two points.
      int getLocalMaximum(int leftX, int rightX);

      int getHeightAt(int x);

    private:
      std::vector<int> colHeights;
      int highestPeak;
      int lowestValley;
      std::vector<Valley> valleys;

      void analyzeImage(cv::Mat inputImage, cv::Mat mask);
      void findValleys();

      HistogramDirection getHistogramDirection(unsigned int index);
  };

}
  
#endif // OPENALPR_VERTICALHISTOGRAM_H
