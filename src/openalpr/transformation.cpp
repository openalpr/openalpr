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

#include "transformation.h"

using namespace std;
using namespace cv;

namespace alpr
{

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
    vector<Point2f> floatPoints;
    for (unsigned int i = 0; i < points.size(); i++)
      floatPoints.push_back(points[i]);

    return transformSmallPointsToBigImage(floatPoints);

  }

  // Re-maps the coordinates from the smallImage to the coordinate space of the bigImage.
  vector<Point2f> Transformation::transformSmallPointsToBigImage(vector<Point2f> points)
  {
    vector<Point2f> bigPoints;
    for (unsigned int i = 0; i < points.size(); i++)
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

    return getTransformationMatrix(corners, quad_pts);
  }

  Mat Transformation::getTransformationMatrix(vector<Point2f> corners, vector<Point2f> outputCorners)
  {

    // Get transformation matrix
    Mat transmtx = getPerspectiveTransform(corners, outputCorners);

    return transmtx;
  }


  Mat Transformation::crop(Size outputImageSize, Mat transformationMatrix)
  {


    Mat deskewed(outputImageSize, this->bigImage.type());

    // Apply perspective transformation to the image
    warpPerspective(this->bigImage, deskewed, transformationMatrix, deskewed.size(), INTER_CUBIC);




    return deskewed;
  }

  vector<Point2f> Transformation::remapSmallPointstoCrop(vector<Point> smallPoints, cv::Mat transformationMatrix)
  {
    vector<Point2f> floatPoints;
    for (unsigned int i = 0; i < smallPoints.size(); i++)
      floatPoints.push_back(smallPoints[i]);

    return remapSmallPointstoCrop(floatPoints, transformationMatrix);
  }

  vector<Point2f> Transformation::remapSmallPointstoCrop(vector<Point2f> smallPoints, cv::Mat transformationMatrix)
  {
    vector<Point2f> remappedPoints;
    perspectiveTransform(smallPoints, remappedPoints, transformationMatrix);

    return remappedPoints;
  }

  Size Transformation::getCropSize(vector<Point2f> areaCorners, Size targetSize)
  {
    // Figure out the approximate width/height of the license plate region, so we can maintain the aspect ratio.
    LineSegment leftEdge(round(areaCorners[3].x), round(areaCorners[3].y), round(areaCorners[0].x), round(areaCorners[0].y));
    LineSegment rightEdge(round(areaCorners[2].x), round(areaCorners[2].y), round(areaCorners[1].x), round(areaCorners[1].y));
    LineSegment topEdge(round(areaCorners[0].x), round(areaCorners[0].y), round(areaCorners[1].x), round(areaCorners[1].y));
    LineSegment bottomEdge(round(areaCorners[3].x), round(areaCorners[3].y), round(areaCorners[2].x), round(areaCorners[2].y));

    float w = distanceBetweenPoints(leftEdge.midpoint(), rightEdge.midpoint());
    float h = distanceBetweenPoints(bottomEdge.midpoint(), topEdge.midpoint());
    
    if (w <= 0 || h <= 0)
      return Size(0,0);
    
    float aspect = w/h;
    int width = targetSize.width;
    int height = round(((float) width) / aspect);
    if (height > targetSize.height)
    {
      height = targetSize.height;
      width = round(((float) height) * aspect);
    }

    return Size(width, height);
  }
  
}