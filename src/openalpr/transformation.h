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

#ifndef OPENALPR_TRANSFORMATION_H
#define	OPENALPR_TRANSFORMATION_H

#include "opencv2/imgproc/imgproc.hpp"
#include "utility.h"

namespace alpr
{

  class Transformation {
  public:
    Transformation(cv::Mat bigImage, cv::Mat smallImage, cv::Rect regionInBigImage);
    virtual ~Transformation();

    std::vector<cv::Point2f> transformSmallPointsToBigImage(std::vector<cv::Point> points);
    std::vector<cv::Point2f> transformSmallPointsToBigImage(std::vector<cv::Point2f> points);

    cv::Mat getTransformationMatrix(std::vector<cv::Point2f> corners, cv::Size outputImageSize);
    cv::Mat getTransformationMatrix(std::vector<cv::Point2f> corners, std::vector<cv::Point2f> outputCorners);

    cv::Mat crop(cv::Size outputImageSize, cv::Mat transformationMatrix);
    std::vector<cv::Point2f> remapSmallPointstoCrop(std::vector<cv::Point> smallPoints, cv::Mat transformationMatrix);
    std::vector<cv::Point2f> remapSmallPointstoCrop(std::vector<cv::Point2f> smallPoints, cv::Mat transformationMatrix);

    cv::Size getCropSize(std::vector<cv::Point2f> areaCorners, cv::Size targetSize);

  private:
    cv::Mat bigImage;
    cv::Mat smallImage;
    cv::Rect regionInBigImage;

  };

}

#endif	/* OPENALPR_TRANSFORMATION_H */

