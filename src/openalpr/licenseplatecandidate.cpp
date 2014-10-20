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

#include <opencv2/core/core.hpp>

#include "licenseplatecandidate.h"

using namespace std;
using namespace cv;

LicensePlateCandidate::LicensePlateCandidate(PipelineData* pipeline_data)
{
  this->pipeline_data = pipeline_data;
  this->config = pipeline_data->config;

}

LicensePlateCandidate::~LicensePlateCandidate()
{
  delete charSegmenter;
}

// Must delete this pointer in parent class
void LicensePlateCandidate::recognize()
{
  charSegmenter = NULL;

  pipeline_data->plate_area_confidence = 0;
  pipeline_data->isMultiline = config->multiline;

  int expandX = round(this->pipeline_data->regionOfInterest.width * 0.20);
  int expandY = round(this->pipeline_data->regionOfInterest.height * 0.15);
  // expand box by 15% in all directions
  Rect expandedRegion = expandRect( this->pipeline_data->regionOfInterest, expandX, expandY, this->pipeline_data->grayImg.cols, this->pipeline_data->grayImg.rows) ;

  pipeline_data->crop_gray = Mat(this->pipeline_data->grayImg, expandedRegion);
  resize(pipeline_data->crop_gray, pipeline_data->crop_gray, Size(config->templateWidthPx, config->templateHeightPx));
  
  
  CharacterRegion charRegion(pipeline_data);

  if (charRegion.confidence > 10)
  {
    PlateLines plateLines(pipeline_data);

    if (pipeline_data->hasPlateBorder)
      plateLines.processImage(pipeline_data->plateBorderMask, 1.10);
    
    plateLines.processImage(pipeline_data->crop_gray, 0.9);

    PlateCorners cornerFinder(pipeline_data->crop_gray, &plateLines, pipeline_data);
    vector<Point> smallPlateCorners = cornerFinder.findPlateCorners();

    if (cornerFinder.confidence > 0)
    {

      timespec startTime;
      getTime(&startTime);


      Mat originalCrop = pipeline_data->crop_gray;
      
      pipeline_data->plate_corners = transformPointsToOriginalImage(this->pipeline_data->grayImg, pipeline_data->crop_gray, expandedRegion, smallPlateCorners);

      Size outputImageSize = getOutputImageSize(pipeline_data->plate_corners);
      Mat transmtx = getTransformationMatrix(pipeline_data->plate_corners, outputImageSize);
      pipeline_data->crop_gray = deSkewPlate(this->pipeline_data->grayImg, outputImageSize, transmtx);

      cout << "Size: " << outputImageSize.width << " - " << outputImageSize.height << endl;
      
      
      // Apply a perspective transformation to the TextLine objects
      // to match the newly deskewed license plate crop
      vector<TextLine> newLines;
      for (uint i = 0; i < pipeline_data->textLines.size(); i++)
      {        
        vector<Point2f> textArea = transformPointsToOriginalImage(this->pipeline_data->grayImg, originalCrop, expandedRegion, 
                pipeline_data->textLines[i].textArea);
        vector<Point2f> linePolygon = transformPointsToOriginalImage(this->pipeline_data->grayImg, originalCrop, expandedRegion, 
                pipeline_data->textLines[i].linePolygon);
        
        vector<Point2f> textAreaRemapped;
        vector<Point2f> linePolygonRemapped;
        
        perspectiveTransform(textArea, textAreaRemapped, transmtx);
        perspectiveTransform(linePolygon, linePolygonRemapped, transmtx);
        
        newLines.push_back(TextLine(textAreaRemapped, linePolygonRemapped));
      }
      
      pipeline_data->textLines.clear();
      for (uint i = 0; i < newLines.size(); i++)
        pipeline_data->textLines.push_back(newLines[i]);
      
      
      
      if (config->debugTiming)
      {
        timespec endTime;
        getTime(&endTime);
        cout << "deskew Time: " << diffclock(startTime, endTime) << "ms." << endl;
      }
      
      charSegmenter = new CharacterSegmenter(pipeline_data);


      pipeline_data->plate_area_confidence = 100;
    }
    charRegion.confidence = 0;
  }
}

// Re-maps the coordinates from the smallImage to the coordinate space of the bigImage.
vector<Point2f> LicensePlateCandidate::transformPointsToOriginalImage(Mat bigImage, Mat smallImage, Rect region, vector<Point> corners)
{
  vector<Point2f> cornerPoints;
  for (uint i = 0; i < corners.size(); i++)
  {
    float bigX = (corners[i].x * ((float) region.width / smallImage.cols));
    float bigY = (corners[i].y * ((float) region.height / smallImage.rows));

    bigX = bigX + region.x;
    bigY = bigY + region.y;

    cornerPoints.push_back(Point2f(bigX, bigY));
  }

  return cornerPoints;
}

Size LicensePlateCandidate::getOutputImageSize(vector<Point2f> corners)
{
  // Figure out the approximate width/height of the license plate region, so we can maintain the aspect ratio.
  LineSegment leftEdge(round(corners[3].x), round(corners[3].y), round(corners[0].x), round(corners[0].y));
  LineSegment rightEdge(round(corners[2].x), round(corners[2].y), round(corners[1].x), round(corners[1].y));
  LineSegment topEdge(round(corners[0].x), round(corners[0].y), round(corners[1].x), round(corners[1].y));
  LineSegment bottomEdge(round(corners[3].x), round(corners[3].y), round(corners[2].x), round(corners[2].y));

  float w = distanceBetweenPoints(leftEdge.midpoint(), rightEdge.midpoint());
  float h = distanceBetweenPoints(bottomEdge.midpoint(), topEdge.midpoint());
  float aspect = w/h;
  int width = config->ocrImageWidthPx;
  int height = round(((float) width) / aspect);
  if (height > config->ocrImageHeightPx)
  {
    height = config->ocrImageHeightPx;
    width = round(((float) height) * aspect);
  }
  
  return Size(width, height);
}

Mat LicensePlateCandidate::getTransformationMatrix(vector<Point2f> corners, Size outputImageSize)
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

Mat LicensePlateCandidate::deSkewPlate(Mat inputImage, Size outputImageSize, Mat transformationMatrix)
{
  
  
  Mat deskewed(outputImageSize, this->pipeline_data->grayImg.type());
  
  // Apply perspective transformation to the image
  warpPerspective(inputImage, deskewed, transformationMatrix, deskewed.size(), INTER_CUBIC);

  
  
  if (this->config->debugGeneral)
    displayImage(config, "quadrilateral", deskewed);

  return deskewed;
}



