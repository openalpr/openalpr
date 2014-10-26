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
#include "edges/edgefinder.h"
#include "transformation.h"

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


  Rect expandedRegion = this->pipeline_data->regionOfInterest;
  
  pipeline_data->crop_gray = Mat(this->pipeline_data->grayImg, expandedRegion);
  resize(pipeline_data->crop_gray, pipeline_data->crop_gray, Size(config->templateWidthPx, config->templateHeightPx));
  
  
  CharacterAnalysis textAnalysis(pipeline_data);

  if (textAnalysis.confidence > 10)
  {
    PlateLines plateLines(pipeline_data);

    if (pipeline_data->hasPlateBorder)
      plateLines.processImage(pipeline_data->plateBorderMask, 1.10);
    
    plateLines.processImage(pipeline_data->crop_gray, 0.9);

    PlateCorners cornerFinder(pipeline_data->crop_gray, &plateLines, pipeline_data);
    vector<Point> smallPlateCorners = cornerFinder.findPlateCorners();

    EdgeFinder edgeFinder(pipeline_data);
    
    if (cornerFinder.confidence > 0)
    {

      timespec startTime;
      getTime(&startTime);


      Mat originalCrop = pipeline_data->crop_gray;
      
      Transformation imgTransform(this->pipeline_data->grayImg, pipeline_data->crop_gray, expandedRegion);
      pipeline_data->plate_corners = imgTransform.transformSmallPointsToBigImage(smallPlateCorners);
      
      Size cropSize = getCropSize(pipeline_data->plate_corners);
      Mat transmtx = imgTransform.getTransformationMatrix(pipeline_data->plate_corners, cropSize);
      pipeline_data->crop_gray = imgTransform.crop(cropSize, transmtx);
      

      if (this->config->debugGeneral)
        displayImage(config, "quadrilateral", pipeline_data->crop_gray);


      
      // Apply a perspective transformation to the TextLine objects
      // to match the newly deskewed license plate crop
      vector<TextLine> newLines;
      for (uint i = 0; i < pipeline_data->textLines.size(); i++)
      {        
        vector<Point2f> textArea = imgTransform.transformSmallPointsToBigImage(pipeline_data->textLines[i].textArea);
        vector<Point2f> linePolygon = imgTransform.transformSmallPointsToBigImage(pipeline_data->textLines[i].linePolygon);
        
        vector<Point2f> textAreaRemapped;
        vector<Point2f> linePolygonRemapped;
        
        textAreaRemapped = imgTransform.remapSmallPointstoCrop(textArea, transmtx);
        linePolygonRemapped = imgTransform.remapSmallPointstoCrop(linePolygon, transmtx);
        
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
    
  }
}

Size LicensePlateCandidate::getCropSize(vector<Point2f> areaCorners)
{
  // Figure out the approximate width/height of the license plate region, so we can maintain the aspect ratio.
  LineSegment leftEdge(round(areaCorners[3].x), round(areaCorners[3].y), round(areaCorners[0].x), round(areaCorners[0].y));
  LineSegment rightEdge(round(areaCorners[2].x), round(areaCorners[2].y), round(areaCorners[1].x), round(areaCorners[1].y));
  LineSegment topEdge(round(areaCorners[0].x), round(areaCorners[0].y), round(areaCorners[1].x), round(areaCorners[1].y));
  LineSegment bottomEdge(round(areaCorners[3].x), round(areaCorners[3].y), round(areaCorners[2].x), round(areaCorners[2].y));

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

