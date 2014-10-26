/*
 * Copyright (c) 2014 New Designs Unlimited, LLC
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

#include "transformation.h"

using namespace std;
using namespace cv;

Transformation::Transformation(Mat bigImage, Mat smallImage, Rect regionInBigImage) {
  this->bigImage = bigImage;
  this->smallImage = smallImage;
  this->regionInBigImage = regionInBigImage;
}


Transformation::~Transformation() {
}

// Re-maps the coordinates from the smallImage to the coordinate space of the bigImage.
vector<Point2f> Transformation::transformSmallPointsToBigImage(vector<Point> points)
{
  vector<Point2f> bigPoints;
  for (uint i = 0; i < points.size(); i++)
  {
    float bigX = (points[i].x * ((float) regionInBigImage.width / smallImage.cols));
    float bigY = (points[i].y * ((float) regionInBigImage.height / smallImage.rows));

    bigX = bigX + regionInBigImage.x;
    bigY = bigY + regionInBigImage.y;

    bigPoints.push_back(Point2f(bigX, bigY));
  }

  return bigPoints;
}


Mat Transformation::getTransformationMatrix(vector<Point2f> corners, Size outputImageSize)
{
  // Corners of the destination image
  vector<Point2f> quad_pts;
  quad_pts.push_back(Point2f(0, 0));
  quad_pts.push_back(Point2f(outputImageSize.width, 0));
  quad_pts.push_back(Point2f(outputImageSize.width, outputImageSize.height));
  quad_pts.push_back(Point2f(0, outputImageSize.height));

  // Get transformation matrix
  Mat transmtx = getPerspectiveTransform(corners, quad_pts);

  return transmtx;
}

Mat Transformation::crop(Size outputImageSize, Mat transformationMatrix)
{
  
  
  Mat deskewed(outputImageSize, this->bigImage.type());
  
  // Apply perspective transformation to the image
  warpPerspective(this->bigImage, deskewed, transformationMatrix, deskewed.size(), INTER_CUBIC);

  
  

  return deskewed;
}

vector<Point2f> Transformation::remapSmallPointstoCrop(vector<Point2f> smallPoints, cv::Mat transformationMatrix)
{
  vector<Point2f> remappedPoints;
  perspectiveTransform(smallPoints, remappedPoints, transformationMatrix);
  
  return remappedPoints;
}
        
