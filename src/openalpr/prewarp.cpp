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


#include <opencv2/highgui/highgui.hpp>

#include "prewarp.h"

using namespace std;
using namespace cv;

namespace alpr
{

  PreWarp::PreWarp(Config* config) {
    this->config = config;
    
    string warp_config = config->prewarp;
    
    // Do a cursory verification based on number of commas
    int commacount = count(warp_config.begin(), warp_config.end(), ',');
    
    if (warp_config.length() < 4)
    {
      // No config specified.  ignore
      if (this->config->debugPrewarp)
        cout << "No prewarp configuration specified" << endl;

      this->valid = false;
    }
    else if (commacount != 9)
    {
      if (this->config->debugPrewarp)
        cout << "Invalid prewarp configuration" << endl;

      this->valid = false;
    }
    else
    {

      // Parse the warp_config
      int first_comma = warp_config.find(",");


      string name = warp_config.substr(0, first_comma);
      
      if (name != "planar")
      {
        this->valid = false;
      }
      else
      {
        stringstream ss(warp_config.substr(first_comma + 1, warp_config.length()));

        ss >> w;
        ss.ignore();
        ss >> h;
        ss.ignore();
        ss >> rotationx;
        ss.ignore();  // Ignore comma
        ss >> rotationy;
        ss.ignore();  // Ignore comma
        ss >> rotationz;
        ss.ignore();  // Ignore comma
        ss >> stretchX;
        ss.ignore();  // Ignore comma
        ss >> dist;
        ss.ignore();  // Ignore comma
        ss >> panX;
        ss.ignore();  // Ignore comma
        ss >> panY;
        
        this->valid = true;
      }

    }
  }


  PreWarp::~PreWarp() {
  }
  
  
  cv::Mat PreWarp::warpImage(Mat image) {
    if (!this->valid)
    {
      if (this->config->debugPrewarp)
        cout << "prewarp skipped due to missing prewarp config" << endl;
      return image;
    }
    
      
    float width_ratio = w / ((float)image.cols);
    float height_ratio = h / ((float)image.rows);

    float rx = rotationx * width_ratio;
    float ry = rotationy * width_ratio;
    float px = panX / width_ratio;
    float py = panY / height_ratio;


    transform = findTransform(image.cols, image.rows, rx, ry, rotationz, px, py, stretchX, dist);
    
    
    Mat warped_image;
  
    warpPerspective(image, warped_image, transform, image.size(), INTER_CUBIC | WARP_INVERSE_MAP);

    
    if (this->config->debugPrewarp && this->config->debugShowImages)
    {
      imshow("Prewarp", warped_image);
    }
    return warped_image;
  }

  // Projects a "region of interest" into the new space
  // The rect needs to be converted to points, warped, then converted back into a 
  // bounding rectangle
  vector<Rect> PreWarp::projectRects(vector<Rect> rects, int maxWidth, int maxHeight, bool inverse) {
    
    if (!this->valid)
      return rects;
    
    vector<Rect> projected_rects;
    
    for (unsigned int i = 0; i < rects.size(); i++)
    {
      vector<Point2f> points;
      points.push_back(Point(rects[i].x, rects[i].y));
      points.push_back(Point(rects[i].x + rects[i].width, rects[i].y));
      points.push_back(Point(rects[i].x + rects[i].width, rects[i].y + rects[i].height));
      points.push_back(Point(rects[i].x, rects[i].y + rects[i].height));
      
      vector<Point2f> projectedPoints = projectPoints(points, inverse);
      
      Rect projectedRect = boundingRect(projectedPoints);
      projectedRect = expandRect(projectedRect, 0, 0, maxWidth, maxHeight);
      projected_rects.push_back(projectedRect);
      
    }
    
    return projected_rects;
  }

  vector<Point2f> PreWarp::projectPoints(vector<Point2f> points, bool inverse) {
    
    if (!this->valid)
      return points;
    
    vector<Point2f> output;
    
    if (!inverse)
      perspectiveTransform(points, output, transform.inv());
    else
      perspectiveTransform(points, output, transform);
    
    return output;
  }
  

  void PreWarp::projectPlateRegions(vector<PlateRegion>& plateRegions, int maxWidth, int maxHeight, bool inverse){
    
    if (!this->valid)
      return;
    
    for (unsigned int i = 0; i < plateRegions.size(); i++)
    {
      vector<Rect> singleRect;
      singleRect.push_back(plateRegions[i].rect);
      vector<Rect> transformedRect = projectRects(singleRect, maxWidth, maxHeight, inverse);
      plateRegions[i].rect.x = transformedRect[0].x;
      plateRegions[i].rect.y = transformedRect[0].y;
      plateRegions[i].rect.width = transformedRect[0].width;
      plateRegions[i].rect.height = transformedRect[0].height;
      
      projectPlateRegions(plateRegions[i].children, maxWidth, maxHeight, inverse);
    }
  }
  
  cv::Mat PreWarp::findTransform(float w, float h, 
          float rotationx, float rotationy, float rotationz, 
          float panX, float panY, float stretchX, float dist) {

    float alpha = rotationx;
    float beta = rotationy;
    float gamma = rotationz;
    float f = 1.0;

    // Projection 2D -> 3D matrix
    Mat A1 = (Mat_<double>(4,3) <<
        1, 0, -w/2,
        0, 1, -h/2,
        0, 0,    0,
        0, 0,    1);
    
    // Camera Intrisecs matrix 3D -> 2D
    Mat A2 = (Mat_<double>(3,4) <<
        f, 0, w/2, 0,
        0, f, h/2, 0,
        0, 0,   1, 0);

    // Rotation matrices around the X axis
    Mat Rx = (Mat_<double>(4, 4) <<
        1,          0,           0, 0,
        0, cos(alpha), -sin(alpha), 0,
        0, sin(alpha),  cos(alpha), 0,
        0,          0,           0, 1);

    // Rotation matrices around the Y axis
    Mat Ry = (Mat_<double>(4, 4) <<
        cos(beta), 0, sin(beta), 0,
        0, 1, 0, 0,
        -sin(beta), 0,  cos(beta), 0,
        0,          0,           0, 1);

    // Rotation matrices around the Z axis
    Mat Rz = (Mat_<double>(4, 4) <<
        cos(gamma), -sin(gamma), 0, 0,
        sin(gamma), cos(gamma), 0, 0,
       0, 0, 1, 0,
        0,          0,           0, 1);

    Mat R = Rx*Ry*Rz;

    // Translation matrix on the Z axis 
    Mat T = (Mat_<double>(4, 4) <<
        stretchX, 0, 0, panX,
        0, 1, 0, panY,
        0, 0, 1, dist,
        0, 0, 0, 1);


    return A2 * (T * (R * A1));

  }

  
}

